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

#ifndef TRACKBALLNAVIGATIONEVENTHANDLER_H__
#define TRACKBALLNAVIGATIONEVENTHANDLER_H__

#include <sigslot/sigslot.h>

#include "tgt/bounds.h"
#include "tgt/logmanager.h"
#include "tgt/event/eventlistener.h"
#include "tgt/navigation/trackball.h"

#include "core/coreapi.h"
#include "core/datastructures/datahandle.h"
#include "core/properties/numericproperty.h"

#include <vector>

namespace campvis {
    class CameraProperty;
    class IHasWorldBounds;
    class VisualizationProcessor;

    /**
     * Wrapper to adapt a CameraProperty to the tgt::Trackball interface.
     */
    class CAMPVIS_CORE_API CamPropNavigationWrapper : public tgt::IHasCamera {
    public:
        /**
         * Constructor
         * \param   camProp     The CameraProperty to wrap around.
         */
        explicit CamPropNavigationWrapper(CameraProperty* camProp);

        /// Virtual Destructor
        virtual ~CamPropNavigationWrapper();

        /// \see tgt::IHasCamera::getCamera()
        virtual tgt::Camera* getCamera();

        /// \see tgt::IHasCamera::update()
        virtual void update();

    private:
        CameraProperty* _cameraProperty;    ///< CameraProperty this class wraps around

        /// Temporary copy of the property's camera which will be modified and written back to the property upon update().
        tgt::Camera _localCopy;
        /// Flag whether _localCopy is dirty (needs to be written back)
        bool _dirty;
    };

    /**
     * EventListener implementing a trackball navigation for a CameraProperty.
     * Implementation inspired by http://www.opengl.org/wiki/Trackball
     * 
     * \note    Also takes care of automatically adjusting the window ratio for the wrapped
     *          camera when the viewport size changes.
     */
    class CAMPVIS_CORE_API TrackballNavigationEventListener : public tgt::EventListener, public sigslot::has_slots {
    public:
        /**
         * Creates a TrackballNavigationEventListener.
         * \note    TrackballNavigationEventListener keeps and accesses \a viewportSizeProp during the whole
         *          lifetime. Hence make sure the pointer is valid at all times.
         * \param   cameraProperty      Pointer to the CameraProperty to apply the navigation to, must not be 0.
         * \param   viewportSizeProp    Pointer to the property defining the viewport size, must not be 0.
         */
        TrackballNavigationEventListener(CameraProperty* cameraProperty, IVec2Property* viewportSizeProp);

        /**
         * Virtual Destructor
         **/
        virtual ~TrackballNavigationEventListener();


        /// \see tgt::EventListener::onEvent()
        virtual void onEvent(tgt::Event* e);
        

        /**
         * Sets the property defining the viewport size.
         * \param   viewportSizeProp    The new property defining the viewport size, must not be 0.
         */
        void setViewportSizeProperty(IVec2Property* viewportSizeProp);

        /**
         * Reinitializes the camera using the data in \a hwb.
         * If the scene bounds have changed, the camera setup is reinitialized positioning the 
         * camera in front of the data along the z-axis and looking at the center of the data.
         * 
         * \param   hwb     Data to use for reinitialization, must not be 0.
         */
        void reinitializeCamera(const IHasWorldBounds* hwb);

        /**
         * Reinitializes the camera using the data in \a hwb.
         * If the scene bounds have changed, the camera setup is reinitialized positioning the 
         * camera in front of the data along the z-axis and looking at the center of the data.
         * 
         * \param   hwb     Data to use for reinitialization, must not be 0.
         */
        void reinitializeCamera(const tgt::Bounds& worldBounds);

        /**
         * Reinitalizes the camera by the given parameters.
         * \param   position    New camera position
         * \param   focus       New camera focus
         * \param   upVector    New camera up vector
         */
        void reinitializeCamera(const tgt::vec3& position, const tgt::vec3& focus, const tgt::vec3& upVector);

        /**
         * Sets the trackball's center.
         * \param   center  The new trackball's center.
         */
        void setCenter(const tgt::vec3& center);

        /**
         * Sets the scene bounds for automatic near/far clipping plane adjustment.
         * \param   bounds  New bounds for the rendered scene.
         */
        void setSceneBounds(const tgt::Bounds& bounds);

        /**
         * Returns the current scene bounds for this trackball.
         * \return  _sceneBounds
         */
        const tgt::Bounds& getSceneBounds() const;

        /**
         * Returns the trackball of the event handler.
         */
        tgt::Trackball* getTrackball();

        /**
         * Adds \a vp to the list of LQ mode processors.
         * During interaction, TrackballNavigationEventListener will set the LQ mode flag of all
         * LQ mode processors.
         * \param   vp  VisualizationProcessor to add to the list of LQ mode processors.
         */
        void addLqModeProcessor(VisualizationProcessor* vp);

        /**
         * Removes \a vp from the list of LQ mode processors.
         * \param   vp  VisualizationProcessor to remove from the list of LQ mode processors.
         */
        void removeLqModeProcessor(VisualizationProcessor* vp);

    protected:


        /// Slot called when _viewportSizeProp changes 
        void onViewportSizePropChanged(const AbstractProperty* p);


        CameraProperty* _cameraProperty;        ///< The CameraProperty to apply the navigation to
        IVec2Property* _viewportSizeProp;       ///< Pointer to the property defining the viewport size
        CamPropNavigationWrapper _cpnw;         ///< The CamPropNavigationWrapper used to adapt to the tgt::Trackball interface

        tgt::Trackball* _trackball;             ///< The tgt::Trackball for the navigation logic
        tgt::Bounds _sceneBounds;               ///< The extent of the scene (in world coordinates)

        /// List of processors for which to enable LQ mode during interaction
        std::vector<VisualizationProcessor*> _lqModeProcessors;

        static const std::string loggerCat_;
    };

}

#endif // TRACKBALLNAVIGATIONEVENTHANDLER_H__
