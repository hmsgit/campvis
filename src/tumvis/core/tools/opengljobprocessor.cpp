// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
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

#include "opengljobprocessor.h"

#include "tgt/assert.h"
#include "tgt/qt/qtcontextmanager.h"
#include "core/tools/job.h"

namespace TUMVis {

    OpenGLJobProcessor::OpenGLJobProcessor()
        : _currentContext(0)
    {
        _jobPool.s_enqueuedJob.connect(this, &OpenGLJobProcessor::OnEnqueuedJob);
    }

    OpenGLJobProcessor::~OpenGLJobProcessor() {
        _jobPool.s_enqueuedJob.disconnect(this);
    }

    void OpenGLJobProcessor::stop() {
        _stopExecution = true;
        _evaluationCondition.notify_all();

        Runnable::stop();
    }

    void OpenGLJobProcessor::run() {
        std::unique_lock<tbb::mutex> lock(CtxtMgr.getGlMutex());

        while (! _stopExecution) {
            OpenGLJob* job = 0;
            while (job = _jobPool.dequeueJob()) {
                if (_currentContext != job->_canvas) {
                    if (_currentContext != 0) {
                        glFinish();
                        LGL_ERROR;
                    }
                    job->_canvas->getContext()->acquire();
                    _currentContext = job->_canvas;
                }

                job->_job->execute();
                delete job->_job;
                delete job;
            }

//            while (! _stopExecution && _jobPool.empty())
                _evaluationCondition.wait(lock);
        }

        // release OpenGL context, so that other threads can access it
        CtxtMgr.releaseCurrentContext();
    }

    void OpenGLJobProcessor::enqueueJob(tgt::GLCanvas* canvas, AbstractJob* job, PriorityPoolPriority priority) {
        _jobPool.enqueueJob(new OpenGLJob(job, canvas), priority);
    }

    void OpenGLJobProcessor::OnEnqueuedJob() {
        _evaluationCondition.notify_all();
    }
}

