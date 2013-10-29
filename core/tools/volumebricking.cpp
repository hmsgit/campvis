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
        _numBricks = referenceImage->getSize();
        for (int i = 0; i < 3; ++i)
            _numBricks.elem[i] = DIV_CEIL(_numBricks.elem[i], _brickSize);

        _numBrickIndices = tgt::hmul(_numBricks);
        _numElementsInBricksArray = DIV_CEIL(_numBrickIndices, 8);
        _bricks = new uint8_t[_numElementsInBricksArray];
        memset(_bricks, 0, _numElementsInBricksArray);
    }

    BinaryBrickedVolume::~BinaryBrickedVolume() {
        delete _bricks;
    }

    std::vector<tgt::svec3> BinaryBrickedVolume::getAllVoxelsForBrick(size_t brickIndex) const {
        const tgt::svec3& refImageSize = _referenceImage->getSize();
        std::vector<tgt::svec3> toReturn;
        toReturn.reserve(_brickSize * _brickSize * _brickSize);

        // traverse each dimension, check that voxel is within reference image size
        tgt::svec3 startVoxel = indexToBrick(brickIndex) * _brickSize;
        for (size_t x = 0; x < _brickSize && startVoxel.x + x < refImageSize.x; ++x) {
            for (size_t y = 0; y < _brickSize && startVoxel.y + y < refImageSize.y; ++y) {
                for (size_t z = 0; z < _brickSize && startVoxel.z + z < refImageSize.z; ++z) {
                    toReturn.push_back(tgt::svec3(startVoxel.x + x, startVoxel.y + y, startVoxel.z + z));
                }
            }
        }

        return toReturn;
    }

    tgt::svec3 BinaryBrickedVolume::indexToBrick(size_t brickIndex) const {
        size_t z = brickIndex / (_numBricks.x * _numBricks.y);
        size_t y = (brickIndex % (_numBricks.x * _numBricks.y)) / _numBricks.x;
        size_t x = brickIndex % _numBricks.x;
        return tgt::svec3(x, y, z);
    }

    size_t BinaryBrickedVolume::brickToIndex(const tgt::svec3& brick) const {
        return brick.x + (_numBricks.x * brick.y) + (_numBricks.x * _numBricks.y * brick.z);
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

    ImageData* BinaryBrickedVolume::exportToImageData() const {
        tgt::svec3 size = _numBricks; 
        size.z = DIV_CEIL(size.z, 8);
        size_t numElementsInCopy = tgt::hmul(size);

        uint8_t* copy = new uint8_t[numElementsInCopy];
        memset(copy, 0, numElementsInCopy);
        memcpy(copy, _bricks, _numElementsInBricksArray);

        ImageData* toReturn = new ImageData(_referenceImage->getDimensionality(), size, 1);
        GenericImageRepresentationLocal<uint8_t, 1>::create(toReturn, copy);
        return toReturn;
    }

    const tgt::svec3& BinaryBrickedVolume::getNumBricks() const {
        return _numBricks;
    }

    size_t BinaryBrickedVolume::getNumBrickIndices() const {
        return _numBrickIndices;
    }


}