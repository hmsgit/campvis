#pragma once

#include <vector>

class CuspGPUData;
class CuspConfidenceMapSolver
{
public:
    CuspConfidenceMapSolver(int width, int height);

    virtual ~CuspConfidenceMapSolver();

    virtual void createSystem(const unsigned char* image, int width, int height,
                              float gradientScaling, float alpha, float beta, float gamma);
    virtual void setInitialSolution(const std::vector<float> &val);
    virtual void solve(int iterations);

    virtual const float* getSolution(int &width, int &height) const;

private:
    // Input image data
    int width, height;

    // Matrices and Vectors
    CuspGPUData* gpuData;
};