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

#ifndef CUSPMONITORS_H__
#define CUSPMONITORS_H__

#include <cusp/monitor.h>
#include <tbb/tick_count.h>

namespace campvis {
namespace cuda {

    /**
     * This class allows to execute a fixed number of conjugate gradient iterations
     * in CUSP. Unlike the default_monitor, this class only calculates the residual
     * norm when the iteration count is reached.
     */
    template <typename ValueType>
#ifndef GIT_CUSP_VERSION
    class iteration_monitor : public cusp::default_monitor<ValueType>
    {
        typedef typename cusp::norm_type<ValueType>::type Real;
        typedef cusp::default_monitor<ValueType> super;
#else
    class iteration_monitor : public cusp::monitor<ValueType>
    {
        typedef typename cusp::detail::norm_type<ValueType>::type Real;
        typedef cusp::monitor<ValueType> super;
#endif

    public:
        template <typename Vector>
        iteration_monitor(const Vector& b, size_t iteration_limit = 500)
            : super(b, iteration_limit, 0.0f, 0.0f)
        { }

        template <typename Vector>
        bool finished(const Vector& r)
        {
            // Only if the maximum iteration count has been reached, actually go ahead and
            // compute the error
            if (super::iteration_count() >= super::iteration_limit()) {
                super::r_norm = cusp::blas::nrm2(r);
                return true;
            }

            return false;
        }
    };

    /**
     * This monitor allows to set a deadline, after which the computation has to stop.
     */
    template <typename ValueType>
#ifndef GIT_CUSP_VERSION
    class deadline_monitor : public cusp::default_monitor<ValueType>
    {
        typedef typename cusp::norm_type<ValueType>::type Real;
        typedef cusp::default_monitor<ValueType> super;
#else
    class deadline_monitor : public cusp::monitor<ValueType>
    {
        typedef typename cusp::detail::norm_type<ValueType>::type Real;
        typedef cusp::monitor<ValueType> super;
#endif

    public:
        template <typename Vector>
        deadline_monitor(const Vector& b, float milliseconds)
            : super(b, 0, 0.0f, 0.0f), _startTime(tbb::tick_count::now()), _seconds(milliseconds / 1000.0f)
        { }

        template <typename Vector>
        bool finished(const Vector& r) {
            // Only if the deadline is reached, stop and compute the error
            if ((tbb::tick_count::now() - _startTime).seconds() > _seconds) {
                r_norm = cusp::blas::nrm2(r);
                return true;
            }

            return false;
        }

        Real residual_norm(void) const {
            return r_norm;
        }


    private:
        Real r_norm;

        tbb::tick_count _startTime;
        float _seconds;
    };
}
}

#endif // CUSPMONITORS_H__