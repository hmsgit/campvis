
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
 * Computes the ambient term according to the given material color \a ka and ambient intensity \a ia.
 *
 * \param   ka  Material ambient color
 * \param   ia  Ambient light intensity
 */
vec3 getAmbientTerm(in vec3 ka, in vec3 ia) {
    return ka * ia;
}


/**
 * Computes the diffuse term according to the given parameters.
 *
 * \param   kd  Material diffuse color
 * \param   id  Diffuse light intensity
 * \param   N   Surface normal
 * \param   L   Normalized light vector
 */
vec3 getDiffuseTerm(in vec3 kd, in vec3 id, in vec3 N, in vec3 L) {
    float NdotL = max(dot(N, L), 0.0);
    return kd * id * NdotL;
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
vec3 getSpecularTerm(in vec3 ks, in vec3 is, in vec3 N, in vec3 L, in vec3 V, in float shininess) {
    vec3 H = normalize(V + L);
    float NdotH = pow(max(dot(N, H), 0.0), shininess);
    return ks * is * NdotH;
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

    vec3 toReturn = getAmbientTerm(ka, light._ambientColor);
    toReturn += getDiffuseTerm(kd, light._diffuseColor, N, L);
    toReturn += getSpecularTerm(ks, light._specularColor, N, L, V, light._shininess);
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
    vec3 N = normalize(normal);
    vec3 V = normalize(camera - position);

    // get light source distance for attenuation and normalize light vector
    vec3 L = light._position - position;
    float d = length(L);
    L /= d;

    vec3 toReturn = getAmbientTerm(materialColor, light._ambientColor);
    toReturn += getDiffuseTerm(materialColor, light._diffuseColor, N, L);
    toReturn += getSpecularTerm(materialColor, light._specularColor, N, L, V, light._shininess);
    #ifdef PHONG_APPLY_ATTENUATION
        toReturn *= computeAttenuation(light._attenuation, d);
    #endif
    return toReturn;
}
