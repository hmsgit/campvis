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

in vec3 ex_TexCoord;            ///< incoming texture coordinate
out vec4 out_Color;             ///< outgoing fragment color

#include "tools/texture2d.frag"
#include "tools/masking.frag"

uniform vec2 _viewportSizeRCP;

uniform bool _integrateGeometry;            ///< flag whether to integrate geometry into the EEP
uniform bool _isEntrypoint;                 ///< true if current run is for entrypoints, false if current run is for exitpoints
uniform Texture2D _entryColorTexture;       ///< depth texture of the entrypoints (only used in exitpoints run)
uniform Texture2D _entryDepthTexture;       ///< depth texture of the entrypoints (only used in exitpoints run)
uniform Texture2D _geometryDepthTexture;    ///< depth texture of rendered geometry

uniform float _near;
uniform float _far;
uniform mat4 _inverseViewMatrix;            ///< inverse camera view matrix
uniform mat4 _inverseProjectionMatrix;      ///< inverse camera projection matrix
uniform mat4 _volumeWorldToTexture;         ///< world-to-texture matrix of volume

void main() {
    vec2 fragCoordNormalized = gl_FragCoord.xy * _viewportSizeRCP;
    MASKING_PROLOG(fragCoordNormalized);

    float fragDepth = gl_FragCoord.z;

    if (_integrateGeometry) {
        float geometryDepth = getElement2DNormalized(_geometryDepthTexture, fragCoordNormalized).z;

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
            float entryDepth = getElement2DNormalized(_entryDepthTexture, fragCoordNormalized).z;
            float exitDepth = gl_FragCoord.z;

            if (geometryDepth <= entryDepth) {
                // geometry before Entrypoint
                out_Color = vec4(0.0);
                fragDepth = exitDepth;
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
                result = vec4((_volumeWorldToTexture * (_inverseViewMatrix * (_inverseProjectionMatrix * result))).xyz, 1.0);

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

    MASKING_EPILOG;
}
