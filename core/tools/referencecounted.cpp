// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "referencecounted.h"
#include "core/tools/job.h"
#include "core/tools/simplejobprocessor.h"

namespace campvis {
    ReferenceCounted::ReferenceCounted()
        : _shareable(true)
    {
        _refCount = 0;
    }

    ReferenceCounted::ReferenceCounted(const ReferenceCounted& rhs)
        : _shareable(true)
    {
        _refCount = 0;
    }

    ReferenceCounted& ReferenceCounted::operator=(const ReferenceCounted& rhs) {
        return *this;
    }

    ReferenceCounted::~ReferenceCounted() {
    }

    void ReferenceCounted::addReference() {
        ++_refCount;
    }

    void ReferenceCounted::removeReference() {
        if (--_refCount == 0)
            SimpleJobProc.enqueueJob(makeJob(&ReferenceCounted::deleteInstance, this));
    }

    void ReferenceCounted::markUnsharable() {
        _shareable = false;
    }

    bool ReferenceCounted::isShareable() const {
        return _shareable;
    }

    bool ReferenceCounted::isShared() const {
        return _refCount > 1;
    }

    void ReferenceCounted::deleteInstance(ReferenceCounted* instance) {
        delete instance;
    }

}

