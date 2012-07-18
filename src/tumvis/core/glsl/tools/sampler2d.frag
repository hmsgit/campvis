struct TextureParameters {
    vec2 _size;
    vec2 _sizeRCP;
};

// Texture lookup function for 2D textures,
// expecting texture coordinates as pixel coordinates, i.e, [(0,0) , textureSize].
vec4 getElement2D(in sampler2D myTexture, in TextureParameters texParams, in vec2 texCoords) {
    vec2 texCoordsNormalized = texCoords * texParams._sizeRCP;
    //vec2 texCoordsTransformed = (texParams.matrix_ * vec4(texCoordsNormalized, 0.0, 1.0)).xy;
    return texture2D(myTexture, texCoordsNormalized);
}

// Texture lookup function for 2D textures,
// expecting normalized texture coordinates, i.e., [0,1].
vec4 getElement2DNormalized(in sampler2D myTexture, in TextureParameters texParams, in vec2 texCoords) {
    //vec2 texCoordsTransformed = (texParams.matrix_ * vec4(texCoords, 0.0, 1.0)).xy;
    return texture2D(myTexture, texCoords);
}
