// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#include "imagerepresentationgl.h"

#include "cgt/assert.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"
#include "cgt/cgt_gl.h"

#include "core/datastructures/imagedata.h"

namespace campvis {

    const std::string ImageRepresentationGL::loggerCat_ = "CAMPVis.core.datastructures.ImageRepresentationGL";

    ImageRepresentationGL* ImageRepresentationGL::create(ImageData* parent, cgt::Texture* texture) {
        ImageRepresentationGL* toReturn = new ImageRepresentationGL(parent, texture);
        toReturn->addToParent();
        return toReturn;
    }

    ImageRepresentationGL* ImageRepresentationGL::create(ImageData* parent, const WeaklyTypedPointer& wtp) {
        ImageRepresentationGL* toReturn = new ImageRepresentationGL(parent, wtp);
        toReturn->addToParent();
        return toReturn;
    }

    ImageRepresentationGL::ImageRepresentationGL(ImageData* parent, cgt::Texture* texture)
        : GenericAbstractImageRepresentation<ImageRepresentationGL>(parent)
        , _texture(texture)
    {
        cgtAssert(texture != 0, "Given texture must not be 0.");
        cgtAssert(parent->getDimensionality() >= 3 || texture->getDimensions().z == 1, "Dimensionality of Parent and texture mismatch!");
        cgtAssert(parent->getDimensionality() >= 2 || texture->getDimensions().y == 1, "Dimensionality of Parent and texture mismatch!");
        cgtAssert(parent->getNumChannels() == texture->getNumChannels(), "Number of Channels of parent and texture mismatch!");
    }

    ImageRepresentationGL::ImageRepresentationGL(ImageData* parent, const WeaklyTypedPointer& wtp) 
        : GenericAbstractImageRepresentation<ImageRepresentationGL>(parent)
    {
        cgtAssert(wtp._numChannels == parent->getNumChannels(), "Number of Channels of parent and texture mismatch!");
        createTexture(wtp);
    }

    ImageRepresentationGL::~ImageRepresentationGL() {
        delete _texture;
    }

    ImageRepresentationGL* ImageRepresentationGL::clone(ImageData* newParent) const {
        GLubyte* data = _texture->downloadTextureToBuffer();
        WeaklyTypedPointer wtp(WeaklyTypedPointer::baseType(_texture->getDataType()), WeaklyTypedPointer::numChannels(_texture->getFormat()), data);
        ImageRepresentationGL* toReturn = ImageRepresentationGL::create(newParent, wtp);
        delete data;
        return toReturn;
    }

    void ImageRepresentationGL::createTexture(const WeaklyTypedPointer& wtp) {
        cgtAssert(wtp._pointer != 0, "Pointer to image data must not be 0!");

        _texture = new cgt::Texture(reinterpret_cast<GLubyte*>(wtp._pointer), getSize(), wtp.getGlFormat(), wtp.getGlInternalFormat(), wtp.getGlDataType(), cgt::Texture::LINEAR);
        setupAndUploadTexture(_texture, wtp.isInteger(), wtp.isSigned());

    }

    void ImageRepresentationGL::setupAndUploadTexture(cgt::Texture* texture, bool isInteger, bool isSigned) {
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
                cgtAssert(false, "Unsupported dimensionality of image.");
                break;
        }

        cgt::TextureUnit tempUnit;
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
        _texture->setWrapping(cgt::Texture::CLAMP_TO_EDGE);

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

        cgt::TextureUnit::setZeroUnit();
        LGL_ERROR;
    }

    void ImageRepresentationGL::bind() const {
        _texture->bind();
    }

    void ImageRepresentationGL::bind(const cgt::TextureUnit& texUnit) const {
        texUnit.activate();
        _texture->bind();
    }

    void ImageRepresentationGL::bind(cgt::Shader* shader, const cgt::TextureUnit& texUnit, const std::string& texUniform /*= "_texture"*/, const std::string& texParamsUniform) const {
        bind(texUnit);
        bool tmp = shader->getIgnoreUniformLocationError();
        shader->setIgnoreUniformLocationError(true);

        switch (getDimensionality()) {
            case 1:
                LERROR("Setting um 1D texture uniforms currently not implemented - you probably wanna do that yourself...");
                break;

            case 2:
                shader->setUniform(texUniform, texUnit.getUnitNumber());
                shader->setUniform(texParamsUniform + "._size", cgt::vec2(getSize().xy()));
                shader->setUniform(texParamsUniform + "._sizeRCP", cgt::vec2(1.f) / cgt::vec2(getSize().xy()));
                shader->setUniform(texParamsUniform + "._numChannels", static_cast<int>(_parent->getNumChannels()));
                break;

            case 3:
                shader->setUniform(texUniform, texUnit.getUnitNumber());
                shader->setUniform(texParamsUniform + "._size", cgt::vec3(getSize()));
                shader->setUniform(texParamsUniform + "._sizeRCP", cgt::vec3(1.f) / cgt::vec3(getSize()));
                shader->setUniform(texParamsUniform + "._numChannels", static_cast<int>(_parent->getNumChannels()));
                shader->setUniform(texParamsUniform + "._voxelSize", _parent->getMappingInformation().getVoxelSize());
                shader->setUniform(texParamsUniform + "._voxelSizeRCP", cgt::vec3(1.f) / _parent->getMappingInformation().getVoxelSize());
                shader->setUniform(texParamsUniform + "._textureToWorldMatrix", _parent->getMappingInformation().getTextureToWorldMatrix());
                shader->setUniform(texParamsUniform + "._worldToTextureMatrix", _parent->getMappingInformation().getWorldToTextureMatrix());
                shader->setUniform(texParamsUniform + "._realWorldMapping", cgt::vec2(_parent->getMappingInformation().getRealWorldMapping()._shift, _parent->getMappingInformation().getRealWorldMapping()._scale));
                break;

            default:
                cgtAssert(false, "Should not reach this!");
                break;
        }
        shader->setIgnoreUniformLocationError(tmp);
        LGL_ERROR;
    }

    void ImageRepresentationGL::downloadTexture() const {
        _texture->downloadTexture();
    }

    const cgt::Texture* ImageRepresentationGL::getTexture() const {

        return _texture;
    }

    size_t ImageRepresentationGL::getLocalMemoryFootprint() const {
        size_t sum = 0;
        if (_texture != 0) {
            sum += sizeof(cgt::Texture);
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

    void ImageRepresentationGL::unbind() const {
        _texture->unbind();
    }

    const WeaklyTypedPointer ImageRepresentationGL::getWeaklyTypedPointerCopy() const {
        void* ptr = _texture->downloadTextureToBuffer(_texture->getFormat(), _texture->getDataType());
        return WeaklyTypedPointer(WeaklyTypedPointer::baseType(_texture->getDataType()), _texture->getNumChannels(), ptr);
    }

    const WeaklyTypedPointer ImageRepresentationGL::getWeaklyTypedPointerConvert(GLenum dataType) const {
        void* ptr = _texture->downloadTextureToBuffer(_texture->getFormat(), dataType);
        return WeaklyTypedPointer(WeaklyTypedPointer::baseType(dataType), _texture->getNumChannels(), ptr);
    }

}