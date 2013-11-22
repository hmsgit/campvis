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
out vec4 out_Sums;
out vec4 out_Squares;
//out float out_Value;

#include "tools/texture3d.frag"

uniform sampler3D _referenceTexture;
uniform TextureParameters3D _referenceTextureParams;

uniform sampler3D _movingTexture;
uniform TextureParameters3D _movingTextureParams;

uniform mat4 _registrationInverse;

uniform bool _applyMask = false;
uniform vec2 _xClampRange = vec2(0.0, 1.0);
uniform vec2 _yClampRange = vec2(0.0, 1.0);
uniform vec2 _zClampRange = vec2(0.0, 1.0);

void main() {
    float sPixels = 0.0;
    float sFixed = 0.0;
    float sMoving = 0.0;
    float sFixedMoving = 0.0;
    float ssFixed = 0.0;
    float ssMoving = 0.0;
    float spMovingFixed = 0.0;
    float sRms = 0.0;

    if (ex_TexCoord.x >= _xClampRange.x && ex_TexCoord.x <= _xClampRange.y && ex_TexCoord.y >= _yClampRange.x && ex_TexCoord.y <= _yClampRange.y) {
        float zStart = min(_referenceTextureParams._sizeRCP.z / 2.0, _zClampRange.x);
        float zEnd = min(1.0, _zClampRange.y);

        for (float z = zStart; z < zEnd; z += _referenceTextureParams._sizeRCP.z) {
            // fetch value from reference volume
            vec3 referenceLookupTexCoord = vec3(ex_TexCoord.xy, z);
            float referenceValue = texture(_referenceTexture, referenceLookupTexCoord).a;

            // apply mask if requested
            if (!_applyMask || referenceValue > 0.0) {
                // compute moving lookup texture coordinates
                vec4 movingLookupTexCoord = _registrationInverse * vec4(referenceLookupTexCoord, 1.0);
                //movingLookupTexCoord.xyz /= movingLookupTexCoord.z;

                // fetch value from moving volume
                float movingValue = 0.0;
                if (all(greaterThanEqual(movingLookupTexCoord.xyz, vec3(0.0))) && all(lessThanEqual(movingLookupTexCoord.xyz, vec3(1.0)))) {
                   movingValue = texture(_movingTexture, movingLookupTexCoord.xyz).a;
                }

                float avg = (referenceValue + movingValue) / 2.0;

                // compute difference metrics
                sPixels += 1.0;
                sFixed += referenceValue;
                sMoving += movingValue;
                sFixedMoving += referenceValue + movingValue;
                ssFixed += referenceValue * referenceValue;
                ssMoving += movingValue * movingValue;
                spMovingFixed += movingValue * referenceValue;
                sRms += (referenceValue - avg) * (referenceValue - avg) + (movingValue - avg) * (movingValue - avg);
            }
        }
    }

    out_Sums = vec4(sPixels, sFixed, sMoving, sFixedMoving);
    out_Squares = vec4(ssFixed, ssMoving, spMovingFixed, sRms);
}
