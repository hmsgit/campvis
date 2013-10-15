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
        , p_url("url", "Image URL", "")
        , p_targetImageID("targetImageName", "Target Image ID", "DevilImageReader.output", DataNameProperty::WRITE)
        , p_importType("ImportType", "Import Type", importOptions, 3)
        , _devilTextureReader(0)
    {
        addProperty(&p_url);
        addProperty(&p_targetImageID);
        addProperty(&p_importType);

        _devilTextureReader = new tgt::TextureReaderDevil();
    }

    DevilImageReader::~DevilImageReader() {
        delete _devilTextureReader;
    }

    void DevilImageReader::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "core/glsl/copyimage.frag", "#define NO_DEPTH\n", false);
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void DevilImageReader::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void DevilImageReader::process(DataContainer& data) {
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