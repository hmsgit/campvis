#include "tools/raycasting.frag"
#include "tools/texture2d.frag"
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"

uniform vec2 _viewportSizeRCP;
uniform bool _jitterEntryPoints;

uniform Texture2D _entryPoints;         // ray entry points
uniform Texture2D _entryPointsDepth;    // ray entry points depth
uniform Texture2D _exitPoints;          // ray exit points
uniform Texture2D _exitPointsDepth;     // ray exit points depth
uniform Texture3D _volume;              // texture lookup parameters for volume_

uniform sampler1D _tfTex;
uniform TFParameters _tfTextureParameters;

uniform float _samplingStepSize;

// TODO: copy+paste from Voreen - eliminate or improve.
const float SAMPLING_BASE_INTERVAL_RCP = 200.0;

/**
 * Computes the DRR by simple raycasting and returns the final fragment color.
 */
vec4 performRaycasting(in vec3 entryPoint, in vec3 exitPoint, in vec2 texCoords) {
    vec4 result = vec4(0.0);
    float depthT = -1.0;

    // calculate ray parameters
    vec3 direction = exitPoint.rgb - entryPoint.rgb;
    float t = 0.0;
    float tend = length(direction);
    direction = normalize(direction);

    if (_jitterEntryPoints)
        jitterEntryPoint(entryPoint, direction, _samplingStepSize);

    while (t < tend) {
        vec3 samplePosition = entryPoint.rgb + t * direction;
        float intensity = getElement3DNormalized(_volume, samplePosition).a;
        vec4 color = lookupTF(_tfTextureParameters, _tfTex, intensity);

        // perform compositing
        if (color.a > 0.0) {
            // accomodate for variable sampling rates (base interval defined by mod_compositing.frag)
            color.a = 1.0 - pow(1.0 - color.a, _samplingStepSize * SAMPLING_BASE_INTERVAL_RCP);
            result.rgb = result.rgb + (1.0 - result.a) * color.a * color.rgb;
            result.a = result.a + (1.0 -result.a) * color.a;
        }

        // save first hit ray parameter for depth value calculation
        if (depthT < 0.0 && result.a > 0.0)
            depthT = t;

        // early ray termination
        if (result.a >= 1.0) {
            result.a = 1.0;
            t = tend;
        }

        t += _samplingStepSize;
    }

    // calculate depth value from ray parameter
    gl_FragDepth = 1.0;
    //if (depthT >= 0.0) {
    //    float depthEntry = getElement2DNormalized(_entryPointsDepth, texCoords).z;
    //    float depthExit = getElement2DNormalized(_exitPointsDepth, texCoords).z;
    //    gl_FragDepth = calculateDepthValue(depthT/tend, depthEntry, depthExit);
    //}

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
        gl_FragData[0] = performRaycasting(frontPos, backPos, p);
    }
}
