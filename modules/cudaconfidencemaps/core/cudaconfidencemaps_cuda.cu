#include "cudaconfidencemaps_cuda.h"

#include <cusp/dia_matrix.h>
#include <cusp/krylov/cg.h>
#include <cusp/precond/diagonal.h>

typedef cusp::dia_matrix<int, float, cusp::host_memory> CuspHostDiaMatrix;
typedef cusp::dia_matrix<int, float, cusp::device_memory> CuspDeviceDiaMatrix;
typedef cusp::array1d<float, cusp::host_memory> CuspHostVector;
typedef cusp::array1d<float, cusp::device_memory> CuspDeviceVector;

namespace campvis {
namespace cuda {

    struct CudaConfidenceMapsSystemGPUData {
        // Data on the host (only solution vector x and matrix L)
        CuspHostDiaMatrix L_h;
        CuspHostVector x_h;

        // Data on the device, a.k.a. GPU (Lx = b)
        CuspDeviceDiaMatrix L_d;
        CuspDeviceVector b_d;
        CuspDeviceVector x_d;

        // Information about the system
        bool isUpsideDown;
        int imageWidth;
        int imageHeight;
        float solutionResidualNorm;
    };

    struct ComputeLaplacianData
    {
        float alpha, beta, gamma;
        float gradientScaling;
        const unsigned char *image;
        int width, height;
        int centralDiagonal;
        int offsets[9];
        float gammaList[9];

        std::vector<float> attenuationLUT;
    };

    CudaConfidenceMapsSystemSolver::CudaConfidenceMapsSystemSolver()
        : _gpuData(new CudaConfidenceMapsSystemGPUData())
    {
        _gpuData->imageWidth = 0;
        _gpuData->imageHeight = 0;
        _gpuData->solutionResidualNorm = 0.0f;
    }

    static inline float _getWeight(const ComputeLaplacianData &data, int x, int y, int diagonal)
    {
        const unsigned char *image = data.image;

        int idx1 = y * data.width + x;
        int idx2 = idx1 + data.offsets[diagonal];

        float attenuation1 = data.attenuationLUT[idx1 / data.width];
        float attenuation2 = data.attenuationLUT[idx2 / data.width];

        float gradient = abs(image[idx1]*attenuation1/255.0f - image[idx2]*attenuation2/255.0f) * data.gradientScaling;

        float weight = exp(-data.beta * (gradient + data.gammaList[diagonal]));
        return weight + 1e-4;
    }

    void CudaConfidenceMapsSystemSolver::uploadImage(const unsigned char* imageData, int imageWidth, int imageHeight,
                                                     float gradientScaling, float alpha, float beta, float gamma,
                                                     bool isUpsideDown) {
        resizeDataStructures(imageWidth, imageHeight, isUpsideDown);

        // Gather all the parameters needed to create the system in one place
        ComputeLaplacianData data;
        data.alpha = alpha;
        data.beta = beta;
        data.gamma = gamma;
        data.gradientScaling = gradientScaling;
        data.image = imageData;
        data.width = imageWidth;
        data.height = imageHeight;
        data.centralDiagonal = 4;
        int offsets[9] = {-imageWidth-1, -imageWidth, -imageWidth+1, -1, 0, 1, imageWidth-1, imageWidth, imageWidth+1};
        float gammaList[9] = {sqrt(2.0f)*gamma, 0.0f, sqrt(2.0f)*gamma, gamma, 0.0f, gamma, sqrt(2.0f)*gamma, 0.0f, sqrt(2.0f)*gamma};

        for (int i = 0; i < 9; ++i) {
            data.offsets[i] = offsets[i];
            data.gammaList[i] = gammaList[i];
            _gpuData->L_h.diagonal_offsets[i] = offsets[i];
        }

        // Precompute attenuation tables
        data.attenuationLUT = std::vector<float>(imageHeight);
        for (int i = 0; i < imageHeight; ++i) {
            float y = (float)i / (float)(imageHeight-1);
            if (isUpsideDown) y = 1 - y;
            data.attenuationLUT[i] = 1 - exp(-alpha * y);
        }

        // Fill in the rest of the matrix
        for (int y = 0; y < imageHeight; ++y) {
            for (int x = 0; x < imageWidth; ++x) {
                int idx = y * imageWidth + x;

                // Filter off out-of-bounds edges
                unsigned short filter = 495; // 111 101 111

                // 8 - neighbourhood filter
                if (x == 0)        filter &= 203; // 011 001 011
                if (x == imageWidth-1)  filter &= 422; // 110 100 110
                if (y == 0)        filter &=  47; // 000 101 111
                if (y == imageHeight-1) filter &= 488; // 111 101 000

                float valueSum = 0.0f;
                if (y == 0 || y == imageHeight - 1) valueSum = 1.0f;

                for (int d = 0; d < 9; ++d) {
                    _gpuData->L_h.values(idx, d) = 0;

                    float value = 0.0f;
                    
                    if (((256>>d) & filter) != 0) {
                        value = _getWeight(data, x, y, d);
                        _gpuData->L_h.values(idx, d) = -value;
                    }

                    valueSum += value;
                }

                _gpuData->L_h.values(idx, data.centralDiagonal) = valueSum;
            }
        }

        // Upload system
        _gpuData->L_d = _gpuData->L_h;
    }

    void CudaConfidenceMapsSystemSolver::resetSolution() {
        int width = _gpuData->imageWidth;
        int height = _gpuData->imageHeight;
        bool isUpsideDown = _gpuData->isUpsideDown;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float value;
                if (!isUpsideDown)
                    value = y / (height - 1.0f);
                else
                    value = 1.0f - y / (height - 1.0f);

                _gpuData->x_h[y * width + x] = value;
            }
        }

        // Uplaod solution vector on the GPU
        _gpuData->x_d = _gpuData->x_h;
    }

    void CudaConfidenceMapsSystemSolver::solve(int maximumIterations, float errorTolerance) {
        cusp::default_monitor<float> monitor(_gpuData->b_d, maximumIterations, errorTolerance);
        cusp::precond::diagonal<float, cusp::device_memory> M(_gpuData->L_d);
        cusp::krylov::cg(_gpuData->L_d, _gpuData->x_d, _gpuData->b_d, monitor, M);

        _gpuData->solutionResidualNorm = monitor.residual_norm();
    }

    const float* CudaConfidenceMapsSystemSolver::getSolution(int& width, int& height) {
        // Downlaod data
        _gpuData->x_h = _gpuData->x_d;
        return &_gpuData->x_h[0];
    }

    float CudaConfidenceMapsSystemSolver::getSolutionResidualNorm() const {
        return _gpuData->solutionResidualNorm;
    }

    void CudaConfidenceMapsSystemSolver::resizeDataStructures(int imageWidth, int imageHeight, bool isUpsideDown) {
        // If the problem size changed, reset the solution vector, as well as resizing the vectors and matrices
        if (_gpuData->imageWidth != imageWidth || _gpuData->imageHeight != imageHeight || _gpuData->isUpsideDown != isUpsideDown) {
            // Resize the system vectors and matrices to accomodate the different image sze
            _gpuData->imageWidth = imageWidth;
            _gpuData->imageHeight = imageHeight;
            int numElements = imageWidth * imageHeight;
            _gpuData->x_h.resize(numElements);
            _gpuData->b_d.resize(numElements);
            _gpuData->x_d.resize(numElements);
            _gpuData->L_d.resize(numElements, numElements, numElements * 9, 9);
            _gpuData->L_h.resize(numElements, numElements, numElements * 9, 9);

            // Set the b vector to 0 (except for the row corresponding to the seed points set to 1)
            if (isUpsideDown) {
                CuspDeviceVector::view lastRow(_gpuData->b_d.begin() + (numElements - imageWidth), _gpuData->b_d.end());
                CuspDeviceVector::view rest(_gpuData->b_d.begin(), _gpuData->b_d.begin() + (numElements - imageWidth));
                cusp::blas::fill(rest,    0.0f);
                cusp::blas::fill(lastRow, 1.0f);
            }
            else {
                CuspDeviceVector::view firstRow(_gpuData->b_d.begin(), _gpuData->b_d.begin() + imageWidth);
                CuspDeviceVector::view rest(_gpuData->b_d.begin() + imageWidth, _gpuData->b_d.end());
                cusp::blas::fill(firstRow, 1.0f);
                cusp::blas::fill(rest,     0.0f);
            }

            resetSolution();
        }
    }

} // cuda
} // campvis