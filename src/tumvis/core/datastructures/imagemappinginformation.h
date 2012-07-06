#ifndef imagemapping_h__
#define imagemapping_h__

#include "tgt/matrix.h"
#include "tools/mapping.h"

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
         * Creates a new default ImageMappingInformation. All mappings are identity.
         */
        ImageMappingInformation();


        /**
         * Get the real world value mapping.
         * \return  Linear mapping for mapping element values to real world values.
         */
        const LinearMaping<float>& getRealWorldMapping() const;

        /**
         * Set the real world value mapping.
         * \param   rwvm    Linear mapping for mapping element values to real world values.
         */
        void setRealWorldMapping(const LinearMaping<float>& rwvm);

    private:
        LinearMaping<float> _realWorldValueMapping;     ///< Linear mapping for mapping element values to real world values
        tgt::mat4 _voxelToWorldTransformation;          ///< Transformation matrix from voxel to world coordinates

        static const std::string loggerCat_;
    };

}
#endif // imagemapping_h__
