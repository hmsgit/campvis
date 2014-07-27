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

#ifndef THREADING_H__
#define THREADING_H__

// This header provides cross-compiler includes for <thread> and <condition_variable>.
// These two includes are defined by C++11 and supported by GCC and MSVC11+. All other
// implementations must use TBBs compat layer implementation.

#if (_MSC_VER == 1600) //check for MSVC2010
    #ifndef TBB_IMPLEMENT_CPP0X
        #define TBB_IMPLEMENT_CPP0X 1
    #endif
    #include <tbb/compat/condition_variable>
    #include <tbb/compat/thread>
    #include <tbb/mutex.h>

    namespace std {
        // it gets even uglier:
        // A C++11 std::condition_variable requires a std::mutex, which however is not provided
        // by TBB's compat layer. Hence, we use a tbb::mutex and redeclare it as std::mutex.
        using tbb::mutex;
    }    
#else
    #include <condition_variable>
    #include <mutex>
    #include <thread>
#endif

#endif // THREADING_H__
