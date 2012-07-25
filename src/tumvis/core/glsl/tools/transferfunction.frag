struct TFParameters {
    vec2 _intensityDomain;
};

/**
 * Linearly maps the image intensity \a intensity to the domain of the transfer function as defined in the TF parameters \a p.
 * \param	p			Transfer function parameters struct
 * \param	intensity	Image intensity (normalized to [0, 1])
 * \return	Intensity mapped to transfer function domain.
 */
float mapIntensityToTFDomain(in TFParameters p, in float intensity) {
    if(intensity <= p._intensityDomain.x)
       return 0.0;
    else if(intensity >= p._intensityDomain.y)
       return 1.0;
    else
        return (intensity - p._intensityDomain.x) / (p._intensityDomain.y - p._intensityDomain.x);
}

/**
 * Performs a 1D transfer function lookup for the given TF and intensity.
 * Before lookup \a intensity will be mapped to the TF domain.
 * \param	p			Transfer function parameters struct
 * \param	tex			Transfer function texture
 * \param	intensity	Image intensity (normalized to [0, 1])
 * \return	The color of the transfer function at the given intensity.
 */
vec4 lookupTF(in TFParameters p, in sampler1D tex, in float intensity) {
    intensity = mapIntensityToTFDomain(p, intensity);
    return texture1D(tex, intensity);
}

/**
 * Performs a 2D transfer function lookup for the given TF, intensity and y-value.
 * Before lookup \a intensity will be mapped to the TF domain.
 * \param	p			Transfer function parameters struct
 * \param	tex			Transfer function texture
 * \param	intensity	Image intensity (normalized to [0, 1])
 * \param	y			y-value for lookup
 * \return	The color of the transfer function at the given intensity and y-value.
 */
vec4 lookupTF(in TFParameters p, in sampler2D tex, in float intensity, in float y) {
    intensity = mapIntensityToTFDomain(p, intensity);
    return texture2D(tex, vec2(intensity, y));
}
