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

#ifndef CAMPVIS_COREAPI_H__
#define CAMPVIS_COREAPI_H__

#include "sigslot/sigslot.h"

#ifdef CAMPVIS_DYNAMIC_LIBS
    #ifdef CAMPVIS_CORE_BUILD_DLL
        // building library -> export symbols
        #ifdef WIN32
            #define CAMPVIS_CORE_API __declspec(dllexport)
        #else
            #define CAMPVIS_CORE_API
        #endif
    #else
        // including library -> import symbols
        #ifdef WIN32
            #define CAMPVIS_CORE_API __declspec(dllimport)
        #else
            #define CAMPVIS_CORE_API
        #endif
    #endif
    #ifdef DLL_TEMPLATE_INST
        // declare and instantiate sigslot templates for symbol ex-/import
        class CAMPVIS_CORE_API sigslot::SIGSLOT_DEFAULT_MT_POLICY;
        template class CAMPVIS_CORE_API sigslot::has_slots<sigslot::SIGSLOT_DEFAULT_MT_POLICY>;
    #endif
#else
    // building/including static library -> do nothing
    #define CAMPVIS_CORE_API
#endif

#endif // CAMPVIS_COREAPI_H__