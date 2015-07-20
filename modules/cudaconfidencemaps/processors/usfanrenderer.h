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

#ifndef USFANRENDERER_H__
#define USFANRENDERER_H__

#include <string>
#include <memory>

#include "core/pipeline/visualizationprocessor.h"
#include "core/pipeline/abstractprocessordecorator.h"
#include "core/properties/allproperties.h"
#include "core/datastructures/multiindexedgeometry.h"

#include "modules/fontrendering/tools/fontatlas.h"

namespace cgt {
    class Shader;
}

namespace campvis {

    class UsFanRenderer : public VisualizationProcessor {
    public:
        /**
         * Constructs a new UsFanRenderer Processor
         **/
        explicit UsFanRenderer(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~UsFanRenderer();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// To be used in ProcessorFactory static methods
        static const std::string getId() { return "UsFanRenderer"; };
        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return getId(); };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Enables to compute scan conversions of Ultrasound Images."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Denis Declara <denis.declara@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_inputImage;
        DataNameProperty p_renderTargetID;

        FloatProperty p_halfAngle;          ///< Half of the fan's opening angle in degrees
        FloatProperty p_innerRadius;

        StringProperty p_text;              ///< Text to render
        StringProperty p_fontFileName;      ///< Path to the font file to use
        IntProperty p_fontSize;             ///< Font size to use

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        void updateFontAtlas();

        cgt::Shader* _shader;
        std::unique_ptr<MultiIndexedGeometry> _grid;

        std::unique_ptr<fontrendering::FontAtlas> _atlas;

    private:
        static const std::string loggerCat_;
    };

}

#endif // USFANRENDERER_H__
