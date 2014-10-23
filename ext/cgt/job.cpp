// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#include "job.h"

#include <memory>
#include <new>

#include "cgt/opengljobprocessor.h"

namespace cgt {

    // Implementation inspired by http://stackoverflow.com/questions/7194127/how-should-i-write-iso-c-standard-conformant-custom-new-and-delete-operators/7194149#7194149
    void* AbstractJob::operator new(std::size_t size) throw(std::bad_alloc) {
        if (size == 0)
            size = 1;

        while (true) {
            void* toReturn = OpenGLJobProcessor::getRef()._jobPool.malloc(size);

            if (toReturn != nullptr)
                return toReturn;

            //allocation was unsuccessful; find out what the current new-handling function is (see below)
            std::new_handler globalHandler = std::set_new_handler(0);
            std::set_new_handler(globalHandler);

            if (globalHandler)             //If new_hander is registered call it
                (*globalHandler)();
            else 
                throw std::bad_alloc();   //No handler is registered throw an exception
        }
    }

    void AbstractJob::operator delete(void* rawMemory, std::size_t size) throw() {
        if (rawMemory == nullptr)
            return;

        OpenGLJobProcessor::getRef()._jobPool.free(rawMemory);
    }

}
