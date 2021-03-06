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

#ifndef CAMPVIS_MODULESAPI_H__
#define CAMPVIS_MODULESAPI_H__

#ifdef CAMPVIS_DYNAMIC_LIBS
    #ifdef CAMPVIS_MODULES_BUILD_DLL
        // building library -> export symbols
        #ifdef WIN32
            #define CAMPVIS_MODULES_API __declspec(dllexport)
        #else
            #define CAMPVIS_MODULES_API
        #endif
    #else
        // including library -> import symbols
        #ifdef WIN32
            #define CAMPVIS_MODULES_API __declspec(dllimport)
        #else
            #define CAMPVIS_MODULES_API
        #endif
    #endif
#else
    // building/including static library -> do nothing
    #define CAMPVIS_MODULES_API
#endif

#endif // CAMPVIS_MODULESAPI_H__
