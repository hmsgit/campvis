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

#ifndef CUDAUTILS_H__
#define CUDAUTILS_H__

namespace campvis {
namespace cuda {

    class CUDAClock
    {
    public:
        CUDAClock() {
            cudaEventCreate(&_start);
            cudaEventCreate(&_stop);
        }

        ~CUDAClock() {
            cudaEventDestroy(_start);
            cudaEventDestroy(_stop);
        }

        void start() {
            cudaEventRecord(_start, 0);  
        }

        float getElapsedMilliseconds() {
            cudaEventRecord(_stop, 0);
            cudaEventSynchronize(_stop);

            float milliseconds;
            cudaEventElapsedTime(&milliseconds, _start, _stop);
            return milliseconds;
        }

    private:
        cudaEvent_t _start, _stop;
    };
}
}

#endif // CUDAUTILS_H__