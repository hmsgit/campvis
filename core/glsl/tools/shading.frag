// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================


/// This struct contains all information about a light source.
struct LightSource {
    vec3 _position;        ///< light position in world space
    vec3 _ambientColor;    ///< ambient color (r,g,b)
    vec3 _diffuseColor;    ///< diffuse color (r,g,b)
    vec3 _specularColor;   ///< specular color (r,g,b)
    vec3 _attenuation;     ///< attenuation (constant, linear, quadratic)
    float _shininess;      ///< Shininess (usually a per-material parameter, but we put it here for convenience)
};

// uniforms needed for shading
uniform float shininess_;       // material shininess parameter
uniform LightSource lightSource_;

/**
 * Returns attenuation factor based on the given parameters.
 *
 * \param   attenuation Attenuation parameter vector (constant, linear, quadratic)
 * \param   d           Distance to the light source.
 */
float computeAttenuation(in vec3 attenuation, in float d) {
    float att = 1.0 / (attenuation.x + (d * attenuation.y) + (d * d * attenuation.z));
    return min(att, 1.0);
}

/**
 * Computes the diffuse term according to the given parameters.
 *
 * \param   id  Diffuse light intensity
 * \param   N   Surface normal
 * \param   L   Normalized light vector
 */
vec3 getDiffuseTerm(in vec3 id, in vec3 N, in vec3 L) {
    float NdotL = max(dot(N, L), 0.0);
    return id * NdotL;
}


/**
 * Computes the specular term according to the given parameters.
 *
 * \param   ks          Material specular color
 * \param   is          Specular light intensity
 * \param   N           Surface normal
 * \param   L           Normalized light vector
 * \param   V           View vector
 * \param   shininess   Shininess coefficient
 */
vec3 getSpecularTerm(in vec3 is, in vec3 N, in vec3 L, in vec3 V, in float shininess) {
    vec3 H = normalize(V + L);
    float NdotH = pow(max(dot(N, H), 0.0), shininess);
    return is * NdotH;
}

/**
 * Computes the phong shading according to the given parameters.
 *
 * \param   position    sample position
 * \param   light       LightSource
 * \param   camera      Camera position
 * \param   normal      Normal
 * \param   ka          Material ambient color
 * \param   kd          Material diffuse color
 * \param   ks          Material specular color
 */
vec3 calculatePhongShading(in vec3 position, in LightSource light, in vec3 camera, in vec3 normal, in vec3 ka, in vec3 kd, in vec3 ks) {
    vec3 N = normalize(normal);
    vec3 V = normalize(camera - position);

    // get light source distance for attenuation and normalize light vector
    vec3 L = light._position - position;
    float d = length(L);
    L /= d;

    vec3 toReturn = ka * light._ambientColor;   // ambient term
    toReturn += kd * getDiffuseTerm(light._diffuseColor, N, L);
    toReturn += ks * getSpecularTerm(light._specularColor, N, L, V, light._shininess);
    #ifdef PHONG_APPLY_ATTENUATION
        toReturn *= computeAttenuation(light._attenuation, d);
    #endif
    return toReturn;
}

/**
 * Computes the phong shading according to the given parameters.
 * 
 * \param   position        sample position
 * \param   light           LightSource
 * \param   camera          Camera position
 * \param   normal          Normal
 * \param   materialColor   Material color (used for all shading coefficients)
 */
vec3 calculatePhongShading(in vec3 position, in vec3 ambientColorOverride, LightSource light, in vec3 camera, in vec3 normal, in vec3 materialColor) {
    vec3 N = normalize(normal);
    vec3 V = normalize(camera - position);

    // get light source distance for attenuation and normalize light vector
    vec3 L = light._position - position;
    float d = length(L);
    L /= d;

    vec3 toReturn = materialColor * ambientColorOverride;   // ambient term
    toReturn += materialColor * getDiffuseTerm(light._diffuseColor, N, L);
    toReturn += materialColor * getSpecularTerm(light._specularColor, N, L, V, light._shininess);
    #ifdef PHONG_APPLY_ATTENUATION
        toReturn *= computeAttenuation(light._attenuation, d);
    #endif
    return toReturn;
}

/**
 * Computes the phong shading according to the given parameters.
 * 
 * \param   position        sample position
 * \param   light           LightSource
 * \param   camera          Camera position
 * \param   normal          Normal
 * \param   materialColor   Material color (used for all shading coefficients)
 */
vec3 calculatePhongShading(in vec3 position, in LightSource light, in vec3 camera, in vec3 normal, in vec3 materialColor) {
    return calculatePhongShading(position, light._ambientColor, light, camera, normal, materialColor);
}


/**
 * Computes the phong shading intensity according to the given parameters.
 * 
 * \param   position        sample position
 * \param   light           LightSource
 * \param   camera          Camera position
 * \param   normal          Normal
 */
float getPhongShadingIntensity(in vec3 position, in LightSource light, in vec3 camera, in vec3 normal) {
    vec3 N = normalize(normal);
    vec3 V = normalize(camera - position);

    // get light source distance for attenuation and normalize light vector
    vec3 L = light._position - position;
    float d = length(L);
    L /= d;

    vec3 toReturn = light._ambientColor;   // ambient term
    toReturn += getDiffuseTerm(light._diffuseColor, N, L);
    toReturn += getSpecularTerm(light._specularColor, N, L, V, light._shininess);
    #ifdef PHONG_APPLY_ATTENUATION
        toReturn *= computeAttenuation(light._attenuation, d);
    #endif
    return (toReturn.x + toReturn.y + toReturn.z) / 3.0;
}