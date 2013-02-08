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
#include "tgt/texturereaderdevil.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/imagerepresentationrendertarget.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

#include "core/tools/quadrenderer.h"


namespace campvis {
    const std::string DevilImageReader::loggerCat_ = "CAMPVis.modules.io.DevilImageReader";

    DevilImageReader::DevilImageReader(GenericProperty<tgt::ivec2>& canvasSize)
        : VisualizationProcessor(canvasSize)
        , p_url("url", "Image URL", "")
        , p_targetImageID("targetImageName", "Target Image ID", "DevilImageReader.output", DataNameProperty::WRITE)
        , p_useRenderTarget("UseRenderTarget", "Read Into RenderTarget", false)
        , _devilTextureReader(0)
    {
        addProperty(&p_url);
        addProperty(&p_targetImageID);
        addProperty(&p_useRenderTarget);

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
            if (p_useRenderTarget.getValue()) {
                ImageData id (2, tex->getDimensions(), tex->getNumChannels());
                ImageRepresentationGL* image = ImageRepresentationGL::create(&id, tex);

                std::pair<ImageData*, ImageRepresentationRenderTarget*> rt = ImageRepresentationRenderTarget::createWithImageData(_renderTargetSize.getValue());
                glPushAttrib(GL_ALL_ATTRIB_BITS);
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_ALWAYS);

                _shader->activate();
                _shader->setIgnoreUniformLocationError(true);
                _shader->setUniform("_viewportSize", _renderTargetSize.getValue());
                _shader->setUniform("_viewportSizeRCP", 1.f / tgt::vec2(_renderTargetSize.getValue()));
                _shader->setIgnoreUniformLocationError(false);
                tgt::TextureUnit texUnit;

                image->bind(_shader, texUnit, "_colorTexture");

                rt.second->activate();
                LGL_ERROR;
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                QuadRdr.renderQuad();
                rt.second->deactivate();

                _shader->deactivate();
                tgt::TextureUnit::setZeroUnit();
                glPopAttrib();
                LGL_ERROR;

                data.addData(p_targetImageID.getValue(), rt.first);
                p_targetImageID.issueWrite();
            }
            else {
                ImageData* id = new ImageData(2, tex->getDimensions(), tex->getNumChannels());
                ImageRepresentationGL::create(id, tex);
                data.addData(p_targetImageID.getValue(), id);
                p_targetImageID.issueWrite();
            }
        }
        else {
            LERROR("Could not load image.");
        }

        applyInvalidationLevel(InvalidationLevel::VALID);
    }
}