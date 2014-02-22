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

#include "renderdata.h"

#include "tgt/framebufferobject.h"
#include "tgt/textureunit.h"
#include "tgt/shadermanager.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"

namespace campvis {

    const std::string RenderData::loggerCat_ = "CAMPVis.core.datastructures.RenderData";

    RenderData::RenderData()
        : AbstractData()
        , _depthTexture(0)
    {

    }

    RenderData::RenderData(const tgt::FramebufferObject* fbo)
        : AbstractData()
        , _depthTexture(0)
    {
        tgt::Texture* const * attachments = fbo->getAttachments();
        for (size_t i = 0; i < TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS; ++i) {
            if (attachments[i] != 0) {
                ImageData* img = new ImageData(2, attachments[i]->getDimensions(), attachments[i]->getNumChannels());
                ImageRepresentationGL* rep = ImageRepresentationGL::create(img, attachments[i]);
                _colorTextures.push_back(DataHandle(img));
            }
        }
        if (attachments[TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS] != 0) {
            ImageData* img = new ImageData(2, attachments[TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS]->getDimensions(), attachments[TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS]->getNumChannels());
            ImageRepresentationGL* rep = ImageRepresentationGL::create(img, attachments[TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS]);
            _depthTexture = DataHandle(img);
        }
    }

    RenderData::~RenderData() {

    }

    RenderData* RenderData::clone() const {
        RenderData* toReturn = new RenderData();
        toReturn->_colorTextures = _colorTextures;
        toReturn->_depthTexture = _depthTexture;
        return toReturn;
    }

    size_t RenderData::getLocalMemoryFootprint() const {
        size_t sum = sizeof(RenderData);

        for (std::vector<DataHandle>::const_iterator it = _colorTextures.begin(); it != _colorTextures.end(); ++it)
            if (it->getData() != 0)
                sum += it->getData()->getLocalMemoryFootprint() + sizeof(DataHandle);

        if (_depthTexture.getData() != 0)
            sum += _depthTexture.getData()->getLocalMemoryFootprint() + sizeof(DataHandle);

        return sum;
    }

    size_t RenderData::getVideoMemoryFootprint() const {
        size_t sum = 0;
        for (std::vector<DataHandle>::const_iterator it = _colorTextures.begin(); it != _colorTextures.end(); ++it)
            if (it->getData() != 0)
                sum += it->getData()->getVideoMemoryFootprint();

        if (_depthTexture.getData() != 0)
            sum += _depthTexture.getData()->getVideoMemoryFootprint();

        return sum;
    }

    size_t RenderData::getNumColorTextures() const {
        return _colorTextures.size();
    }

    const ImageData* RenderData::getColorTexture(size_t index /*= 0*/) const {
        tgtAssert(index < _colorTextures.size(), "Index out of bounds.");
        if (index >= _colorTextures.size())
            return 0;

        return static_cast<const ImageData*>(_colorTextures[index].getData());
    }

    campvis::DataHandle RenderData::getColorDataHandle(size_t index /*= 0*/) const {
        tgtAssert(index < _colorTextures.size(), "Index out of bounds.");
        if (index >= _colorTextures.size())
            return DataHandle(0);

        return _colorTextures[index];
    }

    bool RenderData::hasDepthTexture() const {
        return _depthTexture.getData() != 0;
    }

    const ImageData* RenderData::getDepthTexture() const {
        const AbstractData* d = _depthTexture.getData();
        if (d == 0)
            return 0;

        return static_cast<const ImageData*>(d);
    }

    campvis::DataHandle RenderData::getDepthDataHandle() const {
        return _depthTexture;
    }

    void RenderData::addColorTexture(ImageData* texture) {
        _colorTextures.push_back(DataHandle(texture));
    }

    void RenderData::setDepthTexture(ImageData* texture) {
        _depthTexture = DataHandle(texture);
    }

    void RenderData::bindColorTexture(tgt::Shader* shader, const tgt::TextureUnit& colorTexUnit, const std::string& colorTexUniform /*= "_colorTexture"*/, const std::string& texParamsUniform /*= "_texParams"*/, size_t index /*= 0*/) const {
        tgtAssert(index < _colorTextures.size(), "Index out of bounds.");
        if (index >= _colorTextures.size())
            return;

        const ImageData* id = static_cast<const ImageData*>(_colorTextures[index].getData());
        tgtAssert(id != 0, "WTF, color texture with 0 pointer?!");

        const ImageRepresentationGL* rep = id->getRepresentation<ImageRepresentationGL>(true);
        rep->bind(shader, colorTexUnit, colorTexUniform, texParamsUniform);
    }

    void RenderData::bindDepthTexture(tgt::Shader* shader, const tgt::TextureUnit& depthTexUnit, const std::string& depthTexUniform /*= "_depthTexture"*/, const std::string& texParamsUniform /*= "_texParams"*/) const {
        tgtAssert(_depthTexture.getData() != 0, "Empty Depth Texture!");
        if (_depthTexture.getData() == 0)
            return;

        const ImageData* id = static_cast<const ImageData*>(_depthTexture.getData());
        const ImageRepresentationGL* rep = id->getRepresentation<ImageRepresentationGL>(true);
        rep->bind(shader, depthTexUnit, depthTexUniform, texParamsUniform);
    }

    void RenderData::bind(tgt::Shader* shader, const tgt::TextureUnit& colorTexUnit, const tgt::TextureUnit& depthTexUnit, const std::string& colorTexUniform /*= "_colorTexture"*/, const std::string& depthTexUniform /*= "_depthTexture"*/, const std::string& texParamsUniform /*= "_texParams"*/, size_t index /*= 0*/) const {
        if (hasDepthTexture())
            bindDepthTexture(shader, depthTexUnit, depthTexUniform, texParamsUniform);
        bindColorTexture(shader, colorTexUnit, colorTexUniform, texParamsUniform, index);
    }

}