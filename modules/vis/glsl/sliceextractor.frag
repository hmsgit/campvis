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

#version 330

in vec3 ex_TexCoord;
out vec4 out_Color;

//#include "tools/background.frag"
#include "tools/texture2d.frag"
#include "tools/transferfunction.frag"

uniform sampler2D _texture;
uniform TextureParameters2D _textureParams;

uniform sampler1D _transferFunction;
uniform TFParameters1D _transferFunctionParams;

void main() {
    vec4 texel = getElement2DNormalized(_texture, _textureParams, ex_TexCoord.xy);

    if (_textureParams._numChannels == 1) {
        out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.a);
    }
    else if (_textureParams._numChannels == 3) {
        out_Color = vec4(abs(texel.rgb), 1.0);
    }
    else if (_textureParams._numChannels == 4) {
        out_Color = (abs(texel) - vec4(_transferFunctionParams._intensityDomain.x)) / (_transferFunctionParams._intensityDomain.y - _transferFunctionParams._intensityDomain.x);
    }
    
    //if (out_Color.a == 0) {
    //    renderBackground(ex_TexCoord.xy, out_Color);
    //}
}
