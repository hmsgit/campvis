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

#include "pipelineevaluator.h"
#include "tgt/qt/qtcontextmanager.h"
#include "core/pipeline/abstractpipeline.h"

namespace campvis {
    PipelineEvaluator::PipelineEvaluator(AbstractPipeline* pipeline)
        : Runnable()
        , _pipeline(pipeline)
    {
        tgtAssert(pipeline != 0, "Pipeline must not be 0.");
        pipeline->s_PipelineInvalidated.connect(this, &PipelineEvaluator::OnPipelineInvalidated);
    }

    PipelineEvaluator::~PipelineEvaluator() {
        _pipeline->s_PipelineInvalidated.disconnect(this);
    }

    void PipelineEvaluator::run() {
        std::unique_lock<tbb::mutex> lock(_pipeline->_evaluationMutex);

        while (! _stopExecution) {
            if (_pipeline->getEnabled())
                _pipeline->execute();

            while (!_stopExecution && _pipeline->getInvalidationLevel().isValid())
                _evaluationCondition.wait(lock);
        }

        // release OpenGL context, so that other threads can access it
        CtxtMgr.releaseCurrentContext();
    }

    void PipelineEvaluator::stop() {
        // we need to execute run() one more time to ensure correct release of the OpenGL context
        _stopExecution = true;
        _evaluationCondition.notify_all();

        Runnable::stop();
    }

    void PipelineEvaluator::OnPipelineInvalidated() {
        if (!_stopExecution)
            _evaluationCondition.notify_all();
    }

}
