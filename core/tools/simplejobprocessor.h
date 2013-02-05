// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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

#ifndef SIMPLEJOBPROCESSOR_H__
#define SIMPLEJOBPROCESSOR_H__

#include "tgt/singleton.h"
#include "tbb/include/tbb/task_group.h"

namespace campvis {
    /**
     * Singleton class for managing and executing work items (jobs) in an asynchronous way.
     * Enqueued jobs are dispatched via the tbb::task_group interface as single tasks.
     * No priority schemes are implemented by now.
     */
    class SimpleJobProcessor : public tgt::Singleton<SimpleJobProcessor> {
        friend class tgt::Singleton<SimpleJobProcessor>;

    public:
        /**
         * Destructor, cancels all unfinished jobs.
         */
        virtual ~SimpleJobProcessor();

        /**
         * Enqueues the given Job.
         * \param   job         Job to enqueue
         */
        template<class Func>
        void enqueueJob(const Func& f) {
            _taskGroup.run(f);
        };

    protected:
        SimpleJobProcessor();

        tbb::task_group _taskGroup;
    };

}

#define SimpleJobProc tgt::Singleton<SimpleJobProcessor>::getRef()

#endif // SIMPLEJOBPROCESSOR_H__
