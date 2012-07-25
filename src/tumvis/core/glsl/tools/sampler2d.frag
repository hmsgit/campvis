// TODO: implement coordinate transformation using a trafo matrix?

struct TextureParameters {
    vec2 _size;
    vec2 _sizeRCP;
};

/**
 * Texture lookup function for 2D textures using pixel coordinates, i.e [(0,0) , textureSize].
 * \param	tex			2D texture for lookup
 * \param	texParams	Corresponding texture parameters struct
 * \param	texCoords	Lookup coordinates in pixel coordinates
 * \return	The texel at the given coordinates.
 */
vec4 getElement2D(in sampler2D tex, in TextureParameters texParams, in vec2 texCoords) {
    vec2 texCoordsNormalized = texCoords * texParams._sizeRCP;
    //vec2 texCoordsTransformed = (texParams.matrix_ * vec4(texCoordsNormalized, 0.0, 1.0)).xy;
    return texture2D(tex, texCoordsNormalized);
}

/**
 * Texture lookup function for 2D textures using normalized texture coordinates, i.e. [0,1].
 * \param	tex			2D texture for lookup
 * \param	texParams	Corresponding texture parameters struct
 * \param	texCoords	Lookup coordinates in normlized texture coordinates
 * \return	The texel at the given coordinates.
 */
vec4 getElement2DNormalized(in sampler2D tex, in TextureParameters texParams, in vec2 texCoords) {
    //vec2 texCoordsTransformed = (texParams.matrix_ * vec4(texCoords, 0.0, 1.0)).xy;
    return texture2D(tex, texCoords);
}
