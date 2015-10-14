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

#ifndef SIMPLEJOBPROCESSOR_H__
#define SIMPLEJOBPROCESSOR_H__

#include "cgt/singleton.h"
#include <tbb/task_group.h>

#include "core/coreapi.h"

namespace campvis {
    /**
     * Singleton class for managing and executing work items (jobs) in an asynchronous way.
     * Enqueued jobs are dispatched via the tbb::task_group interface as single tasks.
     * No priority schemes are implemented by now.
     */
    class CAMPVIS_CORE_API SimpleJobProcessor : public cgt::Singleton<SimpleJobProcessor> {
        friend class cgt::Singleton<SimpleJobProcessor>;

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

#define SimpleJobProc cgt::Singleton<SimpleJobProcessor>::getRef()

#endif // SIMPLEJOBPROCESSOR_H__
