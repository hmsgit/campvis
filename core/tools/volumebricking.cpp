// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
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

#include "volumebricking.h"

#include "cgt/assert.h"
#include "cgt/glmath.h"
#include "cgt/texture.h"
#include "cgt/textureunit.h"

#include "core/datastructures/genericimagerepresentationlocal.h"


#define DIV_CEIL(x,y) ((x) > 0) ? (1 + ((x) - 1)/(y)) : ((x) / (y))

namespace campvis {



    BinaryBrickedVolume::BinaryBrickedVolume(const ImageData* referenceImage, size_t brickSize)
        : _referenceImage(referenceImage)
        , _brickSize(brickSize)
        , _bricks(0)
    {
        cgtAssert(_referenceImage != 0, "Reference Image must not be 0!");

        // perform ceiling integer division:
        _dimBricks = referenceImage->getSize();
        for (int i = 0; i < 3; ++i)
            _dimBricks.elem[i] = DIV_CEIL(_dimBricks.elem[i], _brickSize);

        // since we will pack eight values along the x axis into one byte, the _dimBricks.x must be congruent 0 mod 8.
        if (_dimBricks.x % 8 != 0)
            _dimBricks.x += 8 - (_dimBricks.x % 8);
        _numBrickIndices = cgt::hmul(_dimBricks);

        _dimPackedBricks = _dimBricks;
        _dimPackedBricks.x = _dimPackedBricks.x / 8;

        _numElementsInBricksArray = cgt::hmul(_dimPackedBricks);
        _bricks = new uint8_t[_numElementsInBricksArray];
        memset(_bricks, 0, _numElementsInBricksArray);
    }

    BinaryBrickedVolume::~BinaryBrickedVolume() {
        delete _bricks;
    }

    std::vector<cgt::svec3> BinaryBrickedVolume::getAllVoxelsForBrick(size_t brickIndex) const {
        cgt::ivec3 refImageSize = _referenceImage->getSize();
        std::vector<cgt::svec3> toReturn;
        toReturn.reserve((_brickSize+2) * (_brickSize+2) * (_brickSize+2));

        // traverse each dimension, check that voxel is within reference image size
        cgt::ivec3 startVoxel = indexToBrick(brickIndex) * _brickSize;
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

                for (int z = -1; z < static_cast<int>(_brickSize + 1); ++z) {
                    int zz = startVoxel.z + z;
                    if (zz < 0)
                        continue;
                    else if (zz >= refImageSize.z)
                        break;

                    toReturn.push_back(cgt::svec3(xx, yy, zz));
                }
            }
        }

        return toReturn;
    }

    cgt::svec3 BinaryBrickedVolume::indexToBrick(size_t brickIndex) const {
        size_t z = brickIndex / (_dimBricks.x * _dimBricks.y);
        size_t y = (brickIndex % (_dimBricks.x * _dimBricks.y)) / _dimBricks.x;
        size_t x = brickIndex % _dimBricks.x;
        return cgt::svec3(x, y, z);
    }

    size_t BinaryBrickedVolume::brickToIndex(const cgt::svec3& brick) const {
        return brick.x + (_dimBricks.x * brick.y) + (_dimBricks.x * _dimBricks.y * brick.z);
    }

    bool BinaryBrickedVolume::getValueForIndex(size_t brickIndex) const {
        size_t byte = brickIndex / 8;
        size_t bit = brickIndex % 8;
        cgtAssert(brickIndex < _numElementsInBricksArray, "Brick brickIndex out of bounds!");

        return (_bricks[byte] & (1 << bit)) != 0;
    }

    void BinaryBrickedVolume::setValueForIndex(size_t brickIndex, bool value) {
        size_t byte = brickIndex / 8;
        size_t bit = brickIndex % 8;
        cgtAssert(byte < _numElementsInBricksArray, "Brick brickIndex out of bounds!");

        if (value)
            _bricks[byte] |= (1 << bit);
        else
            _bricks[byte] &= ~(1 << bit);
    }

    cgt::Texture* BinaryBrickedVolume::exportToImageData() const {
        cgt::TextureUnit tempUnit;
        tempUnit.activate();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        cgt::Texture* toReturn = new cgt::Texture(GL_TEXTURE_3D, _dimPackedBricks, GL_R8UI, _bricks, GL_RED_INTEGER, GL_UNSIGNED_BYTE, cgt::Texture::NEAREST);
        toReturn->setWrapping(cgt::Texture::CLAMP);
        cgt::TextureUnit::setZeroUnit();
        LGL_ERROR;

        return toReturn;

//         uint8_t* copy = new uint8_t[_numElementsInBricksArray];
//         memcpy(copy, _bricks, _numElementsInBricksArray);
// 
//         ImageData* toReturn = new ImageData(_referenceImage->getDimensionality(), _dimPackedBricks, 1);
//         GenericImageRepresentationLocal<uint8_t, 1>::create(toReturn, copy);
//         return toReturn;
    }

    const cgt::svec3& BinaryBrickedVolume::getNumBricks() const {
        return _dimBricks;
    }

    size_t BinaryBrickedVolume::getNumBrickIndices() const {
        return _numBrickIndices;
    }

    size_t BinaryBrickedVolume::getBrickSize() const {
        return _brickSize;
    }


}