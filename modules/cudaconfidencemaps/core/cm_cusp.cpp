#include "cm_cusp.h"
#include "cm_cusp_cuda_exports.h"

CuspConfidenceMapSolver::CuspConfidenceMapSolver(int width, int height, float gradientScaling,
	                                             float alpha, float beta, float gamma)
	: width(width), height(height), gradientScaling(gradientScaling),
	  alpha(alpha), beta(beta), gamma(gamma), gpuData(CUSP_CM_createGpuData(width, height)) { }

CuspConfidenceMapSolver::~CuspConfidenceMapSolver()
{
	CUSP_CM_destroyGpuData(gpuData);
}

void CuspConfidenceMapSolver::createSystem(const unsigned char* image, int width, int height)
{
	if (width != this->width || height != this->height) {
		CUSP_CM_resizeGpuData(*gpuData, width, height);
	}

	this->width = width;
	this->height = height;

	CUSP_CM_buildEquationSystem(*gpuData, image, width, height, alpha, beta, gamma, gradientScaling);
}

void CuspConfidenceMapSolver::setInitialSolution(const std::vector<float> &val)
{
	CUSP_CM_setInitialSolution(*gpuData, val);
}

void CuspConfidenceMapSolver::solve()
{
	CUSP_CM_uploadSystem(*gpuData);
	CUSP_CM_solveSystem(*gpuData, 100, 1e-20);
	CUSP_CM_downloadSolution(*gpuData);
}

const float* CuspConfidenceMapSolver::getSolution(int &width, int &height) const
{
	const float *ptr = CUSP_CM_getSolutionPtr(*gpuData);
	width = width; height = height; // FIXME: height and width are not actually checked against the size of the buffer...
	return ptr;
}
