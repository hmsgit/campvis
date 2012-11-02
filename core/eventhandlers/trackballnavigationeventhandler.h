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

#ifndef TRACKBALLNAVIGATIONEVENTHANDLER_H__
#define TRACKBALLNAVIGATIONEVENTHANDLER_H__

#include "tgt/bounds.h"
#include "tgt/logmanager.h"
#include "tgt/navigation/trackball.h"
#include "core/eventhandlers/abstracteventhandler.h"

namespace campvis {
    class CameraProperty;

    /**
     * Wrapper to adapt a CameraProperty to the tgt::Trackball interface.
     */
    class CamPropNavigationWrapper : public tgt::IHasCamera {
    public:
        CamPropNavigationWrapper(CameraProperty* camProp);
        virtual ~CamPropNavigationWrapper();

        virtual tgt::Camera* getCamera();

        virtual void update();

    private:
        CameraProperty* _cameraProperty;    ///< CameraProperty this class wraps around

        /// Temporary copy of the property's camera which will be modified and written back to the property upon update().
        tgt::Camera _localCopy;
        /// Flag whether _localCopy is dirty (needs to be written back)
        bool _dirty;
    };

    /**
     * EventHandler implementing a trackball navigation for a CameraProperty.
     * Implementation inspired by http://www.opengl.org/wiki/Trackball
     */
    class TrackballNavigationEventHandler : public AbstractEventHandler {
    public:
        /**
         * Creates a TrackballNavigationEventHandler.
         */
        TrackballNavigationEventHandler(CameraProperty* cameraProperty, const tgt::ivec2& viewportSize);

        /**
         * Virtual Destructor
         **/
        virtual ~TrackballNavigationEventHandler();


        /**
         * Checks, whether the given event \a e is handled by this EventHandler.
         * \param e     The event to check
         * \return      True, if the given event is handled by this EventHandler.
         */
        virtual bool accept(tgt::Event* e);

        /**
         * Performs the event handling.
         * \param e     The event to handle
         */
        virtual void execute(tgt::Event* e);

        /**
         * Sets the viewport size to \a viewportSize.
         * \param viewportSize  The new viewport size.
         */
        void setViewportSize(const tgt::ivec2& viewportSize);

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

    protected:
        CameraProperty* _cameraProperty;
        CamPropNavigationWrapper _cpnw;
        tgt::Trackball* _trackball;
        tgt::Bounds _sceneBounds;

        static const std::string loggerCat_;
    };

}

#endif // TRACKBALLNAVIGATIONEVENTHANDLER_H__
