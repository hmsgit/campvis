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

struct TFParameters1D {
    vec2 _intensityDomain;      ///< The intensity domain to map the LUT to
};

struct TFParameters2D {
    vec2 _intensityDomainX;     ///< The intensity domain for mapping the x value to the LUT
};

/**
 * Linearly maps the image intensity \a intensity to the domain of the transfer function as defined in the TF parameters \a p.
 * \param	intensityDomain     The intensity domain to map the intensity to
 * \param	intensity	        Image intensity (normalized to [0, 1])
 * \return	Intensity mapped to transfer function domain for LUT.
 */
float mapIntensityToTFDomain(in vec2 intensityDomain, in float intensity) {
    if(intensity < intensityDomain.x)
       return -1.0;
    else if(intensity > intensityDomain.y)
       return -1.0;
    else
        return (intensity - intensityDomain.x) / (intensityDomain.y - intensityDomain.x);
}

/**
 * Performs a 1D transfer function lookup for the given TF and intensity.
 * Before lookup \a intensity will be mapped to the TF domain.
 * \param	tf			1D Transfer function sampler
 * \param   texParams   TF parameters struct
 * \param	intensity	Image intensity (normalized to [0, 1])
 * \return	The color of the transfer function at the given intensity.
 */
vec4 lookupTF(in sampler1D tf, in TFParameters1D texParams, in float intensity) {
    intensity = mapIntensityToTFDomain(texParams._intensityDomain, intensity);
    return (intensity >= 0.0 ? texture(tf, intensity) : vec4(0.0, 0.0, 0.0, 0.0));
}

/**
 * Performs a 2D transfer function lookup for the given TF, intensity and y-value.
 * Before lookup \a intensity will be mapped to the TF domain.
 * \param	tf			2D Transfer function sampler
 * \param   texParams   TF parameters struct
 * \param	intensity	Image intensity (normalized to [0, 1])
 * \param	y			y-value for lookup
 * \return	The color of the transfer function at the given intensity and y-value.
 */
vec4 lookupTF(in sampler2D tf, in TFParameters2D texParams, in float intensity, in float y) {
    intensity = mapIntensityToTFDomain(texParams._intensityDomainX, intensity);
    return (intensity >= 0.0 ? texture(tf, vec2(intensity, y)) : vec4(0.0, 0.0, 0.0, 0.0));
}
