// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef imagemapping_h__
#define imagemapping_h__

#include "tgt/matrix.h"
#include "tgt/vector.h"
#include "core/tools/mapping.h"

namespace TUMVis {

    /**
     * Contains all necessary for mapping the image or image values.
     * 
     * \todo    Make a full list of fields that belong in this class. Then check
     *          which of them belong together and which are derived measures.
     */
    class ImageMappingInformation {
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
         * \return  _textureToWolrdTransformation
         */
        const tgt::mat4& getTextureToWorldMatrix() const;

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

    private:
        /**
         * Updates the X-to-Y matrices.
         */
        void updateMatrixes();

        tgt::vec3 _size;                                ///< Image size (number of elements, redundant...)
        tgt::vec3 _offset;                              ///< Position of LLF corner in world coordinates (mm)
        tgt::vec3 _voxelSize;                           ///< Voxel size in (mm)

        LinearMapping<float> _realWorldValueMapping;     ///< Linear mapping for mapping element values to real world values
        tgt::mat4 _textureToWolrdTransformation;        ///< Transformation matrix from texture to world coordinates

        static const std::string loggerCat_;
    };

}
#endif // imagemapping_h__
