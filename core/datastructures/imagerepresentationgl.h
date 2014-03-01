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

#ifndef IMAGEREPRESENTATIONGL_H__
#define IMAGEREPRESENTATIONGL_H__

#include "tgt/tgt_gl.h"
#include "tgt/texture.h"
#include "core/datastructures/genericabstractimagerepresentation.h"
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
    class CAMPVIS_CORE_API ImageRepresentationGL : public GenericAbstractImageRepresentation<ImageRepresentationGL> {
    public:
        /**
         * Creates a new ImageRepresentationGL representation from a tgt::Texture and automatically
         * adds it to \a parent which will take ownerwhip.
         *
         * \note    You do \b not own the returned pointer.
         * 
         * \param   parent  Image this representation represents, must not be 0, will take ownership of the returned pointer.
         * \param   texture OpenGL texture to use, must not be 0, ImageRepresentationGL will take ownership of this texture.
         * \return  A pointer to the newly created ImageRepresentationGL, you do \b not own this pointer!
         */
        static ImageRepresentationGL* create(ImageData* parent, tgt::Texture* texture);        

        /**
         * Creates a new ImageRepresentationGL representation from a tgt::Texture and automatically
         * adds it to \a parent which will take ownerwhip.
         *
         * \note    You do \b not own the returned pointer.
         * 
         * \param   parent  Image this representation represents, must not be 0, will take ownership of the returned pointer.
         * \param   wtp     WeaklyTypedPointer to the image data, must not be 0, ImageRepresentationGL does \b not take ownership of that pointer.
         * \return  A pointer to the newly created ImageRepresentationGL, you do \b not own this pointer!
         */
        static ImageRepresentationGL* create(ImageData* parent, const WeaklyTypedPointer& wtp);        

        /**
         * Destructor
         */
        virtual ~ImageRepresentationGL();


        /**
         * Performs a conversion of \a source to an ImageRepresentationLocal if feasible.
         * Returns 0 if conversion was not successful or source representation type is not compatible.
         * \note    The callee, respectively the callee's parent, has the ownership of the returned pointer.
         * \param   source  Source image representation for conversion.
         * \return  A pointer to a local representation of \a source or 0 on failure. The caller does \b not have ownership.
         */
        static ImageRepresentationGL* tryConvertFrom(const AbstractImageRepresentation* source);
       
        /**
         * \see AbstractImageRepresentation::clone()
         **/
        virtual ImageRepresentationGL* clone(ImageData* newParent) const;

        /// \see AbstractImageRepresentation::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;

        /// \see AbstractImageRepresentation::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;

        /**
         * \see AbstractImageRepresentation::getSubImage
         */
        virtual ImageRepresentationGL* getSubImage(ImageData* parent, const tgt::svec3& llf, const tgt::svec3& urb) const;


        /**
         * Binds the image to the given shader using \a texUnit as texture unit and \a texUniform as shader uniform.
         * \param   shader              Shader to bind image and set uniforms to.
         * \param   texUnit             TextureUnit to bind image to.
         * \param   texUniform          Uniform name for the sampler.
         * \param   texParamsUniform    Name for texture parameters struct uniform.
         */
        void bind(
            tgt::Shader* shader,
            const tgt::TextureUnit& texUnit, 
            const std::string& texUniform = "_texture",
            const std::string& texParamsUniform = "_textureParams") const;

        /**
         * Unbinds the currently bound image from the current texture unit.
         * \note    This is not necessarily this texture but the texture of the same type bound to this texture unit!
         */ 
        void unbind() const;

        /**
         * Download the OpenGL texture from the GPU memory to the texture cpu buffer.
         * \return  _texture
         */
        void downloadTexture() const;

        /**
         * Gets the OpenGL texture.
         * \return  _texture
         */
        const tgt::Texture* getTexture() const;


        /**
         * Returns a WeaklyTypedPointer to the data of this representation.
         * You do \b not own the pointer - do \b not modify its content!
         * \see     ImageRepresentationGL::getWeaklyTypedPointerCopy()
         * \note    Make sure to call this method from a valid OpenGL context.
         * \return  A WeaklyTypedPointer to the data of this representation. Neither you own nor you may modify its data!
         */
        const WeaklyTypedPointer getWeaklyTypedPointer() const;

        /**
         * Returns a WeaklyTypedPointer to the data of this representation.
         * Caller will own the pointer - take care to eventually delete it!
         * \see     ImageRepresentationGL::getWeaklyTypedPointer()
         * \note    Make sure to call this method from a valid OpenGL context.
         * \return  A WeaklyTypedPointer to the data of this representation. Caller takes ownership!
         */
        const WeaklyTypedPointer getWeaklyTypedPointerCopy() const;


    protected:
        /**
         * Creates a new ImageRepresentationGL representation from a tgt::Texture.
         * 
         * \param   parent      Image this representation represents, must not be 0.
         * \param   texture     OpenGL texture to use, must not be 0, ImageRepresentationGL will take ownership of this texture.
         */
        ImageRepresentationGL(ImageData* parent, tgt::Texture* texture);

        /**
         * Creates a new ImageRepresentationGL representation.
         *
         * \param   parent  Image this representation represents, must not be 0.
         * \param   wtp     WeaklyTypedPointer to the image data, must not be 0, ImageRepresentationGL does \b not take ownership of that pointer.
         */
        ImageRepresentationGL(ImageData* parent, const WeaklyTypedPointer& wtp);

        /**
         * Creates a new ImageRepresentationGL representation from GenericImageRepresentationLocal.
         *
         * \param   parent      Image this representation represents, must not be 0.
         * \param   data        Pointer to the GenericImageRepresentationLocal instance, must not be 0
         * \tparam  BASETYPE        Base type of image data
         * \tparam  NUMCHANNELS     Number of channels per element
         */
        template<typename BASETYPE, size_t NUMCHANNELS>
        ImageRepresentationGL(ImageData* parent, const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* data);

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
    campvis::ImageRepresentationGL::ImageRepresentationGL(ImageData* parent, const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* data) 
        : AbstractImageRepresentation(parent)
    {
        createTexture<BASETYPE, NUMCHANNELS>(data);
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    void campvis::ImageRepresentationGL::createTexture(const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* data) {
        tgtAssert(data != 0, "Pointer to image must not be 0!");

        _texture = new tgt::Texture(
            reinterpret_cast<GLubyte*>(data->getImageData()), 
            this->getSize(),
            TypeTraits<BASETYPE, NUMCHANNELS>::glFormat,
            TypeTraits<BASETYPE, NUMCHANNELS>::glInternalFormat, 
            TypeTraits<BASETYPE, NUMCHANNELS>::glDataType, 
            tgt::Texture::LINEAR);

        setupAndUploadTexture(_texture, TypeTraitsHelperPerBasetype<BASETYPE>::isInteger, TypeTraitsHelperPerBasetype<BASETYPE>::isSigned);
    }

}

#endif // IMAGEREPRESENTATIONGL_H__
