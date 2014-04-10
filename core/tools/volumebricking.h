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

#ifndef VOLUMEBRICKING_H__
#define VOLUMEBRICKING_H__

#include "tgt/vector.h"

#include "core/coreapi.h"
#include "core/datastructures/imagedata.h"

#include <vector>

namespace tgt {
    class Texture;
}

namespace campvis {

    class CAMPVIS_CORE_API BinaryBrickedVolume {
    public:
        BinaryBrickedVolume(const ImageData* referenceImage, size_t brickSize);

        ~BinaryBrickedVolume();


        /**
         * Gets the number of bricks in each dimension.
         * \return  _dimBricks
         */
        const tgt::svec3& getNumBricks() const;

        /**
         * Gets the number of brick indices (= hmul(_dimBricks)).
         * \return  _numBrickIndices
         */
        size_t getNumBrickIndices() const;

        /**
         * Gets the number of voxels a brick is covering in each dimension.
         * \return  _brickSize
         */
        size_t getBrickSize() const;


        /**
         * Returns the boolean value for the brick with index \a brickIndex.
         * \param   brickIndex  Lookup brick index
         * \return  The boolean value for the brick with index \a brickIndex.
         */
        bool getValueForIndex(size_t brickIndex) const;

        /**
         * Sets the boolean value for the brick with index \a brickIndex to \a value.
         * \param   brickIndex  Lookup brick index
         * \param   value       The new boolean value for this brick.
         */
        void setValueForIndex(size_t brickIndex, bool value);

        /**
         * Return a vector of all voxels positions in the reference image that are in the brick with the index \a brickIndex.
         * \param   brickIndex  Lookup brick index.
         * \return  A vector of all voxels positions in the reference image that are in the brick with the index \a brickIndex.
         */
        std::vector<tgt::svec3> getAllVoxelsForBrick(size_t brickIndex) const;


        tgt::Texture* exportToImageData() const;

    private:
        /**
         * Returns the brick coordinates for the brick with index \a brickIndex.
         * \param   brickIndex  The Brick index to look up
         * \return  The corresponding 3D brick coordinates.
         */
        tgt::svec3 indexToBrick(size_t brickIndex) const;

        /**
         * Transforms brick coordinates to the corresponding index.
         * \param   brick   Brick coordinates.
         * \return  The corresponding index in if all bricks are in contiguous storage.
         */
        size_t brickToIndex(const tgt::svec3& brick) const;



        const ImageData* _referenceImage;       ///< the reference image
        size_t _brickSize;                      ///< number of voxels a brick is covering in each dimension

        tgt::svec3 _dimBricks;                  ///< number of bricks in each dimension
        size_t _numBrickIndices;                ///< number of brick indices (= hmul(_dimPackedBricks))

        uint8_t* _bricks;                       ///< the densly packed bricks
        tgt::svec3 _dimPackedBricks;            ///< number of elements when bricks are tightly packed
        size_t _numElementsInBricksArray;       ///< number of elements in _bricks
       
    };
}

#endif // VOLUMEBRICKING_H__
