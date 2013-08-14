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

#include "application/campvisapplication.h"
#include "modules/advancedusvis/pipelines/advancedusvis.h"
#include "modules/advancedusvis/pipelines/cmbatchgeneration.h"
#include "modules/vis/pipelines/ixpvdemo.h"
#include "modules/vis/pipelines/dvrvis.h"
#include "modules/vis/pipelines/volumerendererdemo.h"
#include "modules/vis/pipelines/slicevis.h"
#ifdef HAS_KISSCL
#include "modules/opencl/pipelines/openclpipeline.h"
#endif

#ifdef CAMPVIS_HAS_MODULE_SCR_MSK
#include "modules/scr_msk/pipelines/uscompounding.h"
#endif

#ifdef CAMPVIS_HAS_MODULE_COLUMBIA
#include "modules/columbia/pipelines/columbia1.h"
#endif

using namespace campvis;

/**
 * CAMPVis main function, application entry point
 *
 * \param   argc    number of passed arguments
 * \param   argv    vector of arguments
 * \return  0 if program exited successfully
 **/
int main(int argc, char** argv) {
    CampVisApplication app(argc, argv);
    //app.addVisualizationPipeline("Advanced Ultrasound Visualization", new AdvancedUsVis());
    //app.addVisualizationPipeline("Confidence Map Generation", new CmBatchGeneration());
//    app.addVisualizationPipeline("IXPV", new IxpvDemo());
    //app.addVisualizationPipeline("SliceVis", new SliceVis());
    app.addVisualizationPipeline("DVRVis", new DVRVis());
    //app.addVisualizationPipeline("VolumeRendererDemo", new VolumeRendererDemo());
#ifdef HAS_KISSCL
    //app.addVisualizationPipeline("DVR with OpenCL", new OpenCLPipeline());
#endif

#ifdef CAMPVIS_HAS_MODULE_SCR_MSK
    //app.addVisualizationPipeline("US Compounding", new UsCompounding());
#endif

#ifdef CAMPVIS_HAS_MODULE_COLUMBIA
    app.addVisualizationPipeline("Columbia", new Columbia1());
#endif



    app.init();
    int toReturn = app.run();
    app.deinit();

    return toReturn;
}
