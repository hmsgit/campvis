
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

#include "abstractpipeline.h"

#include "tgt/exception.h"
#include "tgt/glcanvas.h"
#include "tgt/glcontext.h"
#include "tgt/tgt_gl.h"

#include "core/pipeline/visualizationprocessor.h"
#include "core/pipeline/abstractprocessor.h"
#include "core/tools/job.h"
#include "core/tools/opengljobprocessor.h"
#include "core/tools/simplejobprocessor.h"

#include <ctime>

// Anonymous OpenGL helper functions
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
    const std::string AbstractPipeline::loggerCat_ = "CAMPVis.core.datastructures.AbstractPipeline";

    AbstractPipeline::AbstractPipeline(DataContainer* dc) 
        : HasPropertyCollection()
        , tgt::EventHandler()
        , tgt::EventListener()
        , _data(dc)
        , _canvas(0)
        , _canvasSize("CanvasSize", "Canvas Size", tgt::ivec2(128, 128), tgt::ivec2(1, 1), tgt::ivec2(4096, 4096))
        , _ignoreCanvasSizeUpdate(false)
        , _renderTargetID("renderTargetID", "Render Target ID", "AbstractPipeline.renderTarget", DataNameProperty::READ)
    {
        tgtAssert(_data != 0, "Pointer to the DataContainer for this pipeline must not be 0!");

        _enabled = false;

        addProperty(&_renderTargetID);
        addProperty(&_canvasSize);
    }

    AbstractPipeline::~AbstractPipeline() {
    }

    void AbstractPipeline::init() {
        _renderTargetID.s_changed.connect<AbstractPipeline>(this, &AbstractPipeline::onPropertyChanged);
        _data->s_dataAdded.connect(this, &AbstractPipeline::onDataContainerDataAdded);

        initAllProperties();

        // initialize all processors:
        for (std::vector<AbstractProcessor*>::iterator it = _processors.begin(); it != _processors.end(); ++it) {
            try {
                (*it)->init();
            }
            catch (tgt::Exception& e) {
                LERROR("Caught Exception during initialization of processor: " << e.what());
            }
        }
    }

    void AbstractPipeline::deinit() {
        deinitAllProperties();

        // deinitialize all processors:
        for (std::vector<AbstractProcessor*>::iterator it = _processors.begin(); it != _processors.end(); ++it) {
            try {
                (*it)->s_invalidated.disconnect(this);
                (*it)->deinit();
            }
            catch (tgt::Exception& e) {
                LERROR("Caught Exception during deinitialization of processor: " << e.what());
            }
        }

        _data->s_dataAdded.disconnect(this);
        _renderTargetID.s_changed.disconnect(this);

        // clear DataContainer
        _data->clear();
    }

    void AbstractPipeline::onPropertyChanged(const AbstractProperty* prop) {
        if (prop == &_renderTargetID) {
            s_renderTargetChanged();
        }
        else if (prop == &_canvasSize && _canvas != 0 && !_ignoreCanvasSizeUpdate) {
            if (_canvasSize.getValue() != _canvas->getSize()) {
                _ignoreCanvasSizeUpdate = true;
                _canvas->setSize(_canvasSize.getValue());
                _ignoreCanvasSizeUpdate = false;
            }
        }
        else {
            HasPropertyCollection::onPropertyChanged(prop);
        }
    }

    const DataContainer& AbstractPipeline::getDataContainer() const {
        return *_data;
    }

    DataContainer& AbstractPipeline::getDataContainer() {
        return *_data;
    }

    void AbstractPipeline::executeProcessor(AbstractProcessor* processor, bool unlockInExtraThred) {
        tgtAssert(processor != 0, "Processor must not be 0.");

        if (processor->getEnabled() && !processor->isLocked()) {
            // update properties if they're invalid
            if (processor->hasInvalidProperties()) {
                processor->updateProperties(*_data);
#if CAMPVIS_DEBUG
                if (processor->hasInvalidProperties())
                    LDEBUG("Processor " << processor->getName() << " still has INVALID_PROPERTIES level. Did you forget to validate the processor in updateProperties()?");
#endif
            }

            // execute processor if needed
            if (processor->hasInvalidResult()) {
                processor->lockProcessor();
                clock_t startTime = clock();

                try {
                    processor->process(*_data);
                }
                catch (std::exception& e) {
                    LERROR("Caught unhandled exception while executing processor " << processor->getName() << ": " << e.what());
                }
                catch (...) {
                    LERROR("Caught unhandled exception while executing processor " << processor->getName() << ": unknown exception");
                }

                if (processor->getClockExecutionTime()) {
                    clock_t endTime = clock();
                    LDEBUG("Executed processor " << processor->getName() << " duration: " << (endTime - startTime));
                }

                // Unlocking processors might be expensive, since a long chain of invalidations might be started
                // -> do this in another thread...
                if (unlockInExtraThred)
                    SimpleJobProc.enqueueJob(makeJob(processor, &AbstractProcessor::unlockProcessor));
                else
                    processor->unlockProcessor();
            }
        }
    }

    const std::vector<AbstractProcessor*>& AbstractPipeline::getProcessors() const {
        return _processors;
    }

    bool AbstractPipeline::getEnabled() const {
        return _enabled;
    }

    void AbstractPipeline::setEnabled(bool enabled) {
        _enabled = enabled;
    }

    void AbstractPipeline::onEvent(tgt::Event* e) {
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

    void AbstractPipeline::setCanvas(tgt::GLCanvas* canvas) {
        _canvas = canvas;
    }

    void AbstractPipeline::setRenderTargetSize(const tgt::ivec2& size) {
        if (_canvasSize.getValue() != size && !_ignoreCanvasSizeUpdate) {
            _canvasSize.setValue(size);
        }
    }

    const tgt::ivec2& AbstractPipeline::getRenderTargetSize() const {
        return _canvasSize.getValue();
    }

    const std::string& AbstractPipeline::getRenderTargetID() const {
        return _renderTargetID.getValue();
    }

    void AbstractPipeline::addProcessor(AbstractProcessor* processor) {
        tgtAssert(processor != 0, "Processor must not be 0.")
        _processors.push_back(processor);
    }

    void AbstractPipeline::lockAllProcessors() {
        for (std::vector<AbstractProcessor*>::iterator it = _processors.begin(); it != _processors.end(); ++it)
            (*it)->lockProcessor();
    }

    void AbstractPipeline::unlockAllProcessors() {
        for (std::vector<AbstractProcessor*>::iterator it = _processors.begin(); it != _processors.end(); ++it)
            (*it)->unlockProcessor();
    }

    void AbstractPipeline::lockGLContextAndExecuteProcessor(AbstractProcessor* processor) {
        tgtAssert(_canvas != 0, "Set a valid canvas before calling this method!");
        GLJobProc.enqueueJob(
            _canvas, 
            makeJobOnHeap<AbstractPipeline, AbstractProcessor*, bool>(this, &AbstractPipeline::executeProcessor, processor, true),
            OpenGLJobProcessor::SerialJob);
    }

    void AbstractPipeline::executeProcessorAndCheckOpenGLState(AbstractProcessor* processor) {
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

    void AbstractPipeline::onDataContainerDataAdded(const std::string& name, const DataHandle& dh) {
        if (name == _renderTargetID.getValue()) {
            s_renderTargetChanged();
        }
    }



}
