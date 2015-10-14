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

#include "core/pipeline/pipelinefactory.h"
#include "core/pipeline/processorfactory.h"

#include "modules/vis/pipelines/advdvrvis.h"
#include "modules/vis/pipelines/dvrvis.h"
#include "modules/vis/pipelines/geometryrendererdemo.h"
#include "modules/vis/pipelines/mprdemo.h"
#include "modules/vis/pipelines/slicevis.h"
#include "modules/vis/pipelines/volumeexplorerdemo.h"
#include "modules/vis/pipelines/volumerendererdemo.h"

#include "modules/vis/processors/advoptimizedraycaster.h"
#include "modules/vis/processors/contextpreservingraycaster.h"
#include "modules/vis/processors/depthdarkening.h"
#include "modules/vis/processors/drrraycaster.h"
#include "modules/vis/processors/eepgenerator.h"
#include "modules/vis/processors/geometryrenderer.h"
#include "modules/vis/processors/mprrenderer.h"
#include "modules/vis/processors/orientationoverlay.h"
#include "modules/vis/processors/proxygeometrygenerator.h"
#include "modules/vis/processors/quadview.h"
#include "modules/vis/processors/rendertargetcompositor.h"
#include "modules/vis/processors/simpleraycaster.h"
#include "modules/vis/processors/sliceextractor.h"
#include "modules/vis/processors/slicerenderer2d.h"
#include "modules/vis/processors/slicerenderer3d.h"
#include "modules/vis/processors/virtualmirrorcombine.h"
#include "modules/vis/processors/virtualmirrorgeometrygenerator.h"
#include "modules/vis/processors/volumeexplorer.h"
#include "modules/vis/processors/volumerenderer.h"

namespace campvis {

    // explicitly instantiate templates to register the pipelines
    template class PipelineRegistrar<AdvDVRVis>;
    template class PipelineRegistrar<DVRVis>;
    template class PipelineRegistrar<GeometryRendererDemo>;
    template class PipelineRegistrar<MprDemo>;
    template class PipelineRegistrar<SliceVis>;
    template class PipelineRegistrar<VolumeRendererDemo>;
    template class PipelineRegistrar<VolumeExplorerDemo>;

    template class SmartProcessorRegistrar<AdvOptimizedRaycaster>;
    template class SmartProcessorRegistrar<ContextPreservingRaycaster>;
    template class SmartProcessorRegistrar<DepthDarkening>;
    template class SmartProcessorRegistrar<DRRRaycaster>;
    template class SmartProcessorRegistrar<EEPGenerator>;
    template class SmartProcessorRegistrar<GeometryRenderer>;
    template class SmartProcessorRegistrar<MprRenderer>;
    template class SmartProcessorRegistrar<OrientationOverlay>;
    template class SmartProcessorRegistrar<ProxyGeometryGenerator>;
    template class SmartProcessorRegistrar<QuadView>;
    template class SmartProcessorRegistrar<RenderTargetCompositor>;
    template class SmartProcessorRegistrar<SimpleRaycaster>;
    template class SmartProcessorRegistrar<SliceExtractor>;
    template class SmartProcessorRegistrar<SliceRenderer2D>;
    template class SmartProcessorRegistrar<SliceRenderer3D>;
    template class SmartProcessorRegistrar<VirtualMirrorCombine>;
    template class SmartProcessorRegistrar<VirtualMirrorGeometryGenerator>;
    template class SmartProcessorRegistrar<VolumeExplorer>;
    template class SmartProcessorRegistrar<VolumeRenderer>;
}