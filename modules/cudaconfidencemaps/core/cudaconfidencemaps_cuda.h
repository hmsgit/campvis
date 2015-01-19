#ifndef CUDADCONFIDENCEMAPS_CUDA_H__
#define CUDADCONFIDENCEMAPS_CUDA_H__

namespace campvis {
namespace cuda {

    struct CudaConfidenceMapsSystemGPUData;

    class CudaConfidenceMapsSystemSolver {
    public:

        CudaConfidenceMapsSystemSolver();
        ~CudaConfidenceMapsSystemSolver();

        /**
         * Uploads an image to the solver. Additionally the matrices and  vectors that are needed to
         * solve the system are created. After calling this function it is possible to call \see solve() in order
         * to start computing confidence maps.
         * \param   imageData   pointer to a buffer containing the grayscale image 8-bit image data
         * \param   imageWidth  width of the image
         * \param   imageHeight height of the image
         * \param   gradientScaling multiplication applied to the computed image gradients
         * \param   alpha       controls the depth attenuation correction
         * \param   beta        controls the non-linear mapping of gradients to weights
         * \param   gamma       controls how much diagonal connections are penalized
         * \param   use8Neighbourhood wether to use a graph connecting all 8 neighbours of a pixel or not. The original
         *                      confidence maps problem formulation uses 8 neighbours. 
         * \param   isUpsideDown if set to true, the image is interpreted as being upside down (as common in OpenGL)
         */
        void uploadImage(const unsigned char* imageData, int imageWidth, int imageHeight,
                         float gradientScaling, float alpha, float beta, float gamma, bool use8Neighbourhood=false, bool isUpsideDown=true);

        /**
         * Resets the current solution vector to a linear fallof from white (top of the image) to black
         * (bottom of the image). Note the calling \see uploadImage() for the first time or with a different
         * image size, also results in the solution being reset.
         */
        void resetSolution();

        /**
         * Returns wether or not the final image is smoothed using the alpha beta filter
         * \param  alpha  controls the changes in X. Must be in the range (0, 1)
         * \param  beta   controls the changes in V. Must be in the range (0, 2) and < 1 if one aims at reducing noise
         */
        bool alphaBetaFilterEnabled() const;

        /**
         * Enables or disables the alpha-beta filtering of the output
         * \param  enabled  wether or not to enable the alpha-beta filtering
         */
        void enableAlphaBetaFilter(bool enabled);

        /**
         * Sets the parameters needed by the alpha-beta filter
         */
        void setAlphaBetaFilterParameters(float alpha, float beta);

        /**
         * After calling \see uploadImage(), this functions launches a solver on the GPU that will solve
         * the diffusion problem.
         * \param   maximumIterations maximum number of iterations the solver will preform
         * \param   errorTolerance    if the solution error sinks below this value, the solver stops early
         */
        void solve(int maximumIterations, float errorTolerance);

        /**
         * Returns a host buffer of the last solution computed by the solver. The pointer is guaranteed to
         * remain valid until the next call of \see uploadImage() or the object is destructed.
         * \param   width   outputs the width of the image
         * \param   height  outputs the height of the image
         * \return a pointer to a float buffer containing the confidence map
         */
        const float* getSolution(int& width, int& height);

        /**
         * Returns the number of CG iterations that were actually performed when solving the system
         * \see solve() was called
         */
        int getSolutionIterationCount() const;

        /**
         * Returns the residual norm of the solution as a measure of error;
         */
        float getSolutionResidualNorm() const;

        /**
         * Returns the number of milliseconds that were needed to build the equation system
         * when calling \see uploadImage()
         */
        float getSystemCreationTime() const;

        /**
         * Returns the number of milliseconds that were needed to solve the system when
         * \see solve() was called
         */
        float getSystemSolveTime() const;

    private:
        void resizeDataStructures(int imageWidth, int imageHeight, bool isUpsideDown, bool use8Neighbourhood);
        void createSystemGPU(const unsigned char* imageData, int imageWidth, int imageHeight,
                             float gradientScaling, float alpha, float beta, float gamma, bool isUpsideDown=true);


        CudaConfidenceMapsSystemGPUData *_gpuData;
    };

} // cuda
} // campvis

#endif // CUDADCONFIDENCEMAPS_CUDA_H__