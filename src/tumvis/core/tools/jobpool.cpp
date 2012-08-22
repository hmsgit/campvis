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

#include "jobpool.h"

#include "tgt/assert.h"
#include "core/tools/job.h"

namespace TUMVis {

    const size_t JobPool::NUM_PRIORITIES = 3;

    JobPool::JobPool()
        : _queues(0)
    {
        _queues = new tbb::concurrent_queue<AbstractJob*>[NUM_PRIORITIES];
    }

    JobPool::~JobPool() {
        // delete jobs
        AbstractJob* toDelete = 0;
        for (size_t i = 0; i < NUM_PRIORITIES; ++i) {
            while (_queues[i].try_pop(toDelete))
                delete toDelete;
        }

        // delete queues
        delete[] _queues;
    }

    void JobPool::enqueueJob(AbstractJob* job, JobPriority priority) {
        size_t i = static_cast<size_t>(priority);
        tgtAssert(i < NUM_PRIORITIES, "Job priority index must be lower than the total number or priorities.");
        tgtAssert(job != 0, "Job must not be 0");

        _queues[i].push(job);
        s_enqueuedJob();
    }

    AbstractJob* JobPool::dequeueJob() {
        // very simple scheduling algorithm. This should be made fairer and avoid starving!
        AbstractJob* toReturn = 0;
        for (size_t i = 0; i < NUM_PRIORITIES; ++i) {
            if (_queues[i].try_pop(toReturn));
            return toReturn;
        }

        return 0;
    }

}

