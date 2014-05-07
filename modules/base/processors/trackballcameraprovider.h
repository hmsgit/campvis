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

#ifndef TRACKBALLCAMERAPROVIDER_H__
#define TRACKBALLCAMERAPROVIDER_H__

#include "tbb/atomic.h"
#include "tgt/bounds.h"
#include "tgt/event/eventlistener.h"
#include "tgt/navigation/trackball.h"
#include "modules/base/processors/cameraprovider.h"

namespace tgt {
    class Trackball;
}

namespace campvis {
    class VisualizationProcessor;

    /**
     * Generates CameraData objects.
     */
    class TrackballCameraProvider : public CameraProvider, public tgt::EventListener , public tgt::IHasCamera {
    public:
        /// Trackball automation mode
        enum AutomationMode {
            FullManual,     ///< All camera properties have to be set manually (as in CameraProvider)
            SemiAutomatic,  ///< User has to define scene bounds
            FullAutomatic   ///< Properties are automatially adapted to a provided image
        };

        /**
         * Constructs a new TrackballCameraProvider Processor
         **/
        TrackballCameraProvider(IVec2Property* canvasSize);

        /**
         * Destructor
         **/
        virtual ~TrackballCameraProvider();


        /// \see AbstractProcessor::updateProperties()
        virtual void updateProperties(DataContainer& dataContainer);

        /// \see tgt::EventListener::onEvent()
        virtual void onEvent(tgt::Event* e);

        /// \see tgt::Camera::IHasCamera::getCamera()
        virtual tgt::Camera* getCamera();

        /// \see tgt::Camera::IHasCamera::update()
        virtual void update();


        /**
        * Adds \a vp to the list of LQ mode processors.
        * During interaction, TrackballNavigationEventListener will set the LQ mode flag of all
        * LQ mode processors.
        * \param vp VisualizationProcessor to add to the list of LQ mode processors.
        */
        void addLqModeProcessor(VisualizationProcessor* vp);

        /**
        * Removes \a vp from the list of LQ mode processors.
        * \param vp VisualizationProcessor to remove from the list of LQ mode processors.
        */
        void removeLqModeProcessor(VisualizationProcessor* vp);


        GenericOptionProperty<AutomationMode> p_automationMode;

        DataNameProperty p_image;

        Vec3Property p_llf;
        Vec3Property p_urb;

    protected:
        void onRenderTargetSizeChanged(const AbstractProperty* prop);

        IVec2Property* _canvasSize;

        /// Temporary copy of the property's camera which will be modified and written back to the property upon update().
        tgt::Camera _localCopy;
        /// The tgt::Trackball for the navigation logic
        tgt::Trackball* _trackball;

        tbb::atomic<bool> _dirty;

        /// List of processors for which to enable LQ mode during interaction
        std::vector<VisualizationProcessor*> _lqModeProcessors;

        static const std::string loggerCat_;
    };

}

#endif // TRACKBALLCAMERAPROVIDER_H__