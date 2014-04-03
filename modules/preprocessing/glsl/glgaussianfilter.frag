// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

uniform sampler3D _texture;
uniform TextureParameters3D _textureParams;
uniform float _zTexCoord;

uniform ivec3 _direction;

uniform int _halfKernelSize;
uniform samplerBuffer _kernel;


void main() {
    ivec3 texel = ivec3(vec3(ex_TexCoord.xy, _zTexCoord) * _textureParams._size);
    vec4 result = vec4(0.0, 0.0, 0.0, 0.0);
    float norm = 0.0;

    for (int i = -_halfKernelSize; i <= _halfKernelSize; ++i) {
        // TODO: why the fuck does abs(i) not work here?!?
        int absi = (i < 0) ? -i : i;

        ivec3 lookupTexel = texel + (_direction * i);
        if (all(greaterThanEqual(lookupTexel, ivec3(0, 0, 0))) && all(lessThan(lookupTexel, _textureParams._size))) {
            vec4 curValue = texelFetch(_texture, lookupTexel, 0);
            float mult = texelFetch(_kernel, absi).r;
            result += curValue * mult;
            norm += mult;
        }

    }

    out_Color = result / norm;
}
