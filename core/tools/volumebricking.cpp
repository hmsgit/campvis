// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#include "volumebricking.h"

#include "tgt/assert.h"
#include "tgt/glmath.h"
#include "tgt/texture.h"
#include "tgt/textureunit.h"

#include "core/datastructures/genericimagerepresentationlocal.h"


#define DIV_CEIL(x,y) ((x) > 0) ? (1 + ((x) - 1)/(y)) : ((x) / (y))

namespace campvis {



    BinaryBrickedVolume::BinaryBrickedVolume(const ImageData* referenceImage, size_t brickSize)
        : _referenceImage(referenceImage)
        , _brickSize(brickSize)
        , _bricks(0)
    {
        tgtAssert(_referenceImage != 0, "Reference Image must not be 0!");

        // perform ceiling integer division:
        _dimBricks = referenceImage->getSize();
        for (int i = 0; i < 3; ++i)
            _dimBricks.elem[i] = DIV_CEIL(_dimBricks.elem[i], _brickSize);

        // since we will pack eight values along the x axis into one byte, the _dimBricks.x must be congruent 0 mod 8.
        if (_dimBricks.x % 8 != 0)
            _dimBricks.x += 8 - (_dimBricks.x % 8);
        _numBrickIndices = tgt::hmul(_dimBricks);

        _dimPackedBricks = _dimBricks;
        _dimPackedBricks.x = _dimPackedBricks.x / 8;

        _numElementsInBricksArray = tgt::hmul(_dimPackedBricks);
        _bricks = new uint8_t[_numElementsInBricksArray];
        memset(_bricks, 0, _numElementsInBricksArray);
    }

    BinaryBrickedVolume::~BinaryBrickedVolume() {
        delete _bricks;
    }

    std::vector<tgt::svec3> BinaryBrickedVolume::getAllVoxelsForBrick(size_t brickIndex) const {
        tgt::ivec3 refImageSize = _referenceImage->getSize();
        std::vector<tgt::svec3> toReturn;
        toReturn.reserve((_brickSize+2) * (_brickSize+2) * (_brickSize+2));

        // traverse each dimension, check that voxel is within reference image size
        tgt::ivec3 startVoxel = indexToBrick(brickIndex) * _brickSize;
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

                    toReturn.push_back(tgt::svec3(xx, yy, zz));
                }
            }
        }

        return toReturn;
    }

    tgt::svec3 BinaryBrickedVolume::indexToBrick(size_t brickIndex) const {
        size_t z = brickIndex / (_dimBricks.x * _dimBricks.y);
        size_t y = (brickIndex % (_dimBricks.x * _dimBricks.y)) / _dimBricks.x;
        size_t x = brickIndex % _dimBricks.x;
        return tgt::svec3(x, y, z);
    }

    size_t BinaryBrickedVolume::brickToIndex(const tgt::svec3& brick) const {
        return brick.x + (_dimBricks.x * brick.y) + (_dimBricks.x * _dimBricks.y * brick.z);
    }

    bool BinaryBrickedVolume::getValueForIndex(size_t brickIndex) const {
        size_t byte = brickIndex / 8;
        size_t bit = brickIndex % 8;
        tgtAssert(brickIndex < _numElementsInBricksArray, "Brick brickIndex out of bounds!");

        return (_bricks[byte] & (1 << bit)) != 0;
    }

    void BinaryBrickedVolume::setValueForIndex(size_t brickIndex, bool value) {
        size_t byte = brickIndex / 8;
        size_t bit = brickIndex % 8;
        tgtAssert(byte < _numElementsInBricksArray, "Brick brickIndex out of bounds!");

        if (value)
            _bricks[byte] |= (1 << bit);
        else
            _bricks[byte] &= ~(1 << bit);
    }

    tgt::Texture* BinaryBrickedVolume::exportToImageData() const {
        tgt::Texture* toReturn = new tgt::Texture(_bricks, _dimPackedBricks, GL_RED_INTEGER, GL_R8UI, GL_UNSIGNED_BYTE, tgt::Texture::NEAREST);
        LGL_ERROR;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        tgt::TextureUnit tempUnit;
        tempUnit.activate();
        toReturn->bind();
        toReturn->uploadTexture();
        LGL_ERROR;
        toReturn->setWrapping(tgt::Texture::CLAMP);
        toReturn->setPixelData(0);
        tgt::TextureUnit::setZeroUnit();
        LGL_ERROR;

        return toReturn;

//         uint8_t* copy = new uint8_t[_numElementsInBricksArray];
//         memcpy(copy, _bricks, _numElementsInBricksArray);
// 
//         ImageData* toReturn = new ImageData(_referenceImage->getDimensionality(), _dimPackedBricks, 1);
//         GenericImageRepresentationLocal<uint8_t, 1>::create(toReturn, copy);
//         return toReturn;
    }

    const tgt::svec3& BinaryBrickedVolume::getNumBricks() const {
        return _dimBricks;
    }

    size_t BinaryBrickedVolume::getNumBrickIndices() const {
        return _numBrickIndices;
    }

    size_t BinaryBrickedVolume::getBrickSize() const {
        return _brickSize;
    }


}