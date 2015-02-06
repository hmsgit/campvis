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

in vec3 geom_TexCoord;
noperspective in vec3 geom_EdgeDistance;

out vec4 out_Color;
out vec4 out_Picking;

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

uniform vec4 _wireframeColor;
uniform float _lineWidth = 1.0;
uniform float _transparency;

vec4 lookupTexture(vec3 worldPosition, sampler3D volume, TextureParameters3D volumeParams, sampler1D tf, TFParameters1D tfParams) {
    vec3 texCoord = worldToTexture(volumeParams, worldPosition).xyz;

    if (all(greaterThanEqual(texCoord, vec3(0.0))) && all(lessThan(texCoord, vec3(1.0)))) {
        vec4 color = texture(volume, texCoord);
        return lookupTF(tf, tfParams, color.r);
    }
}

void main() {
    vec4 color1 = lookupTexture(geom_TexCoord, _volume1, _volumeParams1, _transferFunction1, _transferFunctionParams1);
    vec4 color2 = lookupTexture(geom_TexCoord, _volume2, _volumeParams2, _transferFunction2, _transferFunctionParams2);
    vec4 color3 = lookupTexture(geom_TexCoord, _volume3, _volumeParams3, _transferFunction3, _transferFunctionParams3);

    out_Color = color1 + color2 + color3;
    if (out_Color.a > 1.0)
        out_Color /= out_Color.a;

    float maxElem = max(out_Color.x, max(out_Color.y, out_Color.z));
    if (maxElem > 1.0)
        out_Color.xyz /= maxElem;

    float alpha = max(out_Color.a, 1.0 - _transparency);
    out_Color = vec4(mix(vec3(0.0, 0.0, 0.0), out_Color.rgb, alpha), alpha);

#ifdef WIREFRAME_RENDERING
    // Find the smallest distance to the edges
    float d = min(geom_EdgeDistance.x, min(geom_EdgeDistance.y, geom_EdgeDistance.z)) * 2.0;

    // perform anti-aliasing
    float mixVal = smoothstep(_lineWidth - 1.0, _lineWidth + 1.0, d);

    // Mix the surface color with the line color
    out_Color = mix(vec4(1.0), out_Color, mixVal);
#endif

    // write world position to picking target
    out_Picking = vec4(geom_TexCoord, 1.0);
}
