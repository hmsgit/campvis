
__constant sampler_t gradSmp = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;

float4 calcGradient(float4 sample, image3d_t volumeTex) {

    float4 offset = (float4)(1.f) / convert_float4(get_image_dim(volumeTex));
    float4 gradient;

    float v0  = read_imagef(volumeTex, gradSmp, sample - (float4)(offset.x, 0.0, 0.0, 0.0)).x;
    float v1  = read_imagef(volumeTex, gradSmp, sample - (float4)(0.0, offset.y, 0.0, 0.0)).x;
    float v2  = read_imagef(volumeTex, gradSmp, sample - (float4)(0.0, 0.0, offset.z, 0.0)).x;
    float v3  = read_imagef(volumeTex, gradSmp, sample + (float4)(offset.x, 0.0, 0.0, 0.0)).x;
    float v4  = read_imagef(volumeTex, gradSmp, sample + (float4)(0.0, offset.y, 0.0, 0.0)).x;
    float v5  = read_imagef(volumeTex, gradSmp, sample + (float4)(0.0, 0.0, offset.z, 0.0)).x;
    gradient.x = v0 - v3;
    gradient.y = v1 - v4;
    gradient.z = v2 - v5;
    gradient.w = 0.0;

    return gradient;
}

float4 calcFilteredGradient(float4 sample, image3d_t volumeTex) {
    float4 offset = (float4)(1.f) / convert_float4(get_image_dim(volumeTex));

    float4 g0 = calcGradient(sample, volumeTex);
    float4 g1 = calcGradient(sample + (float4)(-offset.x, -offset.y, -offset.z, 0.0), volumeTex);
    float4 g2 = calcGradient(sample + (float4)( offset.x,  offset.y,  offset.z, 0.0), volumeTex);
    float4 g3 = calcGradient(sample + (float4)(-offset.x,  offset.y, -offset.z, 0.0), volumeTex);
    float4 g4 = calcGradient(sample + (float4)( offset.x, -offset.y,  offset.z, 0.0), volumeTex);
    float4 g5 = calcGradient(sample + (float4)(-offset.x, -offset.y,  offset.z, 0.0), volumeTex);
    float4 g6 = calcGradient(sample + (float4)( offset.x,  offset.y, -offset.z, 0.0), volumeTex);
    float4 g7 = calcGradient(sample + (float4)(-offset.x,  offset.y,  offset.z, 0.0), volumeTex);
    float4 g8 = calcGradient(sample + (float4)( offset.x, -offset.y, -offset.z, 0.0), volumeTex);

    float4 mix0 = mix(mix(g1, g2, 0.5f), mix(g3, g4, 0.5f), 0.5f);
    float4 mix1 = mix(mix(g5, g6, 0.5f), mix(g7, g8, 0.5f), 0.5f);
    return mix(g0, mix(mix0, mix1, 0.5f), 0.75f);
}


__constant sampler_t smpNorm = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;
__constant float SAMPLING_BASE_INTERVAL_RCP = 200.0;

/**
 * Makes a simple raycast through the volume for entry to exit point with minimal diffuse shading.
 */
float4 simpleRaycast(__global read_only image3d_t volumeTex, __global read_only image2d_t tfData, const float4 entryPoint, const float4 exitPoint, float* depth, const float stepSize, float tfLowerBound, float tfUpperBound) {

    // result color
    float4 result = (float4)(0.0, 0.0, 0.0, 0.0);

    float t = 0.0; //the current position on the ray with entryPoint as the origin
    float4 direction = exitPoint - entryPoint; //the direction of the ray
    direction.w = 0.0;
    float tend = fast_length(direction); //the length of the ray

    direction = fast_normalize(direction);

    while(t <= tend) {

        //calculate sample position and get corresponding voxel
        float4 sample = entryPoint + t * direction;
        float intensity = read_imagef(volumeTex, smpNorm, sample).w;

        // apply tf intensity domain mapping:
        intensity = (intensity - tfLowerBound) / (tfUpperBound - tfLowerBound);

        float4 color = read_imagef(tfData, smpNorm, (float2)(intensity, 0.0));

        // apply opacity correction to accomodate for variable sampling intervals
        color.w = 1.0 - pow(1.0 - color.w, stepSize * SAMPLING_BASE_INTERVAL_RCP);

        // Add a little shading.  calcGradient is declared in mod_gradients.cl
        //float4 norm = normalize(calcGradient(sample, volumeTex));
        //color *= fabs(dot(norm, direction));

        //calculate ray integral
        result.xyz = result.xyz + (1.0 - result.w) * color.w * color.xyz;
        result.w = result.w + (1.0 - result.w) * color.w;

        // early ray termination
        if(result.w > 0.95)
            break;

        //raise position on ray
        t += stepSize;
    }

    // TODO: calculate correct depth value
    if(t >= 0.0)
        *depth = t / tend;
    else
        *depth = 1.0;

    return result;
}

//main for raycasting. This function is called for every pixel in view.
// TODO: Depth values are currently not read or written as OpenCL does not support OpenGL GL_DEPTH_COMPONENT image formats.
__kernel void clraycaster(__global read_only image3d_t volumeTex,
                      __global read_only image2d_t tfData,
                      __global read_only image2d_t entryTexCol,
                      __global read_only image2d_t exitTexCol,
                      __global write_only image2d_t outCol,
                      float stepSize,
                      float tfLowerBound,
                      float tfUpperBound
                      )
{
    //output image pixel coordinates
    int2 target = (int2)(get_global_id(0), get_global_id(1));
    // Need to add 0.5 in order to get the correct coordinate. We could also use the integer coordinate directly...
    float2 targetNorm = (convert_float2(target) + (float2)(0.5)) / convert_float2((int2)(get_global_size(0), get_global_size(1)));

    float4 color;
    float depth = 1.0;

    float4 entry = read_imagef(entryTexCol, smpNorm, targetNorm);
    float4 exit  = read_imagef(exitTexCol,  smpNorm, targetNorm);

    if( entry.x != exit.x || entry.y != exit.y || entry.z != exit.z )
        color = simpleRaycast(volumeTex, tfData, entry, exit, &depth, stepSize, tfLowerBound, tfUpperBound);
    else
        color = (float4)(0.0);

    write_imagef(outCol, target, color);
    //write_imagef(outDepth, target, (float4)(depth));
}


__kernel void foobar(__global read_only image2d_t entryTexCol,
                      __global read_only image2d_t exitTexCol,
                      __global write_only image2d_t outCol)
{
    //output image pixel coordinates
    int2 target = (int2)(get_global_id(0), get_global_id(1));
    // Need to add 0.5 in order to get the correct coordinate. We could also use the integer coordinate directly...
    float2 targetNorm = (convert_float2(target) + (float2)(0.5)) / convert_float2((int2)(get_global_size(0), get_global_size(1)));
    float4 color;

    float4 entry = read_imagef(entryTexCol, smpNorm, targetNorm);
    float4 exit  = read_imagef(exitTexCol,  smpNorm, targetNorm);

    if( entry.x != exit.x || entry.y != exit.y || entry.z != exit.z )
        color = exit - entry;
    else
        color = (float4)(0.0);

    write_imagef(outCol, target, color);
}



