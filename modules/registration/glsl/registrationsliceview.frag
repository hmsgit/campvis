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
in vec4 ex_Color;
out vec4 out_Color;

//#include "tools/background.frag"
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"

uniform sampler3D _refImage;
uniform sampler3D _movImage;
uniform TextureParameters3D _refImageParams;
uniform TextureParameters3D _movImageParams;

uniform sampler1D _refTf;
uniform sampler1D _movTf;
uniform TFParameters1D _refTfParams;
uniform TFParameters1D _movTfParams;

uniform mat4 _texCoordsMatrix;
uniform mat4 _trafoMatrix;
uniform vec3 _halfDiagonal;

uniform bool _useTexturing = true;
uniform bool _useSolidColor = true;
uniform vec4 _color = vec4(1.0, 1.0, 1.0, 1.0);

void main() {
    if (_useTexturing) {
        mat4 t2wr = _refImageParams._textureToWorldMatrix;
        mat4 w2tm = _movImageParams._worldToTextureMatrix;

        mat4 moveToCenter = mat4(
            1, 0, 0, 0,
            0, 1, 0, 0, 
            0, 0, 1, 0, 
            -_halfDiagonal.x, -_halfDiagonal.y, -_halfDiagonal.z, 1);
        mat4 moveFromCenter = mat4(
            1, 0, 0, 0,
            0, 1, 0, 0, 
            0, 0, 1, 0, 
            _halfDiagonal.x, _halfDiagonal.y, _halfDiagonal.z, 1);


        vec4 referenceLookupTexCoord = _texCoordsMatrix * vec4(ex_TexCoord, 1.0);
        vec4 movingLookupTexCoord = w2tm * moveFromCenter * _trafoMatrix * moveToCenter * t2wr * referenceLookupTexCoord;

        float refTexel = texture(_refImage, referenceLookupTexCoord.xyz).r;
        float movTexel = 0.0;
        if (all(greaterThanEqual(movingLookupTexCoord.xyz, vec3(0.0))) && all(lessThanEqual(movingLookupTexCoord.xyz, vec3(1.0)))) {
            movTexel = texture(_movImage, movingLookupTexCoord.xyz).r;
        }

        out_Color = (lookupTF(_refTf, _refTfParams, refTexel) + lookupTF(_movTf, _movTfParams, movTexel)) / 2.0;
        float maxElem = max(out_Color.x, max(out_Color.y, max(out_Color.z, out_Color.w)));
        if (maxElem > 1.0)
            out_Color /= maxElem;
    }
    else if (_useSolidColor) {
        out_Color = _color;
    }
    else {
        out_Color = ex_Color;
    }
}