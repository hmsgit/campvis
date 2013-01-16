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

#ifndef IMAGEREPRESENTATIONGL_H__
#define IMAGEREPRESENTATIONGL_H__

#include "tgt/tgt_gl.h"
#include "tgt/texture.h"
#include "core/datastructures/abstractimagerepresentation.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/tools/weaklytypedpointer.h"

namespace tgt {
    class Shader;
    class TextureUnit;
}

namespace campvis {

    /**
     * Stores image data as OpenGL texture.
     * Can bei instantiated either by a WeaklyTypedPointer or strongly typed by an GenericImageRepresentationLocal.
     */
    class ImageRepresentationGL : public AbstractImageRepresentation {
    public:
        
        /**
         * Creates a new ImageRepresentationGL representation from a tgt::Texture.
         * 
         * \param   parent      Image this representation represents, must not be 0.
         * \param   texture     OpenGL texture to use, must not be 0, ImageRepresentationGL will take ownership of this texture.
         */
        ImageRepresentationGL(const ImageData* parent, tgt::Texture* texture);

        /**
         * Creates a new ImageRepresentationGL representation.
         *
         * \param   parent  Image this representation represents, must not be 0.
         * \param   wtp     WeaklyTypedPointer to the image data, must not be 0, ImageRepresentationGL does \b not take ownership of that pointer.
         */
        ImageRepresentationGL(const ImageData* parent, const WeaklyTypedPointer& wtp);

        /**
         * Creates a new ImageRepresentationGL representation from GenericImageRepresentationLocal.
         *
         * \param   parent      Image this representation represents, must not be 0.
         * \param   data        Pointer to the GenericImageDataLocal instance, must not be 0
         * \tparam  BASETYPE        Base type of image data
         * \tparam  NUMCHANNELS     Number of channels per element
         */
        template<typename BASETYPE, size_t NUMCHANNELS>
        ImageRepresentationGL(const ImageData* parent, const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* data);

        /**
         * Destructor
         */
        virtual ~ImageRepresentationGL();

       
        /**
         * \see AbstractImageRepresentation::clone()
         **/
        virtual ImageRepresentationGL* clone() const;

        /// \see AbstractImageRepresentation::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;

        /// \see AbstractImageRepresentation::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;

        /**
         * \see AbstractImageRepresentation::getSubImage
         */
        virtual ImageRepresentationGL* getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const;


        /**
         * Binds the texture without activating a texture unit.
         */
        void bind() const;

        /**
         * Activates the texture unit \a texUnit and binds the texture.
         * \param   texUnit     Texture unit to activate
         */
        void bind(const tgt::TextureUnit& texUnit) const;

        /**
         * Binds the image to the given shader using \a texUnit as texture unit and \a texUniform as shader uniform.
         * \param   shader      Shader to bind image and set uniforms to.
         * \param   texUnit     TextureUnit to bind image to.
         * \param   texUniform  Uniform name for the sampler.
         */
        void bind(
            tgt::Shader* shader,
            const tgt::TextureUnit& texUnit, 
            const std::string& texUniform = "_texture") const;

        /**
         * Gets the OpenGL texture.
         * \return  _texture
         */
        const tgt::Texture* getTexture() const;


    protected:
        /**
         * Creates the OpenGL texture from the given pointer \a wtp.
         * \param wtp   WeaklyTypedPointer with source image data
         */
        void createTexture(const WeaklyTypedPointer& wtp);

        void setupAndUploadTexture(tgt::Texture* texture, bool isInteger, bool isSigned);

        /**
         * Creates the OpenGL texture from the given GenericImageRepresentationLocal \a data.
         * \param data              Pointer to the GenericImageRepresentationLocal instance, must not be 0
         * \tparam  BASETYPE        Base type of image data
         * \tparam  NUMCHANNELS     Number of channels per element
         */
        template<typename BASETYPE, size_t NUMCHANNELS>
        void createTexture(const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* data);

        tgt::Texture* _texture;             //< OpenGL texture

        static const std::string loggerCat_;
    };

// = Template definition ==========================================================================

    template<typename BASETYPE, size_t NUMCHANNELS>
    campvis::ImageRepresentationGL::ImageRepresentationGL(const ImageData* parent, const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* data) 
        : AbstractImageRepresentation(parent)
    {
        createTexture<BASETYPE, NUMCHANNELS>(data);
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    void campvis::ImageRepresentationGL::createTexture(const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* data) {
        tgtAssert(data != 0, "Pointer to image must not be 0!");

        _texture = new tgt::Texture(
            reinterpret_cast<GLubyte*>(data->getImageData()), 
            _size, 
            TypeTraits<BASETYPE, NUMCHANNELS>::glFormat,
            TypeTraits<BASETYPE, NUMCHANNELS>::glInternalFormat, 
            TypeTraits<BASETYPE, NUMCHANNELS>::glDataType, 
            tgt::Texture::LINEAR);

        setupAndUploadTexture(_texture, TypeTraitsHelperPerBasetype<BASETYPE>::isInteger, TypeTraitsHelperPerBasetype<BASETYPE>::isSigned);
    }
}

#endif // IMAGEREPRESENTATIONGL_H__
