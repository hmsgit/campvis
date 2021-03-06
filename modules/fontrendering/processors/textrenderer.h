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

#ifndef TEXTRENDERER_H__
#define TEXTRENDERER_H__

#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/allproperties.h"

#include "modules/modulesapi.h"

namespace campvis {
namespace fontrendering {

    class FontAtlas;

    /**
     * Renders a text using OpenGL
     */
    class CAMPVIS_MODULES_API TextRenderer : public VisualizationProcessor {
    public:
        /**
         * Constructs a new TextRenderer Processor.
         **/
        explicit TextRenderer(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~TextRenderer();

        /// To be used in ProcessorFactory static methods
        static const std::string getId() { return "TextRenderer"; };
        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return getId(); };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Renders a text using OpenGL."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };


        virtual void init();
        virtual void deinit();

        StringProperty p_text;              ///< Text to render
        IVec2Property p_position;           ///< Position of text in viewport coordinates
        StringProperty p_fontFileName;      ///< Path to the font file to use
        IntProperty p_fontSize;             ///< Font size to use
        Vec4Property p_color;               ///< Color to use
        DataNameProperty p_outputImage;     ///< Name/ID for the output image with the rendered image

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);

        /// Initializes the FontAtlas used.
        void initializeFontAtlas();
        /// Callback when _viewportSizeProperty has changed
        void onViewportSizeChanged(const AbstractProperty* prop);

        FontAtlas* _atlas;

        static const std::string loggerCat_;
    };

}
}

#endif // TEXTRENDERER_H__
