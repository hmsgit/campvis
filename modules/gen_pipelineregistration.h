#ifdef CAMPVIS_HAS_MODULE_ADVANCEDUSVIS
#include "modules/advancedusvis/pipelines/advancedusvis.h"
#include "modules/advancedusvis/pipelines/cmbatchgeneration.h"

/// Instantiate templated PipelineRegistrar to automatically register this pipeline.
template class campvis::PipelineRegistrar<campvis::AdvancedUsVis>;
template class campvis::PipelineRegistrar<campvis::CmBatchGeneration>;
#endif

#ifdef CAMPVIS_HAS_MODULE_VIS
#include "modules/vis/pipelines/ixpvdemo.h"
#include "modules/vis/pipelines/dvrvis.h"
#include "modules/vis/pipelines/volumerendererdemo.h"
#include "modules/vis/pipelines/volumeexplorerdemo.h"
#include "modules/vis/pipelines/slicevis.h"

template class campvis::PipelineRegistrar<campvis::DVRVis>;
template class campvis::PipelineRegistrar<campvis::IxpvDemo>;
template class campvis::PipelineRegistrar<campvis::SliceVis>;
template class campvis::PipelineRegistrar<campvis::VolumeExplorerDemo>;
template class campvis::PipelineRegistrar<campvis::VolumeRendererDemo>;
#endif

#ifdef HAS_KISSCL
#include "modules/opencl/pipelines/openclpipeline.h"

/// Instantiate templated PipelineRegistrar to automatically register this pipeline.
template class campvis::PipelineRegistrar<campvis::OpenCLPipeline>;
#endif

#ifdef CAMPVIS_HAS_MODULE_SCR_MSK
#include "modules/scr_msk/pipelines/uscompounding.h"
template class campvis::PipelineRegistrar<campvis::UsCompounding>;
#endif

#ifdef CAMPVIS_HAS_MODULE_COLUMBIA
#include "modules/columbia/pipelines/columbia1.h"

/// Instantiate templated PipelineRegistrar to automatically register this pipeline.
template class campvis::PipelineRegistrar<campvis::Columbia1>;
#endif




