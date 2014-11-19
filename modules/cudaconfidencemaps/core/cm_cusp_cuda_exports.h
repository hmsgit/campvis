#pragma once

#include <vector>

class CuspGPUData;

CuspGPUData* CUSP_CM_createGpuData(int width, int height);
void CUSP_CM_destroyGpuData(CuspGPUData *data);
void CUSP_CM_resizeGpuData(CuspGPUData& data, int width, int height);
void CUSP_CM_buildEquationSystem(CuspGPUData& data, const unsigned char* image, int width, int height,
                                 float alpha, float beta, float gamma,
                                 float gradientScaling);
void CUSP_CM_setInitialSolution(CuspGPUData& data, const std::vector<float>& values);
void CUSP_CM_uploadSystem(CuspGPUData &data);
void CUSP_CM_downloadSolution(CuspGPUData &data);
void CUSP_CM_solveSystem(CuspGPUData& data, int iterations, float precision);
const float* CUSP_CM_getSolutionPtr(const CuspGPUData& data);