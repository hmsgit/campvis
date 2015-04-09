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