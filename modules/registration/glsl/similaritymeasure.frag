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

in vec3 ex_TexCoord;
out vec4 out_Color;
//out float out_Value;

#include "tools/texture3d.frag"

uniform sampler3D _referenceTexture;
uniform TextureParameters3D _referenceTextureParams;

uniform sampler3D _movingTexture;
uniform TextureParameters3D _movingTextureParams;

//uniform mat4 _registrationMatrix;
uniform mat4 _registrationInverse;

void main() {
    ivec2 texel = ivec2(ex_TexCoord.xy * _referenceTextureParams._size.xy);
    float depth = _referenceTextureParams._size.z;
    float sad = 0.0;
    float ssd = 0.0;

    for (float z = _referenceTextureParams._sizeRCP.z / 2.0; z < 1.0; z += _referenceTextureParams._sizeRCP.z) {
        vec3 referenceLookupTexCoord = vec3(ex_TexCoord.xy, z);
        vec4 movingLookupTexCoord = _registrationInverse * vec4(referenceLookupTexCoord, 1.0);
        //movingLookupTexCoord.xyz /= movingLookupTexCoord.z;

//        if (all(greaterThanEqual(movingLookupTexCoord.xyz, vec3(0.0))) && all(lessThanEqual(movingLookupTexCoord.xyz, vec3(1.0)))) {
            float referenceValue = texture(_referenceTexture, referenceLookupTexCoord).a;
            float movingValue = texture(_movingTexture, movingLookupTexCoord.xyz).a;

            float difference = referenceValue - movingValue;
            sad += abs(difference);
            ssd += difference * difference;
//        }
    }

    //out_Value = toReturn;
    out_Color = vec4(ssd, sad, 0.0, 1.0);
}
