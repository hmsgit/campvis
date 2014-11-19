#pragma once

typedef unsigned char uint8;

#include <memory>
#include <vector>
#include <functional>

void measureExecution(std::string message, const std::function<void()> &function);

class ConfidenceMapSolver
{
public:
	virtual ~ConfidenceMapSolver() {}
	virtual void createSystem(const uint8* image, int width, int height) = 0;
	virtual void setInitialSolution(const std::vector<float> &val) = 0;
	virtual void solve() = 0;

	virtual const float* getSolution(int &width, int &height) const = 0;
};

std::unique_ptr<ConfidenceMapSolver> createCUSPSolver(int width, int height, float gradientScaling,
                                                      float alpha, float beta, float gamma);
/*std::unique_ptr<ConfidenceMapSolver> createEIGENSolver(int width, int height, float gradientScaling,
                                                       float alpha, float beta, float gamma);*/