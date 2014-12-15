#include "cudaconfidencemaps_cuda.h"
#include "cudautils.h"

#include <cusp/dia_matrix.h>
#include <cusp/krylov/cg.h>
#include <cusp/precond/diagonal.h>

typedef cusp::dia_matrix<int, float, cusp::host_memory> CuspHostDiaMatrix;
typedef cusp::dia_matrix<int, float, cusp::device_memory> CuspDeviceDiaMatrix;
typedef cusp::array1d<float, cusp::host_memory> CuspHostVector;
typedef cusp::array1d<float, cusp::device_memory> CuspDeviceVector;
typedef cusp::array1d<unsigned char, cusp::device_memory> CuspDeviceByteVector;

/* The implicit graph created by this class to solve the confidence maps problem is as follows
 *
 *   1   1   1   1
 *   |   |   |   |  << These edges have always value 1
 *   #---#---#---#  \
 *   | X | X | X |  |   The graph here is densly connected (8 neighbourhood). The weight
 *   #---#---#---#  |   of all the edges here are computed based on the intensity differences
 *   | X | X | X |  >   of neighbouring pixels. Each node (#) maps to one pixel in the image.
 *   #---#---#---#  |   Overall, the equation system Lx = b represents a diffusion problem.
 *   | X | X | X |  |   By construction L is SPD.
 *   #---#---#---#  /
 *   |   |   |   |  << These edges have always value 1
 *   0   0   0   0
 */

namespace campvis {
namespace cuda {

    /**
     * This struct is used as a sort of Private Implementation of the \see CudaConfidenceMapsSystemSolver
     * class (PIMPL idiom). This is necessary, since it is not possible to expose CUSP types, such as
     * vectors and matrices to code that is not compiled using the CUDA compilers
     */
    struct CudaConfidenceMapsSystemGPUData {
        // Data on the host (only solution vector x and matrix L)
        CuspHostDiaMatrix L_h;
        CuspHostVector x_h;

        // Data on the device, a.k.a. GPU (Lx = b)
        CuspDeviceDiaMatrix L_d;
        CuspDeviceVector b_d;
        CuspDeviceVector x_d;

        // An additional image-sized 8-bit buffer allocated on the gpu that can be used as intermediate
        // storage when needed (e.g. when creating the system of equations on the gpu)
        // This avoids allocations on the GPU on every iteration
        CuspDeviceByteVector imageBuffer_d;

        // Information about the system as well as statistics of the solution
        bool isUpsideDown;
        int imageWidth;
        int imageHeight;
        int iterationCount;
        float solutionResidualNorm;
        float systemCreationTime;
        float systemSolveTime;
    };

    CudaConfidenceMapsSystemSolver::CudaConfidenceMapsSystemSolver()
        : _gpuData(new CudaConfidenceMapsSystemGPUData())
    {
        _gpuData->imageWidth = 0;
        _gpuData->imageHeight = 0;
        _gpuData->solutionResidualNorm = 0.0f;
        _gpuData->systemCreationTime = 0.0f;
        _gpuData->systemSolveTime = 0.0f;
    }

    CudaConfidenceMapsSystemSolver::~CudaConfidenceMapsSystemSolver()
    {
        // This also frees all the memory reserved on the GPU
        delete _gpuData;
    }

    void CudaConfidenceMapsSystemSolver::uploadImage(const unsigned char* imageData, int imageWidth, int imageHeight,
                                                     float gradientScaling, float alpha, float beta, float gamma,
                                                     bool useGPU, bool isUpsideDown) {
        resizeDataStructures(imageWidth, imageHeight, isUpsideDown);

        // Measure execution time and record it in the _gpuData datastructure
        CUDAClock clock; clock.start();

        if (useGPU) {
            createSystemGPU(imageData, imageWidth, imageHeight, gradientScaling, alpha, beta, gamma, isUpsideDown);
        } else {
            createSystemCPU(imageData, imageWidth, imageHeight, gradientScaling, alpha, beta, gamma, isUpsideDown);
        }

        _gpuData->systemCreationTime = clock.getElapsedMilliseconds();
    }

    void CudaConfidenceMapsSystemSolver::resetSolution() {
        int width = _gpuData->imageWidth;
        int height = _gpuData->imageHeight;
        bool isUpsideDown = _gpuData->isUpsideDown;

        // Compute a linear transition image from white to black and set it as current
        // solution vector
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float value;
                if (isUpsideDown) {
                    value = y / (height - 1.0f);
                }
                else {
                    value = 1.0f - y / (height - 1.0f);
                }

                _gpuData->x_h[y * width + x] = value;
            }
        }

        // Uplaod solution vector on the GPU
        _gpuData->x_d = _gpuData->x_h;
        std::cout << "Reset!" << std::endl;
    }

    void CudaConfidenceMapsSystemSolver::solve(int maximumIterations, float errorTolerance) {
        // Measure execution time and record it in the _gpuData datastructure
        CUDAClock clock; clock.start();

        // The solution is computed using Conjugate Gradient with a Diagonal (Jacobi) preconditioner
        cusp::default_monitor<float> monitor(_gpuData->b_d, maximumIterations, errorTolerance);
        cusp::precond::diagonal<float, cusp::device_memory> M(_gpuData->L_d);
        cusp::krylov::cg(_gpuData->L_d, _gpuData->x_d, _gpuData->b_d, monitor, M);
        _gpuData->solutionResidualNorm = monitor.residual_norm();
        _gpuData->iterationCount = monitor.iteration_count();

        // Downlaod the solution, which has been computed on the GPU to the CPU
        _gpuData->x_h = _gpuData->x_d;

        _gpuData->systemSolveTime = clock.getElapsedMilliseconds();
    }

    const float* CudaConfidenceMapsSystemSolver::getSolution(int& width, int& height) {
        width = _gpuData->imageWidth;
        height = _gpuData->imageHeight;
        return thrust::raw_pointer_cast(&_gpuData->x_h[0]);
    }

    int CudaConfidenceMapsSystemSolver::getSolutionIterationCount() const {
        return _gpuData->iterationCount;
    }


    float CudaConfidenceMapsSystemSolver::getSolutionResidualNorm() const {
        return _gpuData->solutionResidualNorm;
    }

    float CudaConfidenceMapsSystemSolver::getSystemCreationTime() const {
        return _gpuData->systemCreationTime;
    }
    
    float CudaConfidenceMapsSystemSolver::getSystemSolveTime() const {
        return _gpuData->systemSolveTime;
    }


    void CudaConfidenceMapsSystemSolver::resizeDataStructures(int imageWidth, int imageHeight, bool isUpsideDown) {
        // If the problem size changed, reset the solution vector, as well as all
        // the vectors and matrices
        if (_gpuData->imageWidth != imageWidth || _gpuData->imageHeight != imageHeight || _gpuData->isUpsideDown != isUpsideDown) {
            // Resize the system vectors and matrices to accomodate the different image size
            _gpuData->imageWidth = imageWidth;
            _gpuData->imageHeight = imageHeight;
            _gpuData->isUpsideDown = isUpsideDown;
            int numElements = imageWidth * imageHeight;
            _gpuData->x_h.resize(numElements);
            _gpuData->b_d.resize(numElements);
            _gpuData->x_d.resize(numElements);
            _gpuData->imageBuffer_d.resize(numElements);
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

    // Utility Structure to group together all the data that is needed to compute the equation system
    struct ComputeLaplacianData
    {
        // User-tweakable parameters
        float alpha, beta, gamma;
        float gradientScaling;

        // Image Data
        const unsigned char *image;
        int width, height;

        // DIA matrix structure
        int centralDiagonal;
        int offsets[9];

        // Penalty terms for each neighbour
        float gammaList[9];

        // Cached values for the image attenuation (controlled by alpha), so that they don't need to
        // be recomputed at each pixel
        std::vector<float> attenuationLUT;
    };

    /**
     * Returns the weight of the edge corresponding to the pixel at (x, y) and it's neighbour, specified
     * by the diagonal parameter (0: top-left, 8: bottom-right)
     */
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

    void CudaConfidenceMapsSystemSolver::createSystemCPU(const unsigned char* imageData, int imageWidth, int imageHeight,
                                                        float gradientScaling, float alpha, float beta, float gamma,
                                                        bool isUpsideDown) {
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

        // Precompute attenuation tables (controlled by the alpha parameter) so that they don't need
        // to be recomputed for each pixel
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

                // The filter indicates which edges are currently valid. (E.g. when the current pixel
                // has x = 0, all the edges facing left are invalid).
                // The 9th bit corresponds to the top-left edge, while the least-significant-bit corresponds
                // to the bottom-right corner 
                unsigned short filter = 495; // 111 101 111

                // 8 - neighbourhood filter
                if (x == 0)             filter &= 203; // 011 001 011
                if (x == imageWidth-1)  filter &= 422; // 110 100 110
                if (y == 0)             filter &=  47; // 000 101 111
                if (y == imageHeight-1) filter &= 488; // 111 101 000

                // If the pixel is in the first or last row, one edge is virtually missing (the one extending
                // vertically to the seed poits). Its value is accounted for in the b vector, however it still
                // needs to be accounted for here in the diagonal of the matrix.
                float valueSum = 0.0f;
                if (y == 0 || y == imageHeight - 1) valueSum = 1.0f;

                for (int d = 0; d < 9; ++d) {
                    float value = 0.0f;
                    
                    if (((256>>d) & filter) != 0) {
                        value = _getWeight(data, x, y, d);
                    }

                    _gpuData->L_h.values(idx, d) = -value;
                    valueSum += value;
                }

                _gpuData->L_h.values(idx, data.centralDiagonal) = valueSum;
            }
        }

        // Upload system
        _gpuData->L_d = _gpuData->L_h;
    }

    static __device__ float d_getWeight(float v1, float v2, float gradientScaling, float beta, float gamma)
    {
        float grad = abs(v1 - v2) * gradientScaling / 255.0f;
        return exp(-beta * (grad + gamma)) + 1e-4;
    }

    static __global__ void k_buildSystem(float* L, int pitch, const unsigned char* image, int width, int height,
                                  float gradientScaling, float alpha, float beta, float gamma, bool isUpsideDown)
    {

        const int x = blockDim.x * blockIdx.x + threadIdx.x;
        const int y = blockDim.y * blockIdx.y + threadIdx.y;
        const int pidx = y * width + x;
        if (x >= width || y >= height) return;

        const float gamma_sq2 = gamma * 1.4142; // Fixme....
        const int offsets[] = {-width-1, -width, -width+1, -1, 0, 1, width-1, width, width+1};
        const float gammas[] = {gamma_sq2, 0.0f, gamma_sq2, gamma, 0.0f, gamma, gamma_sq2, 0.0f, gamma_sq2};

        // Precompute the three attenuation values for the curernt pixel (the row above, current row, and row below)
        float attenuations[3];
        if (isUpsideDown) {
            attenuations[0] = 1.0f - exp(-alpha * (1.0f - (y - 1.0f) / (height - 1.0f)));
            attenuations[1] = 1.0f - exp(-alpha * (1.0f - (y       ) / (height - 1.0f)));
            attenuations[2] = 1.0f - exp(-alpha * (1.0f - (y + 1.0f) / (height - 1.0f)));
        } else {
            attenuations[0] = 1.0f - exp(-alpha * (y - 1.0f) / (height - 1.0f));
            attenuations[1] = 1.0f - exp(-alpha * (y       ) / (height - 1.0f));
            attenuations[2] = 1.0f - exp(-alpha * (y + 1.0f) / (height - 1.0f));
        }

        // Filter off out-of-bounds edges
        unsigned short filter = 495; // 111 101 111

        // 8 - neighbourhood filter
        if (x == 0)        filter &= 203; // 011 001 011
        if (x == width-1)  filter &= 422; // 110 100 110
        if (y == 0)        filter &=  47; // 000 101 111
        if (y == height-1) filter &= 488; // 111 101 000

        // get central pixel
        float centralValue = image[pidx] * attenuations[1];

        // If the pixel is at the top or at the bottom, add a value of 1 to the diagonal, to
        // account for the edge to the seed points
        float valueSum = 0.0f;
        if (y == 0 || y == height - 1)
            valueSum = 1.0f;

        for (int d = 0; d < 9; ++d) {
            float value = 0.0f;
            
            if (((256>>d) & filter) != 0) {
                int pidx_2 = pidx + offsets[d];
                float v = image[pidx_2] * attenuations[d/3];
                value = d_getWeight(centralValue, v, gradientScaling, beta, gammas[d]);
            }

            // The matrix stores the data, so that values on the same diagonal are sequential.
            // This means that all the values from [0, pitch) are on the first diagonal, [pitch, 2*pitch)
            // are on the second diagonal and so on...
            L[d * pitch + pidx] = -value;
            valueSum += value;
        }
        L[4 * pitch + pidx] = valueSum;
    }

    void CudaConfidenceMapsSystemSolver::createSystemGPU(const unsigned char* imageData, int imageWidth, int imageHeight,
                                                     float gradientScaling, float alpha, float beta, float gamma,
                                                     bool isUpsideDown) {
        // Initialize the DIA matrix diagonal offsets
        int offsets[9] = {-imageWidth-1, -imageWidth, -imageWidth+1, -1, 0, 1, imageWidth-1, imageWidth, imageWidth+1};
        for (int i = 0; i < 9; ++i) {
            _gpuData->L_d.diagonal_offsets[i] = offsets[i];
        }

        int numElements = imageWidth * imageHeight;
        dim3 dimBlock(32, 32, 1);
        dim3 dimGrid((imageWidth + 31) / 32, (imageHeight + 31) / 32, 1);

        // Since the image will be needed by the CUDA kernel, it needs to be copied on the GPU first
        cudaMemcpy(thrust::raw_pointer_cast(&_gpuData->imageBuffer_d[0]), imageData, numElements, cudaMemcpyHostToDevice);
        k_buildSystem<<<dimGrid, dimBlock>>>(thrust::raw_pointer_cast(&_gpuData->L_d.values.values[0]), _gpuData->L_d.values.pitch,
                                             thrust::raw_pointer_cast(&_gpuData->imageBuffer_d[0]),
                                             imageWidth, imageHeight,
                                             gradientScaling, alpha, beta, gamma, _gpuData->isUpsideDown);
    }

} // cuda
} // campvis