// TODO: implement coordinate transformation using a trafo matrix?

struct Texture3D {
    // The texture
    sampler3D _texture;

    // Texture size
    vec3 _size;
    vec3 _sizeRCP;
};

/**
 * Texture lookup function for 3D textures using pixel coordinates, i.e [(0,0) , textureSize].
 * \param	tex			Texture3D struct with texture for lookup
 * \param	texCoords	Lookup coordinates in pixel coordinates
 * \return	The texel at the given coordinates.
 */
vec4 getElement3D(in Texture3D tex, in vec3 texCoords) {
    vec3 texCoordsNormalized = texCoords * tex._sizeRCP;
    //vec2 texCoordsTransformed = (texParams.matrix_ * vec4(texCoordsNormalized, 0.0, 1.0)).xy;
    return texture3D(tex._texture, texCoordsNormalized);
}

/**
 * Texture lookup function for 3D textures using normalized texture coordinates, i.e. [0,1].
 * \param	tex			Texture3D struct with texture for lookup
 * \param	texCoords	Lookup coordinates in normlized texture coordinates
 * \return	The texel at the given coordinates.
 */
vec4 getElement3DNormalized(in Texture3D tex, in vec3 texCoords) {
    //vec2 texCoordsTransformed = (texParams.matrix_ * vec4(texCoords, 0.0, 1.0)).xy;
    return texture3D(tex._texture, texCoords);
}
