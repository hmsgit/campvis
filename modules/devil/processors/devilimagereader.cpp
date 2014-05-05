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

#include "devilimagereader.h"


#include <IL/il.h>
#include <cstring>

#include "tgt/logmanager.h"
#include "tgt/filesystem.h"
#include "tgt/shadermanager.h"
#include "tgt/texturereaderdevil.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

#include "core/tools/quadrenderer.h"


namespace campvis {
    const std::string DevilImageReader::loggerCat_ = "CAMPVis.modules.io.DevilImageReader";

    GenericOption<std::string> importOptions[3] = {
        GenericOption<std::string>("rt", "Render Target"),
        GenericOption<std::string>("texture", "OpenGL Texture"),
        GenericOption<std::string>("localIntensity", "Local Intensity Image")
    };

    DevilImageReader::DevilImageReader(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_url("url", "Image URL", "", StringProperty::OPEN_FILENAME)
        , p_targetImageID("targetImageName", "Target Image ID", "DevilImageReader.output", DataNameProperty::WRITE)
        , p_importType("ImportType", "Import Type", importOptions, 3)
        , _shader(nullptr)
        , _devilTextureReader(nullptr)
    {
        addProperty(p_url);
        addProperty(p_targetImageID);
        addProperty(p_importType);

        _devilTextureReader = new tgt::TextureReaderDevil();
    }

    DevilImageReader::~DevilImageReader() {
        delete _devilTextureReader;
    }

    void DevilImageReader::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "core/glsl/copyimage.frag", "#define NO_DEPTH\n");
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void DevilImageReader::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void DevilImageReader::updateResult(DataContainer& data) {
        tgt::Texture* tex = _devilTextureReader->loadTexture(p_url.getValue(), tgt::Texture::LINEAR, false, true, true, false);
        if (tex != 0) {
            if (p_importType.getOptionValue() == "rt") {
                ImageData id (2, tex->getDimensions(), tex->getNumChannels());
                ImageRepresentationGL* image = ImageRepresentationGL::create(&id, tex);

                FramebufferActivationGuard fag(this);
                createAndAttachColorTexture();
                createAndAttachDepthTexture();

                _shader->activate();
                _shader->setIgnoreUniformLocationError(true);
                _shader->setUniform("_viewportSize", getEffectiveViewportSize());
                _shader->setUniform("_viewportSizeRCP", 1.f / tgt::vec2(getEffectiveViewportSize()));
                _shader->setIgnoreUniformLocationError(false);
                tgt::TextureUnit texUnit;

                image->bind(_shader, texUnit, "_colorTexture");

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                QuadRdr.renderQuad();

                _shader->deactivate();
                tgt::TextureUnit::setZeroUnit();
                LGL_ERROR;

                data.addData(p_targetImageID.getValue(), new RenderData(_fbo));
            }
            else if (p_importType.getOptionValue() == "texture") {
                ImageData* id = new ImageData(2, tex->getDimensions(), tex->getNumChannels());
                ImageRepresentationGL::create(id, tex);
                data.addData(p_targetImageID.getValue(), id);
            }
            else if (p_importType.getOptionValue() == "localIntensity") {
                // TODO: Clean up pre-MICCAI mess!
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                tex->downloadTexture();
                size_t numElements = tgt::hmul(tex->getDimensions());
                ImageData* id = new ImageData(2, tex->getDimensions(), 1);

                // TODO: use macro magic to switch through the different data types and number of channels
                if (tex->getDataType() == GL_UNSIGNED_BYTE && tex->getNumChannels() == 3) {
                    tgt::Vector3<uint8_t>* data = reinterpret_cast<tgt::Vector3<uint8_t>*>(tex->getPixelData());
                    uint8_t* copy = new uint8_t[numElements];
                    for (size_t i = 0; i < numElements; ++i) {
                        copy[i] = data[i].x;
                    }
                    GenericImageRepresentationLocal<uint8_t, 1>::create(id, copy);
                }

                delete tex;
                data.addData(p_targetImageID.getValue(), id);
            }
        }
        else {
            LERROR("Could not load image.");
        }

        validate(INVALID_RESULT);
    }
}