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

#ifndef imagemapping_h__
#define imagemapping_h__

#include "tgt/matrix.h"
#include "tgt/vector.h"

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
         * \param   realWorldValueMapping   Linear mapping for mapping element values to real world values, defaults to identity.
         */
        ImageMappingInformation(
            const tgt::vec3& size, 
            const tgt::vec3& offset, 
            const tgt::vec3& voxelSize, 
            const LinearMapping<float>& realWorldValueMapping = LinearMapping<float>::identity);

        /**
         * Returns the position of LLF corner in world coordinates (mm).
         * \return  _offset
         */
        const tgt::vec3& getOffset() const;

        /**
         * Returns the voxel size in mm.
         * \return  _voxelSize
         */
        const tgt::vec3& getVoxelSize() const;

        /**
         * Gets the transformation matrix from texture to world coordinates.
         * \return  _textureToWorldTransformation
         */
        const tgt::mat4& getTextureToWorldMatrix() const;

        /**
         * Gets the transformation matrix from world to texture coordinates.
         * \return  _worldToTextureTransformation
         */
        const tgt::mat4& getWorldToTextureMatrix() const;

        /**
         * Gets the transformation matrix from voxel to world coordinates.
         * \return  _voxelToWorldTransformation
         */
        const tgt::mat4& getVoxelToWorldMatrix() const;

        /**
         * Gets the transformation matrix from world to voxel coordinates.
         * \return  _worldToVoxelTransformation
         */
        const tgt::mat4& getWorldToVoxelMatrix() const;

        /**
         * Get the real world value mapping.
         * \return  Linear mapping for mapping element values to real world values.
         */
        const LinearMapping<float>& getRealWorldMapping() const;
        
        /**
         * Set the real world value mapping.
         * \param   rwvm    Linear mapping for mapping element values to real world values.
         */
        void setRealWorldMapping(const LinearMapping<float>& rwvm);

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

        tgt::vec3 _size;                                ///< Image size (number of elements, redundant...)
        tgt::vec3 _offset;                              ///< Position of LLF corner in world coordinates (mm)
        tgt::vec3 _voxelSize;                           ///< Voxel size in (mm)

        LinearMapping<float> _realWorldValueMapping;     ///< Linear mapping for mapping element values to real world values

        tgt::mat4 _textureToWorldTransformation;        ///< Transformation matrix from texture to world coordinates
        tgt::mat4 _worldToTextureTransformation;        ///< Transformation matrix from world to texture coordinates

        tgt::mat4 _voxelToWorldTransformation;        ///< Transformation matrix from voxel to world coordinates
        tgt::mat4 _worldToVoxelTransformation;        ///< Transformation matrix from world to voxel coordinates
    };

}
#endif // imagemapping_h__
