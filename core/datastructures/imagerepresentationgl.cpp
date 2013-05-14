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

#include "imagerepresentationgl.h"

#include "tgt/assert.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"
#include "tgt/tgt_gl.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationdisk.h"
#include "core/datastructures/imagerepresentationlocal.h"
#include "core/datastructures/imagerepresentationrendertarget.h"
#ifdef CAMPVIS_HAS_MODULE_ITK
#include "modules/itk/core/genericimagerepresentationitk.h"
#endif

namespace campvis {

    const std::string ImageRepresentationGL::loggerCat_ = "CAMPVis.core.datastructures.ImageRepresentationGL";

    ImageRepresentationGL* ImageRepresentationGL::create(ImageData* parent, tgt::Texture* texture) {
        ImageRepresentationGL* toReturn = new ImageRepresentationGL(parent, texture);
        toReturn->addToParent();
        return toReturn;
    }

    ImageRepresentationGL* ImageRepresentationGL::create(ImageData* parent, const WeaklyTypedPointer& wtp) {
        ImageRepresentationGL* toReturn = new ImageRepresentationGL(parent, wtp);
        toReturn->addToParent();
        return toReturn;
    }

    ImageRepresentationGL::ImageRepresentationGL(ImageData* parent, tgt::Texture* texture)
        : GenericAbstractImageRepresentation<ImageRepresentationGL>(parent)
        , _texture(texture)
    {
        tgtAssert(texture != 0, "Given texture must not be 0.");
    }

    ImageRepresentationGL::ImageRepresentationGL(ImageData* parent, const WeaklyTypedPointer& wtp) 
        : GenericAbstractImageRepresentation<ImageRepresentationGL>(parent)
    {
        createTexture(wtp);
    }

    ImageRepresentationGL::~ImageRepresentationGL() {
        delete _texture;
    }

    ImageRepresentationGL* ImageRepresentationGL::tryConvertFrom(const AbstractImageRepresentation* source) {
        if (source == 0)
            return 0;

        // test source image type via dynamic cast
        if (const ImageRepresentationDisk* tester = dynamic_cast<const ImageRepresentationDisk*>(source)) {
            WeaklyTypedPointer wtp = tester->getImageData();
            ImageRepresentationGL* toReturn = ImageRepresentationGL::create(const_cast<ImageData*>(tester->getParent()), wtp);
            delete wtp._pointer;
            return toReturn;
        }
        else if (const ImageRepresentationLocal* tester = dynamic_cast<const ImageRepresentationLocal*>(source)) {
            ImageRepresentationGL* toReturn = ImageRepresentationGL::create(const_cast<ImageData*>(tester->getParent()), tester->getWeaklyTypedPointer());
            return toReturn;
        }
#ifdef CAMPVIS_HAS_MODULE_ITK
        else if (const AbstractImageRepresentationItk* tester = dynamic_cast<const AbstractImageRepresentationItk*>(source)) {
            ImageRepresentationGL* toReturn = ImageRepresentationGL::create(const_cast<ImageData*>(tester->getParent()), tester->getWeaklyTypedPointer());
            return toReturn;
        }
#endif

        return 0;
    }

    ImageRepresentationGL* ImageRepresentationGL::clone(ImageData* newParent) const {
        GLubyte* data = _texture->downloadTextureToBuffer();
        WeaklyTypedPointer wtp(WeaklyTypedPointer::baseType(_texture->getDataType()), WeaklyTypedPointer::numChannels(_texture->getFormat()), data);
        ImageRepresentationGL* toReturn = ImageRepresentationGL::create(newParent, wtp);
        delete data;
        return toReturn;
    }

    ImageRepresentationGL* ImageRepresentationGL::getSubImage(ImageData* parent, const tgt::svec3& llf, const tgt::svec3& urb) const {
        // TODO: implement
        //LWARNING("ImageRepresentationGL::getSubImage() not implemented!");
        return 0;
    }

    void ImageRepresentationGL::createTexture(const WeaklyTypedPointer& wtp) {
        tgtAssert(wtp._pointer != 0, "Pointer to image data must not be 0!");

        _texture = new tgt::Texture(reinterpret_cast<GLubyte*>(wtp._pointer), getSize(), wtp.getGlFormat(), wtp.getGlInternalFormat(), wtp.getGlDataType(), tgt::Texture::LINEAR);
        setupAndUploadTexture(_texture, wtp.isInteger(), wtp.isSigned());

    }

    void ImageRepresentationGL::setupAndUploadTexture(tgt::Texture* texture, bool isInteger, bool isSigned) {
        // Set OpenGL pixel alignment to 1 to avoid problems with NPOT textures
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        switch (getDimensionality()) {
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

        tgt::TextureUnit tempUnit;
        tempUnit.activate();
        _texture->bind();

        // map signed integer types from [-1.0:1.0] to [0.0:1.0] in order to avoid clamping of negative values
        if (isInteger && isSigned) {
            glPixelTransferf(GL_RED_SCALE,   0.5f);
            glPixelTransferf(GL_GREEN_SCALE, 0.5f);
            glPixelTransferf(GL_BLUE_SCALE,  0.5f);
            glPixelTransferf(GL_ALPHA_SCALE, 0.5f);

            glPixelTransferf(GL_RED_BIAS,    0.5f);
            glPixelTransferf(GL_GREEN_BIAS,  0.5f);
            glPixelTransferf(GL_BLUE_BIAS,   0.5f);
            glPixelTransferf(GL_ALPHA_BIAS,  0.5f);

            //_mappingInformation.setRealWorldMapping(LinearMapping<float>(.5f, .5f));
        }

        _texture->uploadTexture();
        _texture->setWrapping(tgt::Texture::CLAMP);

        if (isInteger && isSigned) {
            // restore default
            glPixelTransferf(GL_RED_SCALE,   1.0f);
            glPixelTransferf(GL_GREEN_SCALE, 1.0f);
            glPixelTransferf(GL_BLUE_SCALE,  1.0f);
            glPixelTransferf(GL_ALPHA_SCALE, 1.0f);

            glPixelTransferf(GL_RED_BIAS,    0.0f);
            glPixelTransferf(GL_GREEN_BIAS,  0.0f);
            glPixelTransferf(GL_BLUE_BIAS,   0.0f);
            glPixelTransferf(GL_ALPHA_BIAS,  0.0f);
        }

        // revoke ownership of local pixel data from the texture
        _texture->setPixelData(0);

        tgt::TextureUnit::setZeroUnit();
        LGL_ERROR;
    }

    void ImageRepresentationGL::bind() const {
        _texture->bind();
    }

    void ImageRepresentationGL::bind(const tgt::TextureUnit& texUnit) const {
        texUnit.activate();
        _texture->bind();
    }

    void ImageRepresentationGL::bind(tgt::Shader* shader, const tgt::TextureUnit& texUnit, const std::string& texUniform /*= "_texture"*/, const std::string& texParamsUniform) const {
        bind(texUnit);
        bool tmp = shader->getIgnoreUniformLocationError();
        shader->setIgnoreUniformLocationError(true);

        switch (getDimensionality()) {
            case 1:
                LERROR("Setting um 1D texture uniforms currently not implemented - you probably wanna do that yourself...");
                break;

            case 2:
                shader->setUniform(texUniform, texUnit.getUnitNumber());
                shader->setUniform(texParamsUniform + "._size", tgt::vec2(getSize().xy()));
                shader->setUniform(texParamsUniform + "._sizeRCP", tgt::vec2(1.f) / tgt::vec2(getSize().xy()));
                shader->setUniform(texParamsUniform + "._numChannels", static_cast<int>(_parent->getNumChannels()));
                break;

            case 3:
                shader->setUniform(texUniform, texUnit.getUnitNumber());
                shader->setUniform(texParamsUniform + "._size", tgt::vec3(getSize()));
                shader->setUniform(texParamsUniform + "._sizeRCP", tgt::vec3(1.f) / tgt::vec3(getSize()));
                shader->setUniform(texParamsUniform + "._numChannels", static_cast<int>(_parent->getNumChannels()));
                shader->setUniform(texParamsUniform + "._voxelSize", _parent->getMappingInformation().getVoxelSize());
                shader->setUniform(texParamsUniform + "._voxelSizeRCP", tgt::vec3(1.f) / _parent->getMappingInformation().getVoxelSize());
                shader->setUniform(texParamsUniform + "._textureToWorldMatrix", _parent->getMappingInformation().getTextureToWorldMatrix());
                shader->setUniform(texParamsUniform + "._realWorldMapping", tgt::vec2(_parent->getMappingInformation().getRealWorldMapping()._shift, _parent->getMappingInformation().getRealWorldMapping()._scale));
                break;

            default:
                tgtAssert(false, "Should not reach this!");
                break;
        }
        shader->setIgnoreUniformLocationError(tmp);
        LGL_ERROR;
    }

    const tgt::Texture* ImageRepresentationGL::getTexture() const {
        return _texture;
    }

    size_t ImageRepresentationGL::getLocalMemoryFootprint() const {
        size_t sum = 0;
        if (_texture != 0) {
            sum += sizeof(tgt::Texture);
            if (_texture->getPixelData() != 0) {
                sum += _texture->getBpp() + _texture->getArraySize();
            }
        }

        return sizeof(*this) + sum;
    }

    size_t ImageRepresentationGL::getVideoMemoryFootprint() const {
        return _texture->getSizeOnGPU();
    }

    const WeaklyTypedPointer ImageRepresentationGL::getWeaklyTypedPointer() const {
        if (_texture->getPixelData() == 0) {
            _texture->downloadTexture();
        }
        return WeaklyTypedPointer(WeaklyTypedPointer::baseType(_texture->getDataType()), _texture->getNumChannels(), _texture->getPixelData());
    }


}