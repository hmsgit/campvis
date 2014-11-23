#pragma once

#include <vector>

class CuspGPUData;
class CuspConfidenceMapSolver
{
public:
	CuspConfidenceMapSolver(int width, int height, float gradientScaling,
	                        float alpha, float beta, float gamma);

	virtual ~CuspConfidenceMapSolver();

	virtual void createSystem(const unsigned char* image, int width, int height);
	virtual void setInitialSolution(const std::vector<float> &val);
	virtual void solve();

	virtual const float* getSolution(int &width, int &height) const;

private:
	// Input image data
	int width, height;

	// Solver parameters
	float gradientScaling;
	float alpha, beta, gamma;

	// Matrices and Vectors
	CuspGPUData* gpuData;
};