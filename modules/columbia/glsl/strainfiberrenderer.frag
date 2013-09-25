// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#include "tools/shading.frag"
#include "tools/texture3d.frag"

in vec3 geom_Normal;           ///< incoming texture coordinate
in vec4 geom_Position;
in vec4 geom_Color;
in float geom_SineFlag;

out vec4 out_Color;         ///< outgoing fragment color


uniform vec4 _color;
uniform LightSource _lightSource;
uniform vec3 _cameraPosition;


const float PI = 3.1415926535897932384626433832795;


/**
 * Calculate Phong terms for simple OpenGL line primitives according to
 * the work of Zöckler, Stalling and Hege in "Interactive Visualization Of
 * 3D-Vector Fields Using Illuminated Stream Lines", from 1996.
 */
vec3 phongShadingForLines(vec3 tangent, vec3 view, vec3 light, float shininess) {
    // normalize the vectors again which are interpolated per pixel and are
    // therefore different for each fragment when transfered from vertex to
    // fragment shader!
    // light does not need to be re-normalized, as it remains the same for
    // all fragments.
    //
    vec3 t = normalize(tangent);  // normalize again as the normal is interpolated per pixel!
    vec3 v = normalize(view);     // the same for the view vector: it is interpolated per pixel!
    float LdotT = clamp(dot(light, t), -1.0, 1.0);
    float NdotL = max(sqrt(1.0 - (LdotT * LdotT)), 0.0);

    float VdotT = clamp(dot(v, t), -1.0, 1.0);
    float VdotN = max(sqrt(1.0 - (VdotT * VdotT)), 0.0);
    float RdotV = (LdotT * VdotT) - (NdotL * VdotN);
    float specular = max(pow(RdotV, shininess), 0.0);
    //const float p = 2.0;
    const float p = 4.8;
    return vec3(1.0, pow(NdotL, p), specular);
}

vec3 phongShadingForFaces(vec3 normal, vec3 view, vec3 light, float shininess) {
    vec3 n = normalize(normal);
    vec3 v = normalize(view);
    vec3 l = normalize(light);
    const float p = 4.8;
    
    float NdotL = max(dot(n, l), 0.0);
    vec3 halfVector = normalize(v+l);
    float NdotH = max(dot(n, halfVector), 0.0);

    return vec3(1.0, pow(NdotL, p), pow(NdotH, shininess));
}

/**
 * Applies tube-like texture to triangle strip
 */
vec4 applyTubeTexture(vec3 color) {
    float val = max(sin(geom_SineFlag * PI), .4f);
    return vec4(color*(val + pow(val, 16.f)), geom_Color.a);
}


void main() {
    vec4 color;
#ifdef DO_STRIPES
    color = applyTubeTexture(geom_Color.rbg);//vec4(geom_Normal, 1.0);
#endif
#ifdef DO_TUBES
    color = geom_Color;
#endif

    out_Color = color;

#ifdef ENABLE_SHADING
    // compute tangent (needed for shading and normals)
    //vec3 tangent = geom_Normal.rgb;
    //vec3 phongTerms = phongShadingForLines(tangent, (geom_Position.xyz / geom_Position.z) - _cameraPosition, _lightSource._position, _lightSource._shininess);
    //out_Color.rbg = (color.rbg * _lightSource._ambientColor) + (color.rbg * _lightSource._diffuseColor * phongTerms.y) + (color.rbg * _lightSource._specularColor * phongTerms.z);

    out_Color.rgb = calculatePhongShading(geom_Position.xyz / geom_Position.z, _lightSource, _cameraPosition, geom_Normal, color.rgb, color.rgb, vec3(1.0, 1.0, 1.0));
#endif

    gl_FragDepth = gl_FragCoord.z;
}
