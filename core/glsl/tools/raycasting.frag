// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

/**
 * Code adapted from: https://www.marcusbannerman.co.uk/index.php/component/content/article/42-articles/97-vol-render-optimizations.htm
 */
void jitterEntryPoint(inout vec3 position, in vec3 direction, in float stepSize) {
    float random = fract(sin(gl_FragCoord.x * 12.9898 + gl_FragCoord.y * 78.233) * 43758.5453);
    position = position + direction * (stepSize * random);
}

/**
 * Converts a depth value in eye space to the corresponding depth value in viewport space.
 * \param   depth   Depth value in eye space.
 * \return  The given depth value transformed to viewport space.
 */   
float depthEyeToViewport(in float depth) {
    float f = gl_DepthRange.far;
    float n = gl_DepthRange.near;
    float diff = gl_DepthRange.diff;
    return (1.0/depth)*((f*n)/diff) + 0.5*((f+n)/diff) + 0.5;
}

/**
 * Converts a depth value in viewport space to the corresponding depth value in eye space.
 * \param   depth   Depth value in viewport space.
 * \return  The given depth value transformed to eye space.
 */   
float depthViewportToEye(in float depth) {
    float f = gl_DepthRange.far;
    float n = gl_DepthRange.near;
    float diff = gl_DepthRange.diff;
    return 1.0/((depth - 0.5 - 0.5*((f+n)/diff)) * (diff/(f*n)));
}

/**
 * Interpolates the depth \a ratio in viewport space between \a startDepth and \a endDepth.
 * \note    Mind the costs: Because the depth in viewport space does not scale linearly, the interpolation
 *          must be done in eye space. Hence, this method includes two coordinate transformations!
 * 
 * \param   ratio       Depth value to interpolate given as ratio of endDepth/startDepth.
 * \param   startDepth  Start depth value in viewport space.
 * \param   endDepth    End depth value in viewport space.
 * \return  The interpolated depth value.
 */     
float interpolateDepthViewport(in float ratio, in float startDepth, in float endDepth) {
    float startEye = depthViewportToEye(startDepth);
    float endEye = depthViewportToEye(endDepth);
    return depthEyeToViewport(startEye + ratio*(endEye - startEye));
}


// FIXME:   Because for some reason the above code does not work, I simply copy and pasted Voreens depth calculation.
//          This is not nice.

uniform float const_to_z_w_1;
uniform float const_to_z_w_2;
uniform float const_to_z_e_1;
uniform float const_to_z_e_2;


/**
 * Calculates the depth value for the current sample specified by the parameter t.
 **/
float calculateDepthValue(float t, float entryPointsDepth, float exitPointsDepth) {
   /*
    Converting eye coordinate depth values to windows coordinate depth values:
    (see http://www.opengl.org/resources/faq/technical/depthbuffer.htm 12.050, assuming w_e = 1)

    z_w = (1.0/z_e)*((f*n)/(f-n)) + 0.5*((f+n)/(f-n))+0.5; (f=far plane, n=near plane)

    We calculate constant terms outside:
    const_to_z_w_1 = ((f*n)/(f-n))
    const_to_z_w_2 = 0.5*((f+n)/(f-n))+0.5

    Converting windows coordinates to eye coordinates:

    z_e = 1.0/([z_w - 0.5 - 0.5*((f+n)/(f-n))]*((f-n)/(f*n)));

    with constant terms
    const_to_z_e_1 = 0.5 + 0.5*((f+n)/(f-n))
    const_to_z_e_2 = ((f-n)/(f*n))
   */

    // assign front value given in windows coordinates
    float zw_front = entryPointsDepth;
    // and convert it into eye coordinates
    float ze_front = 1.0/((zw_front - const_to_z_e_1)*const_to_z_e_2);

    // assign back value given in windows coordinates
    float zw_back = exitPointsDepth;
    // and convert it into eye coordinates
    float ze_back = 1.0/((zw_back - const_to_z_e_1)*const_to_z_e_2);

    // interpolate in eye coordinates
    float ze_current = ze_front + t*(ze_back-ze_front);

    // convert back to window coordinates
    float zw_current = (1.0/ze_current)*const_to_z_w_1 + const_to_z_w_2;

    return zw_current;
}