// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

out vec4 out_Color;

#include "tools/texture2d.frag"

uniform vec2 _viewportSizeRCP;

uniform sampler2D _colorTexture;
uniform sampler2D _depthTexture;
uniform TextureParameters2D _texParams;

uniform sampler2D _depthPass2Texture;
uniform TextureParameters2D _pass2TexParams;

uniform vec2 _direction;
uniform float _sigma;
uniform float _lambda;
uniform float _minDepth;
uniform float _maxDepth;

uniform vec3 _coldColor;
uniform vec3 _warmColor;

int _halfKernelDimension;
float[25] _gaussKernel;
float _norm;


/**
 * Normalize the current input depth value to interval [_minDepth, _maxDepth].
 * \depth   depth value to be normalized
 */
float normalizeDepth(float depth) {
    return (depth - _minDepth) * (1.0 / (_maxDepth - _minDepth));
}


/**
 * Initializes the Gauss kernel and its norm.
 */
void initGaussKernel() {
    _halfKernelDimension = int(2.5 * _sigma);
    _norm = 0.0;

    // compute kernel
    for (int i=0; i<=_halfKernelDimension; i++) {
        _gaussKernel[i] = exp(-float(i*i)/(2.0*_sigma*_sigma));
        _norm += _gaussKernel[i];
    }
        
    // so far we have just computed norm for one half
    _norm = (_norm * 2.0) - _gaussKernel[0];
}

/**
 * Apply the Gauss filter in the given direction and return its result.
 * \param   texCoord    Texture coordinates for lookup
 */
float applyDepthGaussFilter(in vec2 texCoord) {
    float result = 0.0;
    for (int i = -_halfKernelDimension; i <= _halfKernelDimension; ++i) {
        // TODO: why the fuck does abs(i) not work here?!?
        int absi = (i < 0) ? -i : i;
        float curDepth = texture(_depthPass2Texture, texCoord + (_direction * _viewportSizeRCP * i)).r;
        result += curDepth * _gaussKernel[absi];
    }
    result /= _norm;
    return result;
}

void main() {
    vec2 texCoord = gl_FragCoord.xy * _viewportSizeRCP;

    // initialize and apply Gaussian filter
    initGaussKernel();
    float filteredDepth = applyDepthGaussFilter(texCoord);

    // separate two passes
    if (_direction.y != 1.0) {
        // only write out filtered depth value
        out_Color = vec4(1.0);
        gl_FragDepth = filteredDepth;
    } else {
        // we are in the second vertical pass and have to modulate the color
        float curDepth = texture(_depthTexture, texCoord).r;
        float deltaD = normalizeDepth(filteredDepth) - normalizeDepth(curDepth);

        // apply depth darkening
        vec4 curColor = texture(_colorTexture, texCoord);
        if (curColor.a == 0)
            discard;

#ifdef USE_COLORCODING
        float deltaDPlus = (deltaD > 0 ? deltaD : 0.0);
        float deltaDMinus = (deltaD < 0 ? -deltaD : 0.0);
        curColor.rgb += (_coldColor * deltaDMinus + _warmColor * deltaDPlus) * _lambda;
#else
        if (deltaD < 0.0) {
            curColor.rgb += deltaD * _lambda;
        }
#endif

        // write out modulated color and original depth value
        out_Color = curColor;
        gl_FragDepth = curDepth;
    }
}
