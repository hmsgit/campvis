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

#ifndef ADVANCEDUSVIS_H__
#define ADVANCEDUSVIS_H__

#include "core/pipeline/autoevaluationpipeline.h"

#include "modules/modulesapi.h"

#include "modules/io/processors/genericimagereader.h"
#include "modules/advancedusvis/processors/advancedusfusion.h"
#include "modules/preprocessing/processors/glgaussianfilter.h"
#include "modules/vis/processors/quadview.h"

namespace campvis {
    class CAMPVIS_MODULES_API AdvancedUsVis : public AutoEvaluationPipeline {
    public:
        /**
         * Creates a AdvancedUsVis pipeline.
         * \param   dataContainer   Reference to the DataContainer containing local working set of data
         *                          for this pipeline, must be valid the whole lifetime of this pipeline.
         */
        explicit AdvancedUsVis(DataContainer& dataContainer);

        /**
         * Virtual Destructor
         **/
        virtual ~AdvancedUsVis();

        /// \see VisualizationPipeline::init()
        virtual void init();
        /// \see VisualizationPipeline::deinit()
        virtual void deinit();

        static const std::string getId() { return "AdvancedUsVis"; };

        virtual void keyEvent(cgt::KeyEvent* e);

    protected:
        GenericImageReader _usReader;
        GenericImageReader _confidenceReader;

        AdvancedUsFusion _usFusion1;
        AdvancedUsFusion _usFusion2;
        AdvancedUsFusion _usFusion3;
        AdvancedUsFusion _usFusion4;
        GlGaussianFilter _usBlurFilter;

        QuadView _quadView;
    };
}

#endif // ADVANCEDUSVIS_H__
