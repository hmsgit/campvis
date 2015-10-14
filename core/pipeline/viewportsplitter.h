// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#ifndef VIEWPORTSPLITTER_H__
#define VIEWPORTSPLITTER_H__

#include "cgt/vector.h"
#include "cgt/event/eventlistener.h"

#include "core/properties/datanameproperty.h"
#include "core/properties/numericproperty.h"

#include <memory>

namespace cgt {
    class FramebufferObject;
    class Shader;
}

namespace campvis {
    class FaceGeometry;
    class ImageData;

    /**
     * Helper class to facilitate splitting the viewport into multiple views.
     * 
     * Splitting can be done either vertically or horizontally. ViewportSplitter takes care of
     * computing the viewport sizes of the sub views (you can access them via properties for easy
     * forwarding to processors), rendering each sub view, as well as handling events, computing
     * the corresponding view and forwarding them to potential listeners.
     */
    class CAMPVIS_CORE_API ViewportSplitter : public cgt::EventListener, public sigslot::has_slots {
    public:
        /// Enumeration on how to split the view
        enum SplitMode {
            HORIZONTAL,     ///< Split the view horizontally
            VERTICAL        ///< Split the view vertically
        };

        /**
         * Creates a ViewportSplitter.
         * 
         * \note    This processor will keep and access \a viewportSizeProp, so make sure the referenced
         *          property exists at least as long as this processor or you set it to a different
         *          property before using setViewportSizeProperty().
         *          
         * \param   numSubViews         Number of sub views to create
         * \param   splitMode           Mode how to split the view
         * \param   viewportSizeProp    Pointer to the property defining the viewport size, must not be 0.
         */
        explicit ViewportSplitter(size_t numSubViews, SplitMode splitMode, IVec2Property* viewportSizeProp);

        /**
         * Virtual Destructor
         **/
        virtual ~ViewportSplitter();

        /**
         * Initialize OpenGL-related stuff.
         */
        virtual void init();

        /**
         * Deinitialize OpenGL-related stuff.
         */
        virtual void deinit();


        /**
         * Sets the property to lookup the input image ID for the given view index.
         * \param   index   Index of the subview for which to define the property.
         * \param   prop    DataNameProperty in which ViewportSplitter can lookup the image ID.
         */
        void setInputImageIdProperty(size_t index, DataNameProperty* prop);

        
        /**
         * Computes the view under the mouse position and forwards them to potential listeners using
         * ths s_onEvent signal.
         * Overloads cgt::onEvent()
         * \param   e   The event.
         */
        virtual void onEvent(cgt::Event* e);

        /**
         * Renders the each sub view into the final view and stores the result in the DataContainer.
         * \param   dataContainer   DataContainer to work on.
         */
        void render(DataContainer& dataContainer);

        /// Event emitted from ViewportSplitter::onEvent, but with adjusted viewport parameters.
        /// The first signal parameter gives the index of the view of the event.
        sigslot::signal2<size_t, cgt::Event*> s_onEvent;

        DataNameProperty p_outputImageId;       ///< Image ID for rendered image.
        IVec2Property p_subViewViewportSize;    ///< Viewport size of each sub view.

    protected:
        /// Callback for s_changed signal of p_viewportSizeProperty
        void onViewportSizePropertyChanged(const AbstractProperty* prop);

        std::vector<DataNameProperty*> p_inputImageIds;     ///< Vector of the properties to lookup input image IDs

        size_t _numSubViews;                        ///< Number of sub views to create
        SplitMode _splitMode;                       ///< Mode how to split the viewport into sub views

        bool _mousePressed;                         ///< Flag whether the mous was pressed in this view (to correctly compute the view of the event)
        size_t _viewIndexOfEvent;                   ///< Index of the sub view for the forwarded event.

        std::unique_ptr<FaceGeometry> _quad;        ///< Geometry used for rendering
        cgt::FramebufferObject* _fbo;               ///< The FBO used for rendering
        IVec2Property* p_viewportSizeProperty;      ///< Pointer to the property defining the viewport (canvas) size.
        cgt::Shader* _copyShader;                   ///< Shader used for rendering

        static const std::string loggerCat_;
    };

}

#endif // VIEWPORTSPLITTER_H__
