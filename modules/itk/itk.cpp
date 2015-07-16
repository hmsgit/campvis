// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "core/pipeline/pipelinefactory.h"
#include "core/pipeline/processorfactory.h"

#include "modules/itk/pipelines/itkfilterdemo.h"
#include "modules/itk/pipelines/itkregistrationdemo.h"
#include "modules/itk/pipelines/itksegmentationdemo.h"

#include "modules/itk/processors/itkimagefilter.h"
#include "modules/itk/processors/itkreader.h"
#include "modules/itk/processors/itkregistration.h"
#include "modules/itk/processors/itksegmentation.h"
#include "modules/itk/processors/itkwatershedfilter.h"

namespace campvis {

    // explicitly instantiate templates to register the pipelines
    template class PipelineRegistrar<ItkFilterDemo>;
    template class PipelineRegistrar<ItkRegistrationDemo>;
    template class PipelineRegistrar<ItkSegmentationDemo>;

    template class SmartProcessorRegistrar<ItkImageFilter>;
    template class SmartProcessorRegistrar<ItkReader>;
    template class SmartProcessorRegistrar<ItkRegistration>;
    template class SmartProcessorRegistrar<ItkSegmentation>;
    template class SmartProcessorRegistrar<ItkWatershedFilter>;

}