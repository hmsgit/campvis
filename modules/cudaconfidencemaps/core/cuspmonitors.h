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
    class iteration_monitor : public cusp::default_monitor<ValueType>
    {
        typedef typename cusp::norm_type<ValueType>::type Real;
        typedef cusp::default_monitor<ValueType> super;

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
    class deadline_monitor : public cusp::default_monitor<ValueType>
    {
        typedef typename cusp::norm_type<ValueType>::type Real;
        typedef cusp::default_monitor<ValueType> super;

    public:
        template <typename Vector>
        deadline_monitor(const Vector& b, float milliseconds)
            : super(b, 0, 0.0f, 0.0f), _startTime(tbb::tick_count::now()), _seconds(milliseconds / 1000.0f)
        { }

        template <typename Vector>
        bool finished(const Vector& r)
        {
            // Only if the deadline is reached, stop and compute the error
            if ((tbb::tick_count::now() - _startTime).seconds() > _seconds) {
                super::r_norm = cusp::blas::nrm2(r);
                return true;
            }

            return false;
        }

    private:
        tbb::tick_count _startTime;
        float _seconds;
    };
}
}

#endif // CUSPMONITORS_H__