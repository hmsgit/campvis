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

#ifndef TRACKBALLNAVIGATIONEVENTHANDLER_H__
#define TRACKBALLNAVIGATIONEVENTHANDLER_H__

#include "tgt/logmanager.h"
#include "tgt/navigation/trackball.h"
#include "core/eventhandlers/abstracteventhandler.h"

namespace TUMVis {
    class CameraProperty;

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

        void reinitializeCamera(const tgt::vec3& position, const tgt::vec3& focus, const tgt::vec3& upVector);

        void setCenter(const tgt::vec3& center);

    protected:
        CameraProperty* _cameraProperty;
        CamPropNavigationWrapper _cpnw;
        tgt::Trackball* _trackball;

        static const std::string loggerCat_;
    };

}

#endif // TRACKBALLNAVIGATIONEVENTHANDLER_H__
