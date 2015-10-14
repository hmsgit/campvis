// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#ifndef imagemapping_h__
#define imagemapping_h__

#include "cgt/matrix.h"
#include "cgt/vector.h"

#include "core/coreapi.h"
#include "core/tools/mapping.h"

namespace campvis {

    /**
     * Contains all necessary for mapping the image or image values.
     * 
     * \todo    Make a full list of fields that belong in this class. Then check
     *          which of them belong together and which are derived measures.
     */
    class CAMPVIS_CORE_API ImageMappingInformation {
    public:
        /**
         * Creates a new default ImageMappingInformation.
         * \param   size                    Image size (number of elements, redundant...)
         * \param   offset                  Position of LLF corner in world coordinates (mm)
         * \param   voxelSize               Voxel size in (mm)
         * \param   customTransformation    Additional custom transformation from voxel to world coordinates, defaults to identity.
         */
        ImageMappingInformation(
            const cgt::vec3& size, 
            const cgt::vec3& offset, 
            const cgt::vec3& voxelSize, 
            const cgt::mat4& customTransformation = cgt::mat4::identity);

        /**
         * Returns the position of LLF corner in world coordinates (mm).
         * \return  _offset
         */
        const cgt::vec3& getOffset() const;

        /**
         * Returns the voxel size in mm.
         * \return  _voxelSize
         */
        const cgt::vec3& getVoxelSize() const;

        /**
         * Returns the additional custom transformation from voxel to world coordinates.
         * \return  _customTransformation
         */
        const cgt::mat4 getCustomTransformation() const;

        /**
         * Gets the transformation matrix from texture to world coordinates.
         * \return  _textureToWorldTransformation
         */
        const cgt::mat4& getTextureToWorldMatrix() const;

        /**
         * Gets the transformation matrix from world to texture coordinates.
         * \return  _worldToTextureTransformation
         */
        const cgt::mat4& getWorldToTextureMatrix() const;

        /**
         * Gets the transformation matrix from voxel to world coordinates.
         * \return  _voxelToWorldTransformation
         */
        const cgt::mat4& getVoxelToWorldMatrix() const;

        /**
         * Gets the transformation matrix from world to voxel coordinates.
         * \return  _worldToVoxelTransformation
         */
        const cgt::mat4& getWorldToVoxelMatrix() const;

        /**
         * Overloaded equal operator
         * \param   obj    Object to compare with.
         */
        bool operator==(const ImageMappingInformation& obj) const ;

    private:
        /**
         * Updates the X-to-Y matrices.
         */
        void updateMatrices();

        cgt::vec3 _size;                                ///< Image size (number of elements, redundant...)
        cgt::vec3 _offset;                              ///< Position of LLF corner in world coordinates (mm)
        cgt::vec3 _voxelSize;                           ///< Voxel size in (mm)
        cgt::mat4 _customTransformation;                ///< Additional custom transformation from voxel to world coordinates

        cgt::mat4 _textureToWorldTransformation;        ///< Transformation matrix from texture to world coordinates
        cgt::mat4 _worldToTextureTransformation;        ///< Transformation matrix from world to texture coordinates

        cgt::mat4 _voxelToWorldTransformation;        ///< Transformation matrix from voxel to world coordinates
        cgt::mat4 _worldToVoxelTransformation;        ///< Transformation matrix from world to voxel coordinates
    };

}
#endif // imagemapping_h__
