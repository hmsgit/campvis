// ================================================================================================
// 
// This file is part of the CAMPVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

struct TransferFunction1D {
    sampler1D _texture;         ///< The texture containing the TF LUT
    vec2 _intensityDomain;      ///< The intensity domain to map the LUT to
};

struct TransferFunction2D {
    sampler2D _texture;         ///< The texture containing the TF LUT
    vec2 _intensityDomainX;     ///< The intensity domain for mapping the x value to the LUT
};

/**
 * Linearly maps the image intensity \a intensity to the domain of the transfer function as defined in the TF parameters \a p.
 * \param	intensityDomain     The intensity domain to map the intensity to
 * \param	intensity	        Image intensity (normalized to [0, 1])
 * \return	Intensity mapped to transfer function domain for LUT.
 */
float mapIntensityToTFDomain(in vec2 intensityDomain, in float intensity) {
    if(intensity <= intensityDomain.x)
       return -1.0;
    else if(intensity >= intensityDomain.y)
       return -1.0;
    else
        return (intensity - intensityDomain.x) / (intensityDomain.y - intensityDomain.x);
}

/**
 * Performs a 1D transfer function lookup for the given TF and intensity.
 * Before lookup \a intensity will be mapped to the TF domain.
 * \param	tf			1D Transfer function struct
 * \param	intensity	Image intensity (normalized to [0, 1])
 * \return	The color of the transfer function at the given intensity.
 */
vec4 lookupTF(in TransferFunction1D tf, in float intensity) {
    intensity = mapIntensityToTFDomain(tf._intensityDomain, intensity);
    return (intensity >= 0.0 ? texture(tf._texture, intensity) : vec4(0.0, 0.0, 0.0, 0.0));
}

/**
 * Performs a 2D transfer function lookup for the given TF, intensity and y-value.
 * Before lookup \a intensity will be mapped to the TF domain.
 * \param	tf		    2D Transfer function struct
 * \param	intensity	Image intensity (normalized to [0, 1])
 * \param	y			y-value for lookup
 * \return	The color of the transfer function at the given intensity and y-value.
 */
vec4 lookupTF(in TransferFunction2D tf, in float intensity, in float y) {
    intensity = mapIntensityToTFDomain(tf._intensityDomainX, intensity);
    return (intensity >= 0.0 ? texture(tf._texture, vec2(intensity, y)) : vec4(0.0, 0.0, 0.0, 0.0));
}
