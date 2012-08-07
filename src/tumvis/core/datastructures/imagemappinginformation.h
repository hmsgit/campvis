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
         * \param   offset                  Position of LLF corner in world coordinates (mm)
         * \param   voxelSize               Voxel size in (mm)
         * \param   realWorldValueMapping   Linear mapping for mapping element values to real world values, defaults to identity.
         */
        ImageMappingInformation(
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
        tgt::vec3 _offset;                              ///< Position of LLF corner in world coordinates (mm)
        tgt::vec3 _voxelSize;                           ///< Voxel size in (mm)

        LinearMapping<float> _realWorldValueMapping;     ///< Linear mapping for mapping element values to real world values
        //tgt::mat4 _voxelToWorldTransformation;          ///< Transformation matrix from voxel to world coordinates

        static const std::string loggerCat_;
    };

}
#endif // imagemapping_h__
