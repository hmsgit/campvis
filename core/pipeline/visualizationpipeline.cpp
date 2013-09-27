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

#include "visualizationpipeline.h"
#include "tgt/tgt_gl.h"
#include "tgt/glcanvas.h"
#include "tgt/glcontext.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/tools/job.h"
#include "core/tools/opengljobprocessor.h"
#include "core/tools/simplejobprocessor.h"

namespace {
    GLboolean getGlBool(GLenum param) {
        GLboolean toReturn;
        glGetBooleanv(param, &toReturn);
        return toReturn;
    };

    GLint getGlInt(GLenum param) {
        GLint toReturn;
        glGetIntegerv(param, &toReturn);
        return toReturn;
    };

    GLfloat getGlFloat(GLenum param) {
        GLfloat toReturn;
        glGetFloatv(param, &toReturn);
        return toReturn;
    }
}

namespace campvis {
    const std::string VisualizationPipeline::loggerCat_ = "CAMPVis.core.datastructures.VisualizationPipeline";

    VisualizationPipeline::VisualizationPipeline() 
        : AbstractPipeline()
        , tgt::EventHandler()
        , tgt::EventListener()
        , _ignoreCanvasSizeUpdate(false)
        , _canvasSize("CanvasSize", "Canvas Size", tgt::ivec2(128, 128), tgt::ivec2(1, 1), tgt::ivec2(4096, 4096))
        , _renderTargetID("renderTargetID", "Render Target ID", "VisualizationPipeline.renderTarget", DataNameProperty::READ)
        , _canvas(0)
    {
        _data.s_dataAdded.connect(this, &VisualizationPipeline::onDataContainerDataAdded);
        addProperty(&_renderTargetID);
        addProperty(&_canvasSize);
        _renderTargetID.s_changed.connect<VisualizationPipeline>(this, &VisualizationPipeline::onPropertyChanged);
    }

    VisualizationPipeline::~VisualizationPipeline() {
    }

    void VisualizationPipeline::onEvent(tgt::Event* e) {
        // copy and paste from tgt::EventHandler::onEvent() but without deleting e
        for (size_t i = 0 ; i < listeners_.size() ; ++i) {
            // check if current listener listens to the eventType of e
            if(listeners_[i]->getEventTypes() & e->getEventType() ){
                listeners_[i]->onEvent(e);
                if (e->isAccepted())
                    break;
            }
        }
    }

    void VisualizationPipeline::init() {
        AbstractPipeline::init();
    }

    void VisualizationPipeline::deinit() {
        _data.s_dataAdded.disconnect(this);
        _renderTargetID.s_changed.disconnect(this);
	
        AbstractPipeline::deinit();
    }

    const tgt::ivec2& VisualizationPipeline::getRenderTargetSize() const {
        return _canvasSize.getValue();
    }

    void VisualizationPipeline::setRenderTargetSize(const tgt::ivec2& size) {
        if (_canvasSize.getValue() != size && !_ignoreCanvasSizeUpdate) {
            _canvasSize.setValue(size);
        }
    }

    void VisualizationPipeline::onDataContainerDataAdded(const std::string& name, const DataHandle& dh) {
        if (name == _renderTargetID.getValue()) {
            s_renderTargetChanged();
        }
    }

    const std::string& VisualizationPipeline::getRenderTargetID() const {
        return _renderTargetID.getValue();
    }

    void VisualizationPipeline::lockGLContextAndExecuteProcessor(AbstractProcessor* processor) {
        tgtAssert(_canvas != 0, "Set a valid canvas before calling this method!");
        GLJobProc.enqueueJob(
            _canvas, 
            makeJobOnHeap<VisualizationPipeline, AbstractProcessor*, bool>(this, &VisualizationPipeline::executeProcessor, processor, true),
            OpenGLJobProcessor::SerialJob);
    }

    void VisualizationPipeline::setCanvas(tgt::GLCanvas* canvas) {
        _canvas = canvas;
    }

    void VisualizationPipeline::onPropertyChanged(const AbstractProperty* prop) {
        if (prop == &_renderTargetID)
            s_renderTargetChanged();
        else if (prop == &_canvasSize && _canvas != 0 && !_ignoreCanvasSizeUpdate) {
            if (_canvasSize.getValue() != _canvas->getSize()) {
                _ignoreCanvasSizeUpdate = true;
                _canvas->setSize(_canvasSize.getValue());
                _ignoreCanvasSizeUpdate = false;
            }
        }
        else
            AbstractPipeline::onPropertyChanged(prop);
    }

    void VisualizationPipeline::onProcessorInvalidated(AbstractProcessor* processor) {
        if (_canvas == 0)
            return;

        tbb::concurrent_hash_map<AbstractProcessor*, bool>::const_accessor a;
        if (_isVisProcessorMap.find(a, processor)) {
            if (a->second) {
                // is VisualizationProcessor
                GLJobProc.enqueueJob(
                    _canvas, 
                    makeJobOnHeap<VisualizationPipeline, AbstractProcessor*>(this, &VisualizationPipeline::executeProcessorAndCheckOpenGLState, processor), 
                    OpenGLJobProcessor::SerialJob);
            }
            else {
                SimpleJobProc.enqueueJob(makeJob<VisualizationPipeline, AbstractProcessor*, bool>(this, &VisualizationPipeline::executeProcessor, processor, false));
            }
        }
        else {
            tgtAssert(false, "Could not find processor in processor map.");
            LWARNING("Caught invalidation of a non-registered processor!");
        }
    }

    void VisualizationPipeline::addProcessor(AbstractProcessor* processor) {
        _isVisProcessorMap.insert(std::make_pair(processor, (dynamic_cast<VisualizationProcessor*>(processor) != 0)));
        AbstractPipeline::addProcessor(processor);
    }
    
    void VisualizationPipeline::executeProcessorAndCheckOpenGLState(AbstractProcessor* processor) {
        AbstractPipeline::executeProcessor(processor, true);

#ifdef CAMPVIS_DEBUG
        tgtAssert(getGlBool(GL_DEPTH_TEST) == false, "Invalid OpenGL state after processor execution, GL_DEPTH_TEST != false.");
        tgtAssert(getGlBool(GL_SCISSOR_TEST) == false, "Invalid OpenGL state after processor execution, GL_SCISSOR_TEST != false.");

        tgtAssert(getGlInt(GL_CULL_FACE_MODE) == GL_BACK, "Invalid OpenGL state after processor execution, GL_CULL_FACE_MODE != GL_BACk.");
        tgtAssert(getGlInt(GL_DEPTH_FUNC) == GL_LESS, "Invalid OpenGL state after processor execution, GL_DEPTH_FUNC != GL_LESS.");

        tgtAssert(getGlFloat(GL_DEPTH_CLEAR_VALUE) == 1.f, "Invalid OpenGL state after processor execution, GL_DEPTH_CLEAR_VALUE != 1.f.");

        tgtAssert(getGlFloat(GL_RED_SCALE) == 1.f, "Invalid OpenGL state after processor execution, GL_RED_SCALE != 1.f.");
        tgtAssert(getGlFloat(GL_GREEN_SCALE) == 1.f, "Invalid OpenGL state after processor execution, GL_GREEN_SCALE != 1.f.");
        tgtAssert(getGlFloat(GL_BLUE_SCALE) == 1.f, "Invalid OpenGL state after processor execution, GL_BLUE_SCALE != 1.f.");
        tgtAssert(getGlFloat(GL_ALPHA_SCALE) == 1.f, "Invalid OpenGL state after processor execution, GL_ALPHA_SCALE != 1.f.");

        tgtAssert(getGlFloat(GL_RED_BIAS) == 0.f, "Invalid OpenGL state after processor execution, GL_RED_BIAS != 0.f.");
        tgtAssert(getGlFloat(GL_GREEN_BIAS) == 0.f, "Invalid OpenGL state after processor execution, GL_GREEN_BIAS != 0.f.");
        tgtAssert(getGlFloat(GL_BLUE_BIAS) == 0.f, "Invalid OpenGL state after processor execution, GL_BLUE_BIAS != 0.f.");
        tgtAssert(getGlFloat(GL_ALPHA_BIAS) == 0.f, "Invalid OpenGL state after processor execution, GL_ALPHA_BIAS != 0.f.");
#endif
    }

}
