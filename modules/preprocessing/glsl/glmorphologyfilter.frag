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

#define FILTER_3D

#ifdef FILTER_2D

uniform sampler2D _texture;
uniform ivec3 _textureSize;

void main() {
    ivec2 texel = ivec2(ex_TexCoord.xy * vec2(_textureSize.xy));

    out_Color = texelFetch(_texture, texel, 0);

    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec2(1, 0));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec2(-1, 0));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec2(0, 1));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec2(0, -1));
}

#endif



#ifdef FILTER_3D
uniform sampler3D _texture;
uniform ivec3 _textureSize;
uniform int _zTexCoord;

void main() {
    ivec3 texel = ivec3(ex_TexCoord.xy * vec2(_textureSize.xy), _zTexCoord);

    out_Color = texelFetch(_texture, texel, 0);

#ifdef CUBE_ELEMENT
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3(-1, -1, -1)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 0, -1, -1)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 1, -1, -1)));

    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3(-1,  0, -1)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 0,  0, -1)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 1,  0, -1)));

    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3(-1,  1, -1)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 0,  1, -1)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 1,  1, -1)));

    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3(-1, -1,  0)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 0, -1,  0)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 1, -1,  0)));

    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3(-1,  0,  0)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 1,  0,  0)));

    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3(-1,  1,  0)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 0,  1,  0)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 1,  1,  0)));

    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3(-1, -1,  1)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 0, -1,  1)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 1, -1,  1)));

    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3(-1,  0,  1)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 0,  0,  1)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 1,  0,  1)));

    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3(-1,  1,  1)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 0,  1,  1)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 1,  1,  1)));
#endif

#ifdef CROSS_ELEMENT
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3(-1,  0,  0)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 1,  0,  0)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 0, -1,  0)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 0,  1,  0)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 0,  0, -1)));
    out_Color = FILTER_OP(out_Color, texelFetchOffset(_texture, texel, 0, ivec3( 0,  0,  1)));
#endif
}
#endif

