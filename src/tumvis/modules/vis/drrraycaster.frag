#include "tools/raycasting.frag"
#include "tools/texture2d.frag"
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"

uniform vec2 _viewportSizeRCP;
uniform bool _jitterEntryPoints;

uniform Texture2D _entryPoints;          // ray entry points
uniform Texture2D _exitPoints;           // ray exit points
uniform Texture3D _volume;            // texture lookup parameters for volume_

uniform sampler1D _tfTex;
uniform TFParameters _tfTextureParameters;

uniform float _samplingStepSize;
uniform float _shift;
uniform float _scale;

// TODO: copy+paste from Voreen - eliminate or improve.
const float SAMPLING_BASE_INTERVAL_RCP = 200.0;

/**
 * Computes the DRR by simple raycasting and returns the final fragment color.
 */
vec4 raycastDRR(in vec3 entryPoint, in vec3 exitPoint) {
    vec4 result = vec4(0.0);

    // calculate ray parameters
    vec3 direction = exitPoint.rgb - entryPoint.rgb;
    float t = 0.0;
    float tend = length(direction);
    direction = normalize(direction);

    if (_jitterEntryPoints)
        jitterEntryPoint(entryPoint, direction, _samplingStepSize);

    while (t < tend) {
        vec3 samplePosition = entryPoint.rgb + t * direction;
        vec4 voxel = getElement3DNormalized(_volume, samplePosition);
        vec4 color = lookupTF(_tfTextureParameters, _tfTex, voxel.a);

#ifdef DEPTH_MAPPING
        // use Bernstein Polynomials for depth-color mapping
        float depth = t / tend;
            
        float b02 = (1-depth) * (1-depth);
        float b12 = 2 * depth * (1-depth);
        float b22 = depth * depth;
            
        color = vec4(0.75*b02 + 0.25*b12, b12, 0.25*b12 + 0.75*b22, 1.0 - pow(1.0 - color.a, _samplingStepSize * SAMPLING_BASE_INTERVAL_RCP));
/*            float t_sq = depth * depth;
        float inv_t_sq = (1-depth) * (1-depth);
        float b04 = inv_t_sq * inv_t_sq;
        float b24 = 6 * t_sq * inv_t_sq;
        float b44 = t_sq * t_sq;
        color = vec4(0.5*b04 + 0.5*b24, b24, 0.5*b24 + 0.5*b44, 1.0 - pow(1.0 - color.a, samplingStepSize_ * SAMPLING_BASE_INTERVAL_RCP));*/

        result.rgb = result.rgb + (1.0 - result.a) * color.a * color.rgb;
        result.a = result.a + color.a * _scale;
#else
        // perform compositing (account for differen step size / sampling rate)
        color.a = 1.0 - pow(1.0 - color.a, _samplingStepSize * SAMPLING_BASE_INTERVAL_RCP);
        result.a = result.a + color.a * _scale;
#endif       

        t += _samplingStepSize;
    }

    // DRR images don't have a meaningful depth value
    gl_FragDepth = 1.0;

#ifdef DEPTH_MAPPING
    result.rgb = result.rgb * vec3(exp(-result.a + _shift));
    //result.rgb = exp(-result.rgb * result.a + shift_);
#else
#ifdef DRR_INVERT
    // apply e-function, as we want the "negative" X-Ray image, we invert the intensity result
    result.rgb = 1 - vec3(exp(-result.a + _shift));
#else
    // apply e-function, the normal way
    result.rgb = vec3(exp(-result.a + _shift));
#endif
#endif
    result.a = 1.0;
    return result;
}

/***
 * The main method.
 ***/
void main() {
    vec2 p = gl_FragCoord.xy * _viewportSizeRCP;
    vec3 frontPos = getElement2DNormalized(_entryPoints, p).rgb;
    vec3 backPos = getElement2DNormalized(_exitPoints, p).rgb;

    //determine whether the ray has to be casted
    if (frontPos == backPos) {
        //background need no raycasting
        discard;
    } else {
        //fragCoords are lying inside the boundingbox
        gl_FragData[0] = raycastDRR(frontPos, backPos);
    }
}
