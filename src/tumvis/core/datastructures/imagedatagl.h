#ifndef IMAGEDATAGL_H__
#define IMAGEDATAGL_H__

#include "tgt/texture.h"
#include "tgt/vector.h"
#include "core/datastructures/imagedata.h"
#include "core/tools/weaklytypedpointer.h"

namespace TUMVis {

    /**
     * Stores image data as OpenGL texture.
     * Can bei instantiated either by a WeaklyTypedPointer or strongly typed by an ImageDataRAM.
     */
    class ImageDataGL : public ImageData {
    public:
        /**
         * Creates a new ImageDataGL representation.
         *
         * \param dimensionality    Dimensionality of data
         * \param size              Size of this image (number of elements per dimension)
         * \param data              Pointer to the image data, must not be 0, ImageDataGL does not take ownership of that pointer.
         */
        ImageDataGL(size_t dimensionality, const tgt::svec3& size, const WeaklyTypedPointer& wtp);

        /**
         * Creates a new ImageDataGL representation from ImageDataRAM.
         *
         * \param dimensionality    Dimensionality of data
         * \param size              Size of this image (number of elements per dimension)
         * \param data              Pointer to the ImageDataRAM instance, must not be 0, type has to match the template parameters.
         * \tparam  BASETYPE        Base type of image data
         * \tparam  NUMCHANNELS     Number of channels per element
         */
        template<typename BASETYPE, size_t NUMCHANNELS>
        ImageDataGL(size_t dimensionality, const tgt::svec3& size, typename TypeTraits<BASETYPE, NUMCHANNELS>::ImageRAMType* data);

        /**
         * Destructor
         */
        virtual ~ImageDataGL();


       
        /**
         * \see AbstractData::clone()
         **/
        virtual ImageDataGL* clone() const;

        /**
         * \see ImageData::getSubImage
         */
        virtual ImageDataGL* getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const;



    protected:
        /**
         * Creates the OpenGL texture from the given pointer \a wtp.
         * \param wtp   WeaklyTypedPointer with source image data
         */
        void createTexture(const WeaklyTypedPointer& wtp);

        /**
         * Creates the OpenGL texture from the given ImageDataRAM \a data.
         * \param data              Pointer to the ImageDataRAM instance, must not be 0, type has to match the template parameters.
         * \tparam  BASETYPE        Base type of image data
         * \tparam  NUMCHANNELS     Number of channels per element
         */
        template<typename BASETYPE, size_t NUMCHANNELS>
        void createTexture(typename TypeTraits<BASETYPE, NUMCHANNELS>::ImageRAMType* data);

        tgt::Texture* _texture;             //< OpenGL texture

        static const std::string loggerCat_;
    };

// = Template definition ==========================================================================

    template<typename BASETYPE, size_t NUMCHANNELS>
    TUMVis::ImageDataGL::ImageDataGL(size_t dimensionality, const tgt::svec3& size, typename TypeTraits<BASETYPE, NUMCHANNELS>::ImageRAMType* imageRAM) 
        : ImageData(dimensionality, size)
    {
        createTexture<BASETYPE, NUMCHANNELS>(data);
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    void TUMVis::ImageDataGL::createTexture(typename TypeTraits<BASETYPE, NUMCHANNELS>::ImageRAMType* data) {
        tgtAssert(data != 0, "Pointer to image must not be 0!");

        _texture = new tgt::Texture(
            reinterpret_cast<GLubyte*>(data->getImageData()), 
            _size, 
            TypeTraits<BASETYPE, NUMCHANNELS>::glFormat,
            TypeTraits<BASETYPE, NUMCHANNELS>::glInternalFormat, 
            TypeTraits<BASETYPE, NUMCHANNELS>::glDataType, 
            tgt::Texture::LINEAR);

        switch (_dimensionality) {
            case 1:
                _texture->setType(GL_TEXTURE_1D);
                break;
            case 2:
                _texture->setType(GL_TEXTURE_2D);
                break;
            case 3:
                _texture->setType(GL_TEXTURE_3D);
                break;
            default:
                tgtAssert(false, "Unsupported dimensionality of image.");
                break;
        }

        _texture->bind();
        _texture->uploadTexture();
        _texture->setWrapping(tgt::Texture::CLAMP);

        // revoke ownership of local pixel data from the texture
        _texture->setPixelData(0);

        LGL_ERROR;
    }
}

#endif // IMAGEDATAGL_H__
