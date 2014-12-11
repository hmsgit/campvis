
// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
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

#include "abstractpipeline.h"

#include <tbb/tick_count.h>

#include "cgt/exception.h"
#include "cgt/glcanvas.h"
#include "cgt/glcontextmanager.h"
#include "cgt/painter.h"
#include "cgt/cgt_gl.h"

#include "core/pipeline/visualizationprocessor.h"
#include "core/pipeline/abstractprocessor.h"


namespace campvis {
    const std::string AbstractPipeline::loggerCat_ = "CAMPVis.core.datastructures.AbstractPipeline";

    AbstractPipeline::AbstractPipeline(DataContainer* dc) 
        : HasPropertyCollection()
        , cgt::EventHandler()
        , cgt::EventListener()
        , _data(dc)
        , _canvas(0)
        , _canvasSize("CanvasSize", "Canvas Size", cgt::ivec2(128, 128), cgt::ivec2(1, 1), cgt::ivec2(4096, 4096))
        , _ignoreCanvasSizeUpdate(false)
        , _renderTargetID("renderTargetID", "Render Target ID", "AbstractPipeline.renderTarget", DataNameProperty::READ)
    {
        cgtAssert(_data != 0, "Pointer to the DataContainer for this pipeline must not be 0!");

        _enabled = false;
        _pipelineDirty = true;

        addProperty(_renderTargetID);
        addProperty(_canvasSize);
    }

    AbstractPipeline::~AbstractPipeline() {

    }

    void AbstractPipeline::init() {
        initAllProperties();

        // initialize all processors:
        for (std::vector<AbstractProcessor*>::iterator it = _processors.begin(); it != _processors.end(); ++it) {
            try {
                (*it)->init();
            }
            catch (cgt::Exception& e) {
                LERROR("Caught Exception during initialization of processor: " << e.what());
            }
        }

        // use trigger signal to enforce blocking call
        s_init.triggerSignal();
    }

    void AbstractPipeline::deinit() {
        // use trigger signal to enforce blocking call
        s_deinit.triggerSignal();

        deinitAllProperties();

        // deinitialize all processors:
        for (std::vector<AbstractProcessor*>::iterator it = _processors.begin(); it != _processors.end(); ++it) {
            try {
                (*it)->s_invalidated.disconnect(this);
                (*it)->deinit();
            }
            catch (cgt::Exception& e) {
                LERROR("Caught Exception during deinitialization of processor: " << e.what());
            }
        }


        // clear DataContainer
        _data->clear();
    }

    void AbstractPipeline::run() {
        std::unique_lock<std::mutex> lock(*cgt::GlContextManager::getRef().getGlMutexForContext(_canvas));
        cgt::GlContextManager::getRef().acquireContext(_canvas, false);

        while (! _stopExecution) {
            if (_enabled && _pipelineDirty) {
                // mark pipeline as not dirty
                _pipelineDirty = false;

                // execute pipeline
                executePipeline();

                // paint on canvas
                _canvas->getPainter()->paint();
            }

            if (!_stopExecution && (!_enabled || !_pipelineDirty)) {
                cgt::GlContextManager::getRef().releaseContext(_canvas, false);
                _evaluationCondition.wait(lock);
                cgt::GlContextManager::getRef().acquireContext(_canvas, false);
            }
        }

        cgt::GlContextManager::getRef().releaseContext(_canvas, false);
    }

    void AbstractPipeline::onPropertyChanged(const AbstractProperty* prop) {
        if (prop == &_renderTargetID) {
            setPipelineDirty();
        }
        else if (prop == &_canvasSize && _canvas != nullptr && !_ignoreCanvasSizeUpdate) {
            if (_canvasSize.getValue() != _canvas->getSize()) {
                _ignoreCanvasSizeUpdate = true;
                _canvas->setSize(_canvasSize.getValue());
                _ignoreCanvasSizeUpdate = false;
            }
        }
    }

    const DataContainer& AbstractPipeline::getDataContainer() const {
        return *_data;
    }

    DataContainer& AbstractPipeline::getDataContainer() {
        return *_data;
    }

    void AbstractPipeline::executeProcessor(AbstractProcessor* processor) {
        cgtAssert(processor != 0, "Processor must not be 0.");

        // execute processor if needed
        if (processor->getEnabled() && !processor->isLocked()) {
            if (! processor->isValid()) {
                tbb::tick_count startTime;
                if (processor->getClockExecutionTime())
                    startTime = tbb::tick_count::now();

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
                    tbb::tick_count endTime = tbb::tick_count::now();
                    LINFO("Executed processor " << processor->getName() << " duration: " << (endTime - startTime).seconds());
                }
            }
        }
    }

    void AbstractPipeline::executeProcessorAndCheckOpenGLState(AbstractProcessor* processor) {
        AbstractPipeline::executeProcessor(processor);

#ifdef CAMPVIS_DEBUG
        cgtAssert(cgt::getGlBool(GL_DEPTH_TEST) == false, "Invalid OpenGL state after processor execution, GL_DEPTH_TEST != false.");
        cgtAssert(cgt::getGlBool(GL_SCISSOR_TEST) == false, "Invalid OpenGL state after processor execution, GL_SCISSOR_TEST != false.");

        cgtAssert(cgt::getGlInt(GL_CULL_FACE_MODE) == GL_BACK, "Invalid OpenGL state after processor execution, GL_CULL_FACE_MODE != GL_BACk.");
        cgtAssert(cgt::getGlInt(GL_DEPTH_FUNC) == GL_LESS, "Invalid OpenGL state after processor execution, GL_DEPTH_FUNC != GL_LESS.");

        cgtAssert(cgt::getGlFloat(GL_DEPTH_CLEAR_VALUE) == 1.f, "Invalid OpenGL state after processor execution, GL_DEPTH_CLEAR_VALUE != 1.f.");

        cgtAssert(cgt::getGlFloat(GL_RED_SCALE) == 1.f, "Invalid OpenGL state after processor execution, GL_RED_SCALE != 1.f.");
        cgtAssert(cgt::getGlFloat(GL_GREEN_SCALE) == 1.f, "Invalid OpenGL state after processor execution, GL_GREEN_SCALE != 1.f.");
        cgtAssert(cgt::getGlFloat(GL_BLUE_SCALE) == 1.f, "Invalid OpenGL state after processor execution, GL_BLUE_SCALE != 1.f.");
        cgtAssert(cgt::getGlFloat(GL_ALPHA_SCALE) == 1.f, "Invalid OpenGL state after processor execution, GL_ALPHA_SCALE != 1.f.");

        cgtAssert(cgt::getGlFloat(GL_RED_BIAS) == 0.f, "Invalid OpenGL state after processor execution, GL_RED_BIAS != 0.f.");
        cgtAssert(cgt::getGlFloat(GL_GREEN_BIAS) == 0.f, "Invalid OpenGL state after processor execution, GL_GREEN_BIAS != 0.f.");
        cgtAssert(cgt::getGlFloat(GL_BLUE_BIAS) == 0.f, "Invalid OpenGL state after processor execution, GL_BLUE_BIAS != 0.f.");
        cgtAssert(cgt::getGlFloat(GL_ALPHA_BIAS) == 0.f, "Invalid OpenGL state after processor execution, GL_ALPHA_BIAS != 0.f.");
#endif
    }

    const std::vector<AbstractProcessor*>& AbstractPipeline::getProcessors() const {
        return _processors;
    }

    bool AbstractPipeline::getEnabled() const {
        return _enabled;
    }

    void AbstractPipeline::setEnabled(bool enabled) {
        _enabled = enabled;
        setPipelineDirty();
    }

    void AbstractPipeline::onEvent(cgt::Event* e) {
        // copy and paste from cgt::EventHandler::onEvent() but without deleting e
        for (size_t i = 0 ; i < listeners_.size() ; ++i) {
            // don't forward this event to ourselves - otherwise we'll end up with an endless loop.
            if (listeners_[i] == this) {
                cgt::EventListener::onEvent(e);
            }
            // check if current listener listens to the eventType of e
            else if (listeners_[i]->getEventTypes() & e->getEventType() ){
                listeners_[i]->onEvent(e);
            }

            if (e->isAccepted())
                break;
        }
    }

    void AbstractPipeline::setCanvas(cgt::GLCanvas* canvas) {
        _canvas = canvas;
    }

    void AbstractPipeline::setRenderTargetSize(const cgt::ivec2& size) {
        if (_canvasSize.getValue() != size && !_ignoreCanvasSizeUpdate) {
            _canvasSize.setValue(size);
        }
    }

    const cgt::ivec2& AbstractPipeline::getRenderTargetSize() const {
        return _canvasSize.getValue();
    }

    const std::string& AbstractPipeline::getRenderTargetID() const {
        return _renderTargetID.getValue();
    }

    void AbstractPipeline::addProcessor(AbstractProcessor* processor) {
        cgtAssert(processor != 0, "Processor must not be 0.")
        _processors.push_back(processor);
    }

    void AbstractPipeline::setPipelineDirty() {
        _pipelineDirty = true;

        if (_enabled)
            _evaluationCondition.notify_all();
    }

    AbstractProcessor* AbstractPipeline::getProcessor(const std::string& name) const {
        for (auto it = _processors.begin(); it != _processors.end(); ++it) {
            if ((*it)->getName() == name)
                return *it;
        }

        return nullptr;
    }



}
