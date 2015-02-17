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

in vec3 ex_TexCoord;            ///< incoming texture coordinate
out vec4 out_Color;             ///< outgoing fragment color

#include "tools/texture2d.frag"

uniform vec2 _viewportSizeRCP;

uniform bool _integrateGeometry;            ///< flag whether to integrate geometry into the EEP
uniform bool _isEntrypoint;                 ///< true if current run is for entrypoints, false if current run is for exitpoints

uniform sampler2D _entryDepthTexture;       ///< depth texture of the entrypoints (only used in exitpoints run)
uniform TextureParameters2D _entryDepthTexParams;

uniform sampler2D _geometryDepthTexture;    ///< depth texture of rendered geometry
uniform TextureParameters2D _geometryDepthTexParams;

uniform float _near;
uniform float _far;
uniform mat4 _inverseViewMatrix;            ///< inverse camera view matrix
uniform mat4 _inverseProjectionMatrix;      ///< inverse camera projection matrix
uniform mat4 _volumeWorldToTexture;         ///< world-to-texture matrix of volume

void main() {
    vec2 fragCoordNormalized = gl_FragCoord.xy * _viewportSizeRCP;
    float fragDepth = gl_FragCoord.z;

    if (_integrateGeometry) {
        float geometryDepth = texture(_geometryDepthTexture, fragCoordNormalized).r;

        if (_isEntrypoint) {
            // integrating geometry into Entrypoints
            float entryDepth = gl_FragCoord.z;

            if (geometryDepth <= entryDepth) {
                // geometry before Entrypoint
                out_Color = vec4(0.0);
                fragDepth = geometryDepth;
            }
            else {
                // geometry behind Entrypoint
                out_Color = vec4(ex_TexCoord, 1.0);
                fragDepth = entryDepth;
            }
        }
        else {
            // integrating geometry into Exitpoints
            float entryDepth = texture(_entryDepthTexture, fragCoordNormalized).r;
            float exitDepth = gl_FragCoord.z;

            if (geometryDepth <= entryDepth) {
                // geometry before Entrypoint
                out_Color = vec4(0.0);
                fragDepth = geometryDepth;
            }
            else if (geometryDepth <= exitDepth) {
                // geometry between entrypoint and exitpoint

                // transform viewport coordinates to [-1, 1] NDC
                vec4 result = vec4(fragCoordNormalized, geometryDepth, 1.0);
                result = 2.0 * result - 1.0;
            
                // reverse perspective division by w (which is equal to the camera-space z)
                float origZG = (2.0 * _far * _near) / ((_far + _near) - result.z * (_far - _near));
                result *= origZG;

                // unproject and reverse camera-transform
                result = vec4((_inverseViewMatrix * (_inverseProjectionMatrix * result)).xyz, 1.0);

                out_Color = vec4(result.xyz, 1.0);
                fragDepth = geometryDepth;
            }
            else {
                // geometry behind exitpoint
                out_Color = vec4(ex_TexCoord, 1.0);
                fragDepth = exitDepth;
            }
        }
    }
    else {
        out_Color = vec4(ex_TexCoord, 1.0);
    }

    gl_FragDepth = fragDepth;
}
