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

#include "autoevaluationpipeline.h"
#include "tgt/tgt_gl.h"
#include "tgt/glcanvas.h"
#include "tgt/glcontext.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/tools/job.h"
#include "core/tools/opengljobprocessor.h"
#include "core/tools/simplejobprocessor.h"


namespace campvis {
    const std::string AutoEvaluationPipeline::loggerCat_ = "CAMPVis.core.datastructures.AutoEvaluationPipeline";

    AutoEvaluationPipeline::AutoEvaluationPipeline() 
        : AbstractPipeline()
    {
    }

    AutoEvaluationPipeline::~AutoEvaluationPipeline() {
    }

    void AutoEvaluationPipeline::init() {
        AbstractPipeline::init();

        // connect invalidation of each processor
        for (std::vector<AbstractProcessor*>::iterator it = _processors.begin(); it != _processors.end(); ++it) {
            (*it)->s_invalidated.connect(this, &AutoEvaluationPipeline::onProcessorInvalidated);
        }
    }

    void AutoEvaluationPipeline::deinit() {
        for (std::vector<AbstractProcessor*>::iterator it = _processors.begin(); it != _processors.end(); ++it) {
            (*it)->s_invalidated.disconnect(this);
        }
    
        AbstractPipeline::deinit();
    }

    void AutoEvaluationPipeline::onProcessorInvalidated(AbstractProcessor* processor) {
        if (_canvas == 0)
            return;

        tbb::concurrent_hash_map<AbstractProcessor*, bool>::const_accessor a;
        if (_isVisProcessorMap.find(a, processor)) {
            if (a->second) {
                // is VisualizationProcessor
                GLJobProc.enqueueJob(
                    _canvas, 
                    makeJobOnHeap<AutoEvaluationPipeline, AbstractProcessor*>(this, &AutoEvaluationPipeline::executeProcessorAndCheckOpenGLState, processor), 
                    OpenGLJobProcessor::SerialJob);
            }
            else {
                SimpleJobProc.enqueueJob(makeJob<AutoEvaluationPipeline, AbstractProcessor*, bool>(this, &AutoEvaluationPipeline::executeProcessor, processor, false));
            }
        }
        else {
            tgtAssert(false, "Could not find processor in processor map.");
            LWARNING("Caught invalidation of a non-registered processor!");
        }
    }

    void AutoEvaluationPipeline::addProcessor(AbstractProcessor* processor) {
        _isVisProcessorMap.insert(std::make_pair(processor, (dynamic_cast<VisualizationProcessor*>(processor) != 0)));
        AbstractPipeline::addProcessor(processor);
    }

}
