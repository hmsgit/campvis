// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#include "rendervolumevoxelizing.h"

#include "tgt/assert.h"
#include "tgt/glmath.h"
#include "tgt/texture.h"
#include "tgt/textureunit.h"
#include "tgt/tgt_gl.h"

#include "core/datastructures/genericimagerepresentationlocal.h"

#define DIV_CEIL(x,y) ((x) > 0) ? (1 + ((x) - 1)/(y)) : ((x) / (y))

namespace campvis {



    VoxelizedRenderVolume::VoxelizedRenderVolume(const ImageData* referenceImage, size_t brickSize)
        : _referenceImage(referenceImage)
        , _brickSize(brickSize)
        , _bricks(0)
    {
        tgtAssert(_referenceImage != 0, "Reference Image must not be 0!");

        // perform ceiling integer division:
        // z is not considered.
        _dimBricks = referenceImage->getSize();
        for (int i = 0; i < 2; ++i)
            _dimBricks.elem[i] = DIV_CEIL(_dimBricks.elem[i], _brickSize);

        // set the depth of the bricks
        _brickDepth = _dimBricks.z / VOXEL_DEPTH;

        // since the texture is a 2D texture and the elements store the depth  will pack VOXEL_DEPTH number of values along the z axis into one block, the _dimBricks.z is 
        _dimBricks.z = VOXEL_DEPTH;

        _numBrickIndices = tgt::hmul(_dimBricks);

        _dimPackedBricks = _dimBricks;
        _dimPackedBricks.z = _dimPackedBricks.z / VOXEL_DEPTH;

        _numElementsInBricksArray = tgt::hmul(_dimPackedBricks);
#if (VOXEL_DEPTH == 8)
        _bricks = new uint8_t[_numElementsInBricksArray];
        memset(_bricks, 0, _numElementsInBricksArray);
#else if(VOXEL_DEPTH == 32)
        _bricks = new uint32_t[_numElementsInBricksArray];
        memset(_bricks, 0, 4 * _numElementsInBricksArray);
#endif
        
    }

    VoxelizedRenderVolume::~VoxelizedRenderVolume() {
        delete _bricks;
    }

    std::vector<tgt::svec3> VoxelizedRenderVolume::getAllVoxelsForBrick(size_t brickIndex) const {
        tgt::ivec3 refImageSize = _referenceImage->getSize();
        std::vector<tgt::svec3> toReturn;
        toReturn.reserve((_brickSize+2) * (_brickSize+2) * (_brickDepth+2));

        // traverse each dimension, check that voxel is within reference image size
        tgt::ivec3 startVoxel = indexToBrick(brickIndex);
        startVoxel.x *= _brickSize;
        startVoxel.y *= _brickSize;
        startVoxel.z *= _brickDepth;

        for (int x = -1; x < static_cast<int>(_brickSize + 1); ++x) {
            int xx = startVoxel.x + x;
            if (xx < 0)
                continue;
            else if (xx >= refImageSize.x)
                break;

            for (int y = -1; y < static_cast<int>(_brickSize + 1); ++y) {
                int yy = startVoxel.y + y;
                if (yy < 0)
                    continue;
                else if (yy >= refImageSize.y)
                    break;

                for (int z = -1; z < static_cast<int>(_brickDepth + 1); ++z) {
                    int zz = startVoxel.z + z;
                    if (zz < 0)
                        continue;
                    else if (zz >= refImageSize.z)
                        break;

                    toReturn.push_back(tgt::svec3(xx, yy, zz));
                }
            }
        }

        return toReturn;
    }

    tgt::svec3 VoxelizedRenderVolume::indexToBrick(size_t brickIndex) const {
        size_t z = brickIndex / (_dimBricks.x * _dimBricks.y);
        size_t y = (brickIndex % (_dimBricks.x * _dimBricks.y)) / _dimBricks.x;
        size_t x = (brickIndex % (_dimBricks.x * _dimBricks.y)) % _dimBricks.x;
        return tgt::svec3(x, y, z);
    }

    size_t VoxelizedRenderVolume::brickToIndex(const tgt::svec3& brick) const {
        return brick.x + (_dimBricks.x * brick.y) + (_dimBricks.x * _dimBricks.y * brick.z);
    }

    bool VoxelizedRenderVolume::getValueForIndex(size_t brickIndex) const {
        size_t byte = brickIndex % (_dimBricks.x * _dimBricks.y);
        size_t bit = brickIndex / (_dimBricks.x * _dimBricks.y);
        tgtAssert(byte < _numElementsInBricksArray, "Brick brickIndex out of bounds!");

        return (_bricks[byte] & (1 << bit)) != 0;
    }

    void VoxelizedRenderVolume::setValueForIndex(size_t brickIndex, bool value) {
        size_t byte = brickIndex % (_dimBricks.x * _dimBricks.y);
        size_t bit = brickIndex / (_dimBricks.x * _dimBricks.y);
        tgtAssert(byte < _numElementsInBricksArray, "Brick brickIndex out of bounds!");
#if (VOXEL_DEPTH == 8)
        if (value)
            _bricks[byte] |= static_cast<uint8_t>(1 << bit);
        else
            _bricks[byte] &= static_cast<uint8_t>(~(1 << bit));
#else if(VOXEL_DEPTH == 32)
        if (value)
            _bricks[byte] |= static_cast<uint32_t>(1 << bit);
        else
            _bricks[byte] &= static_cast<uint32_t>(~(1 << bit));
#endif
    }

    tgt::Texture* VoxelizedRenderVolume::createEmptyImageData() const {
#if(VOXEL_DEPTH == 8)
        tgt::Texture* toReturn = new tgt::Texture(0, _dimPackedBricks, GL_RED_INTEGER, GL_R32UI, GL_UNSIGNED_BYTE, tgt::Texture::NEAREST);
#else if(VOXEL_DEPTH == 32)
        tgt::Texture* toReturn = new tgt::Texture(0, _dimPackedBricks, GL_RED_INTEGER, GL_R32UI, GL_UNSIGNED_INT, tgt::Texture::NEAREST);
#endif
        LGL_ERROR;
       glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        tgt::TextureUnit tempUnit;
        tempUnit.activate();
        toReturn->bind();
        toReturn->uploadTexture();
        LGL_ERROR;
        toReturn->setWrapping(tgt::Texture::CLAMP);
        toReturn->setPixelData(0);
        LGL_ERROR;

        return toReturn;
    }

    tgt::Texture* VoxelizedRenderVolume::exportToImageData() const {
#if(VOXEL_DEPTH == 8)
        tgt::Texture* toReturn = new tgt::Texture(_bricks, _dimPackedBricks, GL_RED_INTEGER, GL_R32UI, GL_UNSIGNED_BYTE, tgt::Texture::NEAREST);
#else if(VOXEL_DEPTH == 32)
        tgt::Texture* toReturn = new tgt::Texture((GLubyte*)_bricks, _dimPackedBricks, GL_RED_INTEGER, GL_R32UI, GL_UNSIGNED_INT, tgt::Texture::NEAREST);
#endif
        
        LGL_ERROR;
       glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        tgt::TextureUnit tempUnit;
        tempUnit.activate();
        toReturn->bind();
        toReturn->uploadTexture();
        LGL_ERROR;
        toReturn->setWrapping(tgt::Texture::CLAMP);
        LGL_ERROR;

        return toReturn;
    }

    const tgt::svec3& VoxelizedRenderVolume::getNumBricks() const {
        return _dimBricks;
    }

    size_t VoxelizedRenderVolume::getNumBrickIndices() const {
        return _numBrickIndices;
    }

    size_t VoxelizedRenderVolume::getBrickSize() const {
        return _brickSize;
    }

    size_t VoxelizedRenderVolume::getBrickDepth() const {
        return _brickDepth;
    }

    int VoxelizedRenderVolume::getMaxDim() const {
        return std::max(_dimPackedBricks.x, _dimPackedBricks.y);
    }

    int VoxelizedRenderVolume::getWidth() {
        return _dimPackedBricks.x;
    }

    int VoxelizedRenderVolume::getHeight() {
        return _dimPackedBricks.y;
    }
}