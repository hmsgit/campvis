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

#ifndef ITKSEGMENTATIONDEMO_H__
#define ITKSEGMENTATIONDEMO_H__

#include "core/pipeline/autoevaluationpipeline.h"

#include "modules/modulesapi.h"

#include "modules/base/processors/lightsourceprovider.h"
#include "modules/itk/processors/itkreader.h"
#include "modules/itk/processors/itkimagefilter.h"
#include "modules/itk/processors/itksegmentation.h"

namespace campvis {
    class CAMPVIS_MODULES_API ItkSegmentationDemo : public AutoEvaluationPipeline {
    public:
        /**
         * Creates a ItkSegmentationDemo pipeline.
         * \param   dataContainer   Reference to the DataContainer containing local working set of data
         *                          for this pipeline, must be valid the whole lifetime of this pipeline.
         */
        explicit ItkSegmentationDemo(DataContainer& dataContainer);

        /**
        * Virtual Destructor
        **/
        virtual ~ItkSegmentationDemo();

        /// \see AutoEvaluationPipeline::init()
        virtual void init();
        /// \see AutoEvaluationPipeline::deinit()
        virtual void deinit();

        static const std::string getId() { return "ItkSegmentationDemo"; };

    protected:
        LightSourceProvider _lsp;
        ItkReader _imageReader;
        ItkImageFilter _itkFilter;
        ItkSegmentation _itkSegmentation;
    };
}

#endif // ITKSEGMENTATIONDEMO_H__
