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
#include "core/datastructures/imagerepresentationrendertarget.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/tools/job.h"
#include "core/tools/opengljobprocessor.h"
#include "core/tools/simplejobprocessor.h"

namespace campvis {
    const std::string VisualizationPipeline::loggerCat_ = "CAMPVis.core.datastructures.VisualizationPipeline";

    VisualizationPipeline::VisualizationPipeline() 
        : AbstractPipeline()
        , tgt::EventListener()
        , _lqMode(false)
        , _effectiveRenderTargetSize("canvasSize", "Canvas Size", tgt::ivec2(128, 128))
        , _renderTargetID("renderTargetID", "Render Target ID", "VisualizationPipeline.renderTarget")
        , _canvas(0)
    {
        _data.s_dataAdded.connect(this, &VisualizationPipeline::onDataContainerDataAdded);
        addProperty(&_renderTargetID);
        _renderTargetID.s_changed.connect<VisualizationPipeline>(this, &VisualizationPipeline::onPropertyChanged);
    }

    VisualizationPipeline::~VisualizationPipeline() {
        _data.s_dataAdded.disconnect(this);
    }

    void VisualizationPipeline::onEvent(tgt::Event* e) {
        // cycle through event handlers, ask each one if it handles the event and if so, execute it.
        for (std::vector<AbstractEventHandler*>::iterator it = _eventHandlers.begin(); it != _eventHandlers.end() && e->isAccepted(); ++it) {
            if ((*it)->accept(e)) {
                (*it)->execute(e);
            }
        }

        if (e->isAccepted()) {
            EventListener::onEvent(e);
        }
    }

    void VisualizationPipeline::init() {
        AbstractPipeline::init();
    }

    void VisualizationPipeline::deinit() {
        AbstractPipeline::deinit();
    }

    const tgt::ivec2& VisualizationPipeline::getRenderTargetSize() const {
        return _renderTargetSize;
    }

    void VisualizationPipeline::setRenderTargetSize(const tgt::ivec2& size) {
        _renderTargetSize = size;
        updateEffectiveRenderTargetSize();
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
            makeJobOnHeap<VisualizationPipeline, AbstractProcessor*>(this, &VisualizationPipeline::executeProcessor, processor),
            OpenGLJobProcessor::SerialJob);
    }

    void VisualizationPipeline::setCanvas(tgt::GLCanvas* canvas) {
        _canvas = canvas;
    }

    void VisualizationPipeline::onPropertyChanged(const AbstractProperty* prop) {
        if (prop == &_renderTargetID)
            s_renderTargetChanged();
        else
            AbstractPipeline::onPropertyChanged(prop);
    }

    void VisualizationPipeline::addEventHandler(AbstractEventHandler* eventHandler) {
        tgtAssert(eventHandler != 0, "Event handler must not be 0.");
        _eventHandlers.push_back(eventHandler);
    }

    void VisualizationPipeline::enableLowQualityMode() {
        _lqMode = true;
        updateEffectiveRenderTargetSize();
    }

    void VisualizationPipeline::disableLowQualityMode() {
        _lqMode = false;
        updateEffectiveRenderTargetSize();
    }

    void VisualizationPipeline::updateEffectiveRenderTargetSize() {
        if (_lqMode)
            _effectiveRenderTargetSize.setValue(_renderTargetSize / 2);
        else
            _effectiveRenderTargetSize.setValue(_renderTargetSize);
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
                    makeJobOnHeap<VisualizationPipeline, AbstractProcessor*>(this, &VisualizationPipeline::executeProcessor, processor), 
                    OpenGLJobProcessor::SerialJob);
            }
            else {
                SimpleJobProc.enqueueJob(makeJob<VisualizationPipeline, AbstractProcessor*>(this, &VisualizationPipeline::executeProcessor, processor));
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

}
