// ================================================================================================
// 
// This file is part of the CAMPVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef PIPELINEEVALUATOR_H__
#define PIPELINEEVALUATOR_H__

#include "sigslot/sigslot.h"
#include "core/tools/runnable.h"
#include "tbb/include/tbb/compat/condition_variable"

namespace campvis {
    class AbstractPipeline;

    /**
     * The PipelineEvaluator evaluates its pipeline in its own thread.
     * Evaluation is implemented using condidional wait - hence the pipeline is only evaluated when
     * \a pipeline emits the s_PipelineInvalidated signal.
     * 
     * \todo    Let one PipelineEvaluator evaluate multiple pipelines.
     * \sa      Runnable
     */
    class PipelineEvaluator : public Runnable, public sigslot::has_slots<> {
    public:
        /**
         * Creates a new PipelineEvaluator for the given pipeline \a pipeline.
         * \param   pipeline    Pipeline to evaluate
         */
        PipelineEvaluator(AbstractPipeline* pipeline);

        /**
         * Destructor, stops and waits for the evaluation thread if it's still running.
         */
        virtual ~PipelineEvaluator();

        /// \see Runnable::stop
        void stop();
        
        /**
         * Performs the pipeline evaluation using conditional wait.
         * \sa Runnable::run
         */
        void run();

        /**
         * Slot for notifications when the pipeline was invalidated.
         */
        void OnPipelineInvalidated();

    protected:
        AbstractPipeline* _pipeline;                    ///< Pipeline to evaluate
        std::condition_variable _evaluationCondition;   ///< conditional wait condition
    };

}

#endif // PIPELINEEVALUATOR_H__
