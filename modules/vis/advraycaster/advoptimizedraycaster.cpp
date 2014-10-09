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

#include "AdvOptimizedRaycaster.h"

#include "core/tools/quadrenderer.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorgradient.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/lightsourcedata.h"
#include "core/datastructures/geometrydatafactory.h"

#include "modules/vis/advraycaster/voxeltexturemipmapping.h"

#include <tbb/tbb.h>

namespace campvis {
    const std::string AdvOptimizedRaycaster::loggerCat_ = "CAMPVis.modules.vis.AdvOptimizedRaycaster";

    AdvOptimizedRaycaster::AdvOptimizedRaycaster(IVec2Property* viewportSizeProp)
        : RaycastingProcessor(viewportSizeProp, "modules/vis/advraycaster/glsl/AdvOptimizedRaycaster.frag", true, "400")
        , p_enableShading("EnableShading", "Enable Shading", true)
        , p_lightId("LightId", "Input Light Source", "lightsource", DataNameProperty::READ)
        , _vv(0)
        , _quad(0)
        , _voxelGeneratorShdr(0)
        , _vvTex(0)
        , _mipMapGen(0)
    {
        addDecorator(new ProcessorDecoratorGradient());

        addProperty(p_enableShading, INVALID_RESULT | INVALID_PROPERTIES | INVALID_SHADER);
        addProperty(p_lightId);

        setPropertyInvalidationLevel(p_transferFunction, INVALID_BBV | INVALID_RESULT);
        setPropertyInvalidationLevel(p_sourceImageID, INVALID_BBV | INVALID_RESULT);

        decoratePropertyCollection(this);
    }

    AdvOptimizedRaycaster::~AdvOptimizedRaycaster() {
    }

    void AdvOptimizedRaycaster::init() {
        RaycastingProcessor::init();

        _quad = GeometryDataFactory::createQuad(tgt::vec3(0.f), tgt::vec3(1.f), tgt::vec3(1.f, 1.f, 0.f), tgt::vec3(0.f, 0.f, 0.f));
        _voxelGeneratorShdr = ShdrMgr.loadWithCustomGlslVersion("core/glsl/passthrough.vert", "", "modules/vis/advraycaster/glsl/rendervolumevoxelizing.frag", "", "400");

        invalidate(INVALID_BBV);
    }

    void AdvOptimizedRaycaster::deinit() {
        delete _vv;
        delete _vvTex;
        RaycastingProcessor::deinit();
    }

    void AdvOptimizedRaycaster::processImpl(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image) {
        tgt::TextureUnit bbvUnit;

        if (getInvalidationLevel() & INVALID_BBV){

            renderVv(data);

            validate(INVALID_BBV);
        }
        
        ScopedTypedData<LightSourceData> light(data, p_lightId.getValue());

        if (p_enableShading.getValue() == false || light != nullptr) {
            _shader->activate();

            if (_vvTex != 0){
                // bind
                bbvUnit.activate();
                _vvTex->bind();
                _shader->setIgnoreUniformLocationError(true);
                _shader->setUniform("_vvTexture", bbvUnit.getUnitNumber());
                _shader->setUniform("_vvTextureParams._size", tgt::vec3(_vvTex->getDimensions()));
                _shader->setUniform("_vvTextureParams._sizeRCP", tgt::vec3(1.f) / tgt::vec3(_vvTex->getDimensions()));
                _shader->setUniform("_vvTextureParams._numChannels", static_cast<int>(1));

                _shader->setUniform("_vvVoxelSize", static_cast<int>(_vv->getBrickSize()));
                _shader->setUniform("_vvVoxelDepth", static_cast<int>(_vv->getBrickDepth()));
                _shader->setUniform("_hasVv", true);

                _shader->setUniform("_vvMaxMipMapLevel", _maxMipMapLevel);

                _shader->setIgnoreUniformLocationError(false);
            } else {
                _shader->setUniform("_hasVv", false);
            }

            if (p_enableShading.getValue() && light != nullptr) {
                light->bind(_shader, "_lightSource");
            }

            FramebufferActivationGuard fag(this);
            createAndAttachTexture(GL_RGBA8);
            createAndAttachTexture(GL_RGBA32F);
            createAndAttachTexture(GL_RGBA32F);
            createAndAttachDepthTexture();


            static const GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2 };
            glDrawBuffers(3, buffers);

            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            QuadRdr.renderQuad();

            glDisable(GL_DEPTH_TEST);
            LGL_ERROR;

            glDrawBuffers(1, buffers);

            data.addData(p_targetImageID.getValue(), new RenderData(_fbo)); 
        }
    }

    std::string AdvOptimizedRaycaster::generateHeader() const {
        std::string toReturn = RaycastingProcessor::generateHeader();
        if (p_enableShading.getValue())
            toReturn += "#define ENABLE_SHADING\n";
        return toReturn;
    }

    void AdvOptimizedRaycaster::updateProperties() {
        validate(AbstractProcessor::INVALID_PROPERTIES);
    }

    void AdvOptimizedRaycaster::renderVv(DataContainer& data) {
        delete _vv;
        _vv = 0;
        delete _vvTex;
        _vvTex = 0;
        delete _mipMapGen;
        _mipMapGen = 0;

        ImageRepresentationGL::ScopedRepresentation img(data, p_sourceImageID.getValue());

        if (img != 0){
            _vv = new VoxelizedRenderVolume(img->getParent(), 4);
            _vvTex = _vv->createEmptyImageData();
            _mipMapGen = new VoxelTexMipMapGenerator();
            
            LDEBUG("Start computing voxel visibilities...");
                
            glPushAttrib(GL_ALL_ATTRIB_BITS);

            tgt::FramebufferObject frameBuffer;

            LGL_ERROR;
            /// Activate the shader to generate the voxelized volume.
            _voxelGeneratorShdr->activate();

            tgt::TextureUnit volumeUnit, tfUnit;
            volumeUnit.activate();
            tfUnit.activate();

            LGL_ERROR;
            _voxelGeneratorShdr->setIgnoreUniformLocationError(true);
                img->bind(_voxelGeneratorShdr, volumeUnit, "_volume", "_volumeTextureParams");
                p_transferFunction.getTF()->bind(_voxelGeneratorShdr, tfUnit, "_transferFunction", "_transferFunctionParams");
                _voxelGeneratorShdr->setUniform("_voxelDepth", static_cast<unsigned int>(_vv->getBrickDepth()));
                _voxelGeneratorShdr->setUniform("_voxelSize", static_cast<unsigned int>(_vv->getBrickSize()));
            _voxelGeneratorShdr->setIgnoreUniformLocationError(false);
            LGL_ERROR;
            frameBuffer.activate();

            // Set OpenGL pixel alignment to 1 to avoid problems with NPOT textures
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            tgt::mat4 projection = tgt::mat4::createOrtho(0, 1, 0, 1, -1, 1);
            _voxelGeneratorShdr->setUniform("_projectionMatrix", projection);

            LGL_ERROR;
                    
            //write to:
            glViewport(0, 0, static_cast<GLsizei>(_vvTex->getWidth()), static_cast<GLsizei>(_vvTex->getHeight()));

            frameBuffer.attachTexture(_vvTex, GL_COLOR_ATTACHMENT0, 0, 0);
            frameBuffer.isComplete();

            LGL_ERROR;
            _quad->render(GL_POLYGON);
            LGL_ERROR;

            _shader->deactivate();    
            frameBuffer.deactivate();

            glPopAttrib();

            LDEBUG("...finished computing voxel visibilities.");
                    
            LGL_ERROR;
            LDEBUG("Start computing the levels of the voxel object.");
                    
            _mipMapGen->attachMipmapsTo(_vvTex, _vv->getWidth(), _vv->getHeight());
            _mipMapGen->renderMipmapsFor(_vvTex, _vv->getWidth(), _vv->getHeight(), _vv->getBrickSize());
            _maxMipMapLevel = _mipMapGen->computeMaxLevel(_vv->getWidth(), _vv->getHeight());
                   

            LDEBUG("...finished computing voxel visibilities mip maps.");

        }
    }
}
