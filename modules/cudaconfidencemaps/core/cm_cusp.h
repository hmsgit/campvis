#pragma once

#include "cm.h"

class CuspGPUData;

/* Laplacian construction parameters */
class CuspConfidenceMapSolver : public ConfidenceMapSolver
{
public:
	CuspConfidenceMapSolver(int width, int height, float gradientScaling,
	                        float alpha, float beta, float gamma);

	virtual ~CuspConfidenceMapSolver();

	virtual void createSystem(const uint8* image, int width, int height);
	virtual void setInitialSolution(const std::vector<float> &val);
	virtual void solve();

	virtual const float* getSolution(int &width, int &height) const;

private:
	// Input image data
	int width, height;

	// Solver parameters
	float alpha, beta, gamma;
	float gradientScaling;

	// Matrices and Vectors
	CuspGPUData* gpuData;
};