// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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
#include "core/datastructures/imagedatarendertarget.h"
#include "core/tools/job.h"
#include "core/tools/opengljobprocessor.h"

namespace TUMVis {
    const std::string VisualizationPipeline::loggerCat_ = "TUMVis.core.datastructures.VisualizationPipeline";

    VisualizationPipeline::VisualizationPipeline() 
        : AbstractPipeline()
        , tgt::EventListener()
        , _renderTargetSize("canvasSize", "Canvas Size", tgt::ivec2(128, 128))
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

    PropertyCollection& VisualizationPipeline::getPropertyCollection() {
        return _properties;
    }

    void VisualizationPipeline::init() {
        AbstractPipeline::init();
    }

    void VisualizationPipeline::deinit() {
        AbstractPipeline::deinit();
    }

    const tgt::ivec2& VisualizationPipeline::getRenderTargetSize() const {
        return _renderTargetSize.getValue();
    }

    void VisualizationPipeline::onDataContainerDataAdded(const std::string& name, const DataHandle& dh) {
        if (name == _renderTargetID.getValue()) {
            s_renderTargetChanged();
        }
    }

    const std::string& VisualizationPipeline::getRenderTargetID() const {
        return _renderTargetID.getValue();
    }

    void VisualizationPipeline::setRenderTargetSize(const tgt::ivec2& size) {
        _renderTargetSize.setValue(size);
    }

    void VisualizationPipeline::lockGLContextAndExecuteProcessor(AbstractProcessor* processor) {
        tgtAssert(_canvas != 0, "Set a valid canvas before calling this method!");
        GLJobProc.enqueueJob(
            _canvas, 
            new CallMemberFunc1ArgJob<VisualizationPipeline, AbstractProcessor*>(this, &VisualizationPipeline::executeProcessor, processor),
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

}
