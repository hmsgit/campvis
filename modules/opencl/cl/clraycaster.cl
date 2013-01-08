//#include "tools/raycasting.cl"
float4 jitterEntryPoint(float4 position, float4 direction, float stepSize) {
    float random;
    fract(sin((float)get_global_id(0) * 12.9898f + (float)get_global_id(1) * 78.233f) * 43758.5453f, &random);
    return position + direction * (stepSize * random);
}


__constant sampler_t smpNorm = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;
__constant float SAMPLING_BASE_INTERVAL_RCP = 200.0f;

/**
 * Makes a simple raycast through the volume for entry to exit point with minimal diffuse shading.
 */
float4 simpleRaycast(
    image3d_t volumeTex, 
    image2d_t tfData, 
    const float4 entryPoint, 
    const float4 exitPoint, 
    float* depth, 
    const float stepSize, 
    float tfLowerBound, 
    float tfUpperBound) {

    // result color
    float4 result = (float4)(0.0f, 0.0f, 0.0f, 0.0f);

    float t = 0.0f; //the current position on the ray with entryPoint as the origin
    float4 direction = exitPoint - entryPoint; //the direction of the ray
    direction.w = 0.0f;
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
__kernel void clraycaster( read_only image3d_t volumeTex,
                       read_only image2d_t tfData,
                       read_only image2d_t entryTexCol,
                       read_only image2d_t exitTexCol,
                       write_only image2d_t outCol,
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


__kernel void foobar( read_only image2d_t entryTexCol,
                       read_only image2d_t exitTexCol,
                       write_only image2d_t outCol)
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



