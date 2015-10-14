// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

in vec3 ex_TexCoord;
out vec4 out_Color;

#include "tools/texture3d.frag"

uniform sampler3D _referenceTexture;
uniform TextureParameters3D _referenceTextureParams;

uniform sampler3D _movingTexture;
uniform TextureParameters3D _movingTextureParams;

uniform float _zTex;
uniform mat4 _registrationInverse;

uniform bool _applyMask;
uniform vec2 _xClampRange;
uniform vec2 _yClampRange;
uniform vec2 _zClampRange;

void main() {
    float result = 0.0;
    float difference = 0.0;
    float sad = 0.0;

    if (   ex_TexCoord.x >= _xClampRange.x && ex_TexCoord.x <= _xClampRange.y 
        && ex_TexCoord.y >= _yClampRange.x && ex_TexCoord.y <= _yClampRange.y
        && _zTex >= _zClampRange.x         && _zTex <= _zClampRange.y) {
        // compute lookup coordinates
        vec3 referenceLookupTexCoord = vec3(ex_TexCoord.xy, _zTex);
        vec4 movingLookupTexCoord = _registrationInverse * vec4(referenceLookupTexCoord, 1.0);

        // fetch texels
        float referenceValue = texture(_referenceTexture, referenceLookupTexCoord).r;
        float movingValue = 0.0;
        
        if (!_applyMask || referenceValue > 0.0)
            movingValue = texture(_movingTexture, movingLookupTexCoord.xyz).r;

        // compute differences
        difference = referenceValue - movingValue;
        sad = abs(difference);
        //float ssd = difference * difference;
    }
    
    // write output color
    result = difference;
    out_Color = vec4(result, result, result, result);
}
