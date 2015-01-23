// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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
#include "tools/transferfunction.frag"

// Input volume2
uniform sampler3D _volume1;
uniform sampler3D _volume2;
uniform sampler3D _volume3;
uniform TextureParameters3D _volumeParams1;
uniform TextureParameters3D _volumeParams2;
uniform TextureParameters3D _volumeParams3;

// Transfer function
uniform sampler1D _transferFunction1;
uniform sampler1D _transferFunction2;
uniform sampler1D _transferFunction3;
uniform TFParameters1D _transferFunctionParams1;
uniform TFParameters1D _transferFunctionParams2;
uniform TFParameters1D _transferFunctionParams3;


vec4 lookupTexture(vec3 worldPosition, sampler3D volume, TextureParameters3D volumeParams, sampler1D tf, TFParameters1D tfParams) {
    vec3 texCoord = worldToTexture(volumeParams, worldPosition).xyz;

    if (all(greaterThanEqual(texCoord, vec3(0.0))) && all(lessThan(texCoord, vec3(1.0)))) {
        vec4 color = texture(volume, texCoord);
        return lookupTF(tf, tfParams, color.r);
    }
}

void main() {
    vec4 color1 = lookupTexture(ex_TexCoord, _volume1, _volumeParams1, _transferFunction1, _transferFunctionParams1);
    vec4 color2 = lookupTexture(ex_TexCoord, _volume2, _volumeParams2, _transferFunction2, _transferFunctionParams2);
    vec4 color3 = lookupTexture(ex_TexCoord, _volume3, _volumeParams3, _transferFunction3, _transferFunctionParams3);

    out_Color = color1 + color2 + color3;
    if (out_Color.w > 1.0)
        out_Color /= out_Color.w;

    out_Color = vec4(mix(out_Color.rgb, vec3(0.0, 0.0, 0.0), out_Color.a), 1.0);
}
