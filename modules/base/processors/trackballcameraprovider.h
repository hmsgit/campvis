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
#include "cgt/bounds.h"
#include "cgt/event/eventlistener.h"
#include "cgt/navigation/trackball.h"

#include "modules/modulesapi.h"
#include "modules/base/processors/cameraprovider.h"

namespace cgt {
    class Trackball;
}

namespace campvis {
    class VisualizationProcessor;

    /**
     * Generates CameraData objects.
     */
    class CAMPVIS_MODULES_API TrackballCameraProvider : public CameraProvider, public cgt::EventListener, public cgt::AbstractCameraProxy {
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
        explicit TrackballCameraProvider(IVec2Property* canvasSize);

        /**
         * Destructor
         **/
        virtual ~TrackballCameraProvider();


        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "TrackballCameraProvider"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Provides trackball navigation interaction metaphors when generating CameraData objects."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::STABLE; };

        /// \see AbstractProcessor::init()
        virtual void init();
        /// \see AbstractProcessor::deinit()
        virtual void deinit();

        /// \see AbstractProcessor::updateProperties()
        virtual void updateProperties(DataContainer& dataContainer);

        /// \see cgt::EventListener::onEvent()
        virtual void onEvent(cgt::Event* e);

        /// \see cgt::Camera::AbstractCameraProxy::getCamera()
        virtual cgt::Camera* getCamera();

        /// \see cgt::Camera::AbstractCameraProxy::update()
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


        /**
         * Sets the property defining the viewport size to \a viewportSizeProp.
         * \note    This processor will keep and access this pointer, so make sure the referenced
         *          property exists at least as long as this processor or you set it to a different
         *          property before.
         * \param   viewportSizeProp    Pointer to the property defining the viewport size, must not be 0.
         */
        void setViewportSizeProperty(IVec2Property* viewportSizeProp);

        /**
         * Reinitializes the camera using the given world bounds.
         * If the scene bounds have changed, the camera setup is reinitialized positioning the 
         * camera in front of the data along the z-axis and looking at the center of the data.
         * 
         * \param   bounds  The world bounds to use for reinitializing the camera.
         */
        void reinitializeCamera(const cgt::Bounds& worldBounds);

        GenericOptionProperty<AutomationMode> p_automationMode;

        DataNameProperty p_image;   ///< Reference Image to use to determine bounding box (optional)

        Vec3Property p_llf;
        Vec3Property p_urb;

    protected:
        void onRenderTargetSizeChanged(const AbstractProperty* prop);

        IVec2Property* _canvasSize;

        /// Temporary copy of the property's camera which will be modified and written back to the property upon update().
        cgt::Camera _localCopy;
        /// The cgt::Trackball for the navigation logic
        cgt::Trackball* _trackball;

        tbb::atomic<bool> _dirty;

        /// List of processors for which to enable LQ mode during interaction
        std::vector<VisualizationProcessor*> _lqModeProcessors;

        static const std::string loggerCat_;
    };

}

#endif // TRACKBALLCAMERAPROVIDER_H__