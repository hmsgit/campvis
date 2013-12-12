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

#include "glgradientvolumegenerator.h"

#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorgradient.h"

#include "core/classification/simpletransferfunction.h"
#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

#include "core/tools/quadrenderer.h"

namespace campvis {

    const std::string GlGradientVolumeGenerator::loggerCat_ = "CAMPVis.modules.classification.GlGradientVolumeGenerator";

    GlGradientVolumeGenerator::GlGradientVolumeGenerator(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputImage("InputImage", "Input Image", "", DataNameProperty::READ, AbstractProcessor::INVALID_PROPERTIES | AbstractProcessor::INVALID_RESULT)
        , p_outputImage("OutputImage", "Output Image", "GlGradientVolumeGenerator.out", DataNameProperty::WRITE)
        , _shader(0)
    {
        addDecorator(new ProcessorDecoratorGradient());

        addProperty(&p_inputImage);
        addProperty(&p_outputImage);
        decoratePropertyCollection(this);
    }

    GlGradientVolumeGenerator::~GlGradientVolumeGenerator() {

    }

    void GlGradientVolumeGenerator::init() {
        VisualizationProcessor::init();

        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/preprocessing/glsl/glgradientvolumegenerator.frag", generateHeader(), false);
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void GlGradientVolumeGenerator::deinit() {
        ShdrMgr.dispose(_shader);
        VisualizationProcessor::deinit();
    }

    void GlGradientVolumeGenerator::process(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_inputImage.getValue());

        if (img != 0) {
            if (hasInvalidShader()) {
                _shader->setHeaders(generateHeader());
                _shader->rebuild();
                validate(INVALID_SHADER);
            }

            const tgt::svec3& size = img->getSize();
            tgt::ivec2 viewportSize = size.xy();

            tgt::TextureUnit inputUnit;
            inputUnit.activate();

            // create texture for result
            tgt::Texture* resultTexture = new tgt::Texture(0, tgt::ivec3(size), GL_RGB, GL_RGB32F, GL_FLOAT, tgt::Texture::LINEAR);
            resultTexture->uploadTexture();

            // activate shader and bind textures
            _shader->activate();
            img->bind(_shader, inputUnit);

            // activate FBO and attach texture
            _fbo->activate();
            glViewport(0, 0, static_cast<GLsizei>(viewportSize.x), static_cast<GLsizei>(viewportSize.y));

            // render quad to compute difference measure by shader
            for (int z = 0; z < size.z; ++z) {
                float zTexCoord = static_cast<float>(z)/static_cast<float>(size.z) + .5f/static_cast<float>(size.z);
                _shader->setUniform("_zTexCoord", zTexCoord);
                _fbo->attachTexture(resultTexture, GL_COLOR_ATTACHMENT0, 0, z);
                QuadRdr.renderQuad();
            }
            _fbo->detachAll();
            _fbo->deactivate();
            _shader->deactivate();

            // put resulting image into DataContainer
            ImageData* id = new ImageData(3, size, 1);
            ImageRepresentationGL::create(id, resultTexture);
            id->setMappingInformation(img->getParent()->getMappingInformation());
            data.addData(p_outputImage.getValue(), id);

            tgt::TextureUnit::setZeroUnit();
            LGL_ERROR;
        }
        else {
            LERROR("No suitable input image found.");
        }

        validate(INVALID_RESULT);
    }

    std::string GlGradientVolumeGenerator::generateHeader() const {
        return getDecoratedHeader();
    }

}
