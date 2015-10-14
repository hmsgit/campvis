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

#include "qtjobprocessor.h"

namespace campvis {
    QtJobProcessor::QtJobProcessor() 
        : QWidget() 
    {
        connect(this, SIGNAL(newJobSignal(cgt::AbstractJob*)), this, SLOT(onJobArrived(cgt::AbstractJob*)));
    }

    QtJobProcessor::~QtJobProcessor()
    {

    }

    void QtJobProcessor::enqueueJob(cgt::AbstractJob* job) {
        emit newJobSignal(job);
    }

    void QtJobProcessor::enqueueJob(std::function<void(void)> fn)
    {
        emit newJobSignal(cgt::makeJobOnHeap(fn));
    }

    void QtJobProcessor::onJobArrived(cgt::AbstractJob* job)
    {
        cgtAssert(job, "Job must not be null!");

        (*job)();
        delete job;
    }

}

