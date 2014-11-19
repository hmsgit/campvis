
/*<

__global__ void k_buildCMSystem(cudaTextureObject_t image, int width, int height,
                                float alpha, float beta, float gamma,
                                float *out_weights)
{
	int x = blockIdx.x * blockDim.x + threadIdx.x;
	int y = blockIdx.y * blockDim.y + threadIdx.y;
	int idx = y * width + x;

	if (x >= width || y >= height) return;

	float attenuation_center = exp(-alpha * (float)y / height);
	float attenuation_bottom = exp(-alpha * (float)(y+1) / height);

	float c_center = tex2D(image, x, y)     * attenuation_center;
	float c_right  = tex2D(image, x+1, y)   * attenuation_center;
	float c_bottom = tex2D(image, x, y+1)   * attenuation_bottom;
	float c_se     = tex2D(image, x+1, y+1) * attenuation_bottom;
	float c_sw     = tex2D(image, x-1, y+1) * attenuation_bottom;

	float w_right  = exp(-beta * (abs(c_center - c_right) + gamma));
	float w_bottom = exp(-beta * (abs(c_center - c_bottom)));
	float w_se     = exp(-beta * (abs(c_center - c_se) + sqrt(2.0f) * gamma));
	float w_sw     = exp(-beta * (abs(c_center - c_sw) + sqrt(2.0f) * gamma));

	out_weights[idx*4 + 0] = w_right;
	out_weights[idx*4 + 1] = w_sw;
	out_weights[idx*4 + 2] = w_bottom;
	out_weights[idx*4 + 3] = w_se;
}


int main()
{
	// Create texture object
	cudaResourceDesc resDesc;
	memset(&resDesc, 0, sizeof(resDesc));
	resDesc.resType = cudaResourceTypePitch2D


	return 0;
}*/