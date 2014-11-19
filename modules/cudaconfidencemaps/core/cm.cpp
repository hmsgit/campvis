#include <chrono>
#include <iostream>
#include <iomanip>

#include "cm.h"
#include "cm_cusp.h"

void measureExecution(std::string message, const std::function<void()> &function)
{
	std::cout << "started:  " << message << std::endl;

	auto start = std::chrono::high_resolution_clock::now();

	function();

	auto stop = std::chrono::high_resolution_clock::now();

	float milliseconds = std::chrono::duration_cast<std::chrono::microseconds>(stop-start).count() / 1000.0f;
	std::cout << "finished: " << std::left << std::setw(35) << message << "["
	          << milliseconds << "ms]" << std::endl;
}

std::unique_ptr<ConfidenceMapSolver> createCUSPSolver(int width, int height, float gradientScaling,
                                                      float alpha, float beta, float gamma)
{
	return std::make_unique<CuspConfidenceMapSolver>(width, height, gradientScaling, alpha, beta, gamma);
}