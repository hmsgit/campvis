#include <vector>
#include <cmath>
#include <cusp/dia_matrix.h>
#include <cusp/krylov/cg.h>
#include <cusp/precond/diagonal.h>

#include "cm_cusp_cuda_exports.h"

typedef cusp::dia_matrix<int, float, cusp::host_memory> CuspHostDiaMatrix;
typedef cusp::dia_matrix<int, float, cusp::device_memory> CuspDeviceDiaMatrix;
typedef cusp::array1d<float, cusp::host_memory> CuspHostVector;
typedef cusp::array1d<float, cusp::device_memory> CuspDeviceVector;

class CuspGPUData {
public:
	CuspGPUData(int width, int height)
		: L_h(width*height, width*height, width*height*9, 9),
	      b_h(width*height), x_h(width*height),
		  L_d(width*height, width*height, width*height*9, 9),
	      b_d(width*height), x_d(width*height) {};

	void resize(int width, int height)
	{
		this->L_h.resize(width*height, width*height, width*height*9, 9);
		this->b_h.resize(width*height);
		this->x_h.resize(width*height);
	}

	CuspHostDiaMatrix L_h;
	CuspHostVector b_h;
	CuspHostVector x_h;

	CuspDeviceDiaMatrix L_d;
	CuspDeviceVector b_d;
	CuspDeviceVector x_d;
};

CuspGPUData* CUSP_CM_createGpuData(int width, int height)
{
	CuspGPUData *data = new CuspGPUData(width, height);
	return data;
}

void CUSP_CM_destroyGpuData(CuspGPUData *data)
{
	delete data;
}

void CUSP_CM_resizeGpuData(CuspGPUData& data, int width, int height)
{
	data.resize(width, height);
}

void CUSP_CM_setInitialSolution(CuspGPUData& data, const std::vector<float>& values)
{
	for (int i = 0; i < data.x_h.size() && i < values.size(); ++i) {
		data.x_h[i] = values[i];
	}
}

void CUSP_CM_uploadSystem(CuspGPUData &data)
{
	data.x_d = data.x_h;
	data.b_d = data.b_h;
	data.L_d = data.L_h;
}

void CUSP_CM_downloadSolution(CuspGPUData &data)
{
	data.x_h = data.x_d;
	// FIXME: Should not be needed... (Clamp solution)
	for (int i = 0; i < data.x_h.size(); ++i) {
		data.x_h[i] = min(1.0f, data.x_h[i]);
	}
}

void CUSP_CM_solveSystem(CuspGPUData& data, int iterations, float precision)
{
	cusp::default_monitor<float> monitor(data.b_d, iterations, precision);
	//cusp::precond::bridson_ainv<float, cusp::device_memory> M(A, 1);
	cusp::precond::diagonal<float, cusp::device_memory> M(data.L_d);
	//cusp::krylov::bicgstab(A, x, b, monitor);
	cusp::krylov::cg(data.L_d, data.x_d, data.b_d, monitor, M);
}

const float* CUSP_CM_getSolutionPtr(const CuspGPUData& data)
{
	//const_cast<CuspGPUData&>(data).x_h = const_cast<CuspGPUData&>(data).x_d;
	return &data.x_h[0];
}




struct ComputeLaplacianData
{
	float alpha, beta, gamma;
	float gradientScaling;
	const unsigned char *image;
	int width, height;
	int centralDiagonal;
	int offsets[9];
	float gammaList[9];
};


static inline float _getAttenuation(int y, int height, float alpha) {
	return (1 - exp(-alpha * ((float)y / (float)(height-1))));
}

static inline float _getGradient(const ComputeLaplacianData &data, int idx, int offset)
{
	const unsigned char *image = data.image;
	int y1 = idx / data.width;
	int y2 = (idx+offset) / data.width;

	float a1 = _getAttenuation(y1, data.height, data.alpha);
	float a2 = _getAttenuation(y2, data.height, data.alpha);

	return abs(image[idx]*a1/255.0f - image[idx+offset]*a2/255.0f);
}

static inline float _calculateWeight(float gradient, float beta, float gamma, float scaling)
{
	return exp(-beta * (gradient*scaling + gamma));
}

static inline float _getWeight(const ComputeLaplacianData &data, int x, int y, int diagonal)
{
	float gradient = _getGradient(data, y * data.width + x, data.offsets[diagonal]);
	float weight = _calculateWeight(gradient, data.beta, data.gammaList[diagonal], data.gradientScaling);
	return weight + 1e-4;
}

void CUSP_CM_buildEquationSystem(CuspGPUData &gpudata, const unsigned char* image, int width, int height,
                                 float alpha, float beta, float gamma,
                                 float gradientScaling)
{
	// Gather all of the options together
	ComputeLaplacianData data;
	data.alpha = alpha; data.beta = beta; data.gamma = gamma;
	data.gradientScaling = gradientScaling;
	data.image = image;
	data.width = width; data.height = height;
	data.centralDiagonal = 4;
	int offsets[9] = {-width-1, -width, -width+1, -1, 0, 1, width-1, width, width+1};
	float gammaList[9] = {sqrt(2.0f)*gamma, 0.0f, sqrt(2.0f)*gamma, gamma, 0.0f, gamma, sqrt(2.0f)*gamma, 0.0f, sqrt(2.0f)*gamma};
	for (int i = 0; i < 9; ++i) {
		data.offsets[i] = offsets[i];
		data.gammaList[i] = gammaList[i];
	}

	// Prepare equation system data structure
	for (int i = 0; i < 9; ++i) {
		gpudata.L_h.diagonal_offsets[i] = data.offsets[i];
	}

	// Reset B
	for (int x = 0; x < width * height; ++x) {
		gpudata.b_h[x] = 0.0f;
	}

	// Fill in first row of b, which sholud be one
	for (int x = 0; x < width; ++x) {
		gpudata.b_h[x] = 1.0f;
	}

	// Fill in rows of A corresponding to first and last row of the image
	for (int x = 0; x < width; ++x) {
		for (int d = 0; d < 9; ++d) {
			gpudata.L_h.values(x, d) = (d == data.centralDiagonal ? 1.0f : 0.0f);
			gpudata.L_h.values(width*(height-1) + x, d) = (d == data.centralDiagonal ? 1.0f : 0.0f);
		}
	}

	// Fill in the rest of the matrix
	for (int y = 1; y < height-1; ++y) {
		for (int x = 0; x < width; ++x) {
			int idx = y * width + x;

			// Filter off out-of-bounds edges
			unsigned short filter = 495; // 111 101 111

			if (x == 0)        filter &= 203; // 011 001 011
			if (x == width-1)  filter &= 422; // 110 100 110
			if (y == 1)        filter &=  47; // 000 101 111
			if (y == height-2) filter &= 488; // 111 101 000

			float valueSum = 0.0f;

			for (int d = 0; d < 9; ++d) {
				gpudata.L_h.values(idx, d) = 0;

				float value = 0.0f;
				
				if (((256>>d) & filter) != 0) {
					value = _getWeight(data, x, y, d);
					gpudata.L_h.values(idx, d) = -value;
				} else if(y == 1 || y == height - 2) {
					unsigned short filter2 = 495; // 111 101 11
					if (x == 0)        filter2 &= 203; // 011 001 011
					if (x == width-1)  filter2 &= 422; // 110 100 110
					if (((256>>d) & filter2) != 0) {
						value = _getWeight(data, x, y, d);

						if (y == 1) gpudata.b_h[idx] += value;
					}
				}

				valueSum += value;
			}

			gpudata.L_h.values(idx, data.centralDiagonal) = valueSum;
		}
	}
}