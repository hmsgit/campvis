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

#include "tgt/assert.h"
#include "tgt/event/mouseevent.h"

#include "trackballcameraprovider.h"
#include "core/datastructures/cameradata.h"
#include "core/datastructures/imagedata.h"
#include "core/pipeline/visualizationprocessor.h"

namespace campvis {


    // = TrackballNavigationEventHandler ==============================================================
    
    static const GenericOption<TrackballCameraProvider::AutomationMode> automationOptions[3] = {
        GenericOption<TrackballCameraProvider::AutomationMode>("manual", "Fully Manual", TrackballCameraProvider::FullManual),
        GenericOption<TrackballCameraProvider::AutomationMode>("semiAutomatic", "SemiAutomatic", TrackballCameraProvider::SemiAutomatic),
        GenericOption<TrackballCameraProvider::AutomationMode>("automatic", "Fully Automatic", TrackballCameraProvider::FullAutomatic)
    };

    const std::string TrackballCameraProvider::loggerCat_ = "CAMPVis.modules.io.TrackballCameraProvider";

    TrackballCameraProvider::TrackballCameraProvider(IVec2Property* canvasSize) 
        : CameraProvider()
        , p_automationMode("AutomationMode", "Automation Mode", automationOptions, 3)
        , p_image("ReferenceImage", "Reference Image", "", DataNameProperty::READ)
        , p_llf("LLF", "Bounding Box LLF", tgt::vec3(0.f), tgt::vec3(-10000.f), tgt::vec3(10000.f))
        , p_urb("URB", "Bounding Box URB", tgt::vec3(0.f), tgt::vec3(-10000.f), tgt::vec3(10000.f))
        , _canvasSize(canvasSize)
        , _trackball(0)
    {
        _dirty = false;

        addProperty(p_automationMode);
        addProperty(p_image, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_llf);
        addProperty(p_urb);
        
        _trackball = new tgt::Trackball(this, _canvasSize->getValue());
        _canvasSize->s_changed.connect(this, &TrackballCameraProvider::onRenderTargetSizeChanged);
    }

    TrackballCameraProvider::~TrackballCameraProvider() {
        _canvasSize->s_changed.disconnect(this);
        delete _trackball;
    }

    void TrackballCameraProvider::onEvent(tgt::Event* e) {
        if (typeid(*e) == typeid(tgt::MouseEvent)) {
            tgt::MouseEvent* me = static_cast<tgt::MouseEvent*>(e);
            if (me->action() == tgt::MouseEvent::PRESSED) {
                for (std::vector<VisualizationProcessor*>::iterator it = _lqModeProcessors.begin(); it != _lqModeProcessors.end(); ++it)
                    (*it)->p_lqMode.setValue(true);
                _trackball->mousePressEvent(me);
            }
            else if (me->action() == tgt::MouseEvent::RELEASED) {
                for (std::vector<VisualizationProcessor*>::iterator it = _lqModeProcessors.begin(); it != _lqModeProcessors.end(); ++it)
                    (*it)->p_lqMode.setValue(false);
                _trackball->mouseReleaseEvent(me);
            }
            else if (me->action() == tgt::MouseEvent::MOTION) {
                _trackball->mouseMoveEvent(me);
            }
            else if (me->action() == tgt::MouseEvent::WHEEL)
                _trackball->wheelEvent(me);
        }
        else if (typeid(*e) == typeid(tgt::KeyEvent)) {
            _trackball->keyEvent(static_cast<tgt::KeyEvent*>(e));
        }
    }

    tgt::Camera* TrackballCameraProvider::getCamera() {
        if (!_dirty) {
            _localCopy = tgt::Camera(
                p_position.getValue(),
                p_focus.getValue(),
                p_upVector.getValue(),
                p_fov.getValue(),
                p_aspectRatio.getValue(),
                p_clippingPlanes.getValue().x,
                p_clippingPlanes.getValue().y,
                p_projectionMode.getOptionValue());
        }
        _dirty = true;
        return &_localCopy;
    }

    void TrackballCameraProvider::update() {
        ignorePropertyChanges();

        p_position.setValue(_localCopy.getPosition());
        p_focus.setValue(_localCopy.getFocus());
        p_upVector.setValue(_localCopy.getUpVector());
        p_fov.setValue(_localCopy.getFovy());
        p_aspectRatio.setValue(_localCopy.getRatio());
        p_clippingPlanes.setValue(tgt::vec2(_localCopy.getNearDist(), _localCopy.getFarDist()));
        
        switch (_localCopy.getProjectionMode()) {
            case tgt::Camera::PERSPECTIVE:
                p_projectionMode.selectById("perspective");
                break;
            case tgt::Camera::ORTHOGRAPHIC:
                p_projectionMode.selectById("orthographic");
                break;
            case tgt::Camera::FRUSTUM:
                p_projectionMode.selectById("frustum");
                break;
        }

        _dirty = false;

        observePropertyChanges();
        invalidate(AbstractProcessor::INVALID_RESULT);
    }

    void TrackballCameraProvider::onRenderTargetSizeChanged(const AbstractProperty* prop) {
        _trackball->setViewprtSize(_canvasSize->getValue());
        float ratio = static_cast<float>(_canvasSize->getValue().x) / static_cast<float>(_canvasSize->getValue().y);
        p_aspectRatio.setValue(ratio);
    }

    void TrackballCameraProvider::updateProperties(DataContainer& data) {
        // convert data
        ScopedTypedData<ImageData> img(data, p_image.getValue());
        if (img != 0) {
            tgt::Bounds volumeExtent = img->getWorldBounds();
            tgt::vec3 pos = volumeExtent.center() - tgt::vec3(0, 0, tgt::length(volumeExtent.diagonal()));

            if (_trackball->getSceneBounds() != volumeExtent) {
                _trackball->setSceneBounds(volumeExtent);
                _trackball->setCenter(volumeExtent.center());
                _trackball->reinitializeCamera(pos, volumeExtent.center(), p_upVector.getValue());
            }
        }
        validate(INVALID_PROPERTIES);
    }

    void TrackballCameraProvider::addLqModeProcessor(VisualizationProcessor* vp) {
        tgtAssert(vp != 0, "Pointer to processor must not be 0.");
        tgtAssert(std::find(_lqModeProcessors.begin(), _lqModeProcessors.end(), vp) == _lqModeProcessors.end(), "Processor already in list of LQ mode processors.");

        _lqModeProcessors.push_back(vp);
    }

    void TrackballCameraProvider::removeLqModeProcessor(VisualizationProcessor* vp) {
        for (std::vector<VisualizationProcessor*>::iterator it = _lqModeProcessors.begin(); it != _lqModeProcessors.end(); ++it) {
            if (*it == vp) {
                _lqModeProcessors.erase(it);
                return;
            }
        }
    }

}