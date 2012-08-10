#ifndef IMAGEDATAGL_H__
#define IMAGEDATAGL_H__

#include "tgt/tgt_gl.h"
#include "tgt/texture.h"
#include "tgt/textureunit.h"
#include "tgt/vector.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagedatalocal.h"
#include "core/tools/weaklytypedpointer.h"

namespace tgt {
    class Shader;
}

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
         * \param wtp               WeaklyTypedPointer to the image data, must not be 0, ImageDataGL does \b not take ownership of that pointer.
         */
        ImageDataGL(size_t dimensionality, const tgt::svec3& size, const WeaklyTypedPointer& wtp);

        /**
         * Creates a new ImageDataGL representation from GenericImageDataLocal.
         *
         * \param dimensionality    Dimensionality of data
         * \param size              Size of this image (number of elements per dimension)
         * \param data              Pointer to the GenericImageDataLocal instance, must not be 0
         * \tparam  BASETYPE        Base type of image data
         * \tparam  NUMCHANNELS     Number of channels per element
         */
        template<typename BASETYPE, size_t NUMCHANNELS>
        ImageDataGL(size_t dimensionality, const tgt::svec3& size, const GenericImageDataLocal<BASETYPE, NUMCHANNELS>* data);

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


        /**
         * Binds the texture without activating a texture unit.
         */
        void bind() const;

        /**
         * Activates the texture unit \a texUnit and binds the texture.
         * \param   texUnit     Texture unit to activate
         */
        void bind(const tgt::TextureUnit& texUnit) const;

        void bind(
            tgt::Shader* shader,
            const tgt::TextureUnit& texUnit, 
            const std::string& texUniform = "_texture") const;



    protected:
        /**
         * Creates the OpenGL texture from the given pointer \a wtp.
         * \param wtp   WeaklyTypedPointer with source image data
         */
        void createTexture(const WeaklyTypedPointer& wtp);

        /**
         * Creates the OpenGL texture from the given ImageDataRAM \a data.
         * \param data              Pointer to the GenericImageDataLocal instance, must not be 0
         * \tparam  BASETYPE        Base type of image data
         * \tparam  NUMCHANNELS     Number of channels per element
         */
        template<typename BASETYPE, size_t NUMCHANNELS>
        void createTexture(const GenericImageDataLocal<BASETYPE, NUMCHANNELS>* data);

        tgt::Texture* _texture;             //< OpenGL texture

        static const std::string loggerCat_;
    };

// = Template definition ==========================================================================

    template<typename BASETYPE, size_t NUMCHANNELS>
    TUMVis::ImageDataGL::ImageDataGL(size_t dimensionality, const tgt::svec3& size, const GenericImageDataLocal<BASETYPE, NUMCHANNELS>* data) 
        : ImageData(dimensionality, size)
    {
        createTexture<BASETYPE, NUMCHANNELS>(data);
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    void TUMVis::ImageDataGL::createTexture(const GenericImageDataLocal<BASETYPE, NUMCHANNELS>* data) {
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
