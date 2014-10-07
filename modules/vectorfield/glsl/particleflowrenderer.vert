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

#include "tools/texture3d.frag"

subroutine void RenderPassType();
subroutine uniform RenderPassType RenderPass;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Velocity;
layout(location = 2) in float in_StartTime;
layout(location = 3) in vec3 in_InitialPosition;

out vec3 ex_Position;
out vec3 ex_Velocity;
out float ex_StartTime;

out float ex_Transparency;

/// Matrix defining model-to-world transformation
uniform mat4 _modelMatrix = mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0);

/// Matrix defining view transformation
uniform mat4 _viewMatrix = mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0);

/// Matrix defining projection transformation
uniform mat4 _projectionMatrix = mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0);


uniform float _time;
uniform float _frameLength;
uniform float _lifetime;
uniform float _scale;
uniform vec2 _threshold;

uniform sampler3D _volume;
uniform TextureParameters3D _volumeTextureParams;


subroutine (RenderPassType)
void update() {
    if (_time > in_StartTime) {
        float age = _time - in_StartTime;
        if (age > _lifetime || length(in_Velocity) < _threshold.x || length(in_Velocity) > _threshold.y) {
            // particle expired, recycle
            ex_Position = in_InitialPosition;
            ex_Velocity = texture(_volume, (ex_Position / _volumeTextureParams._size).xyz).xyz;
            ex_StartTime = _time;
        }
        else {
            // particle alive, advance
            ex_Position = in_Position + (in_Velocity * _frameLength * 0.05 * _scale);

            // compute new velocity by mixture of old velocity and flow at current location, model some inertia
            vec3 v = texture(_volume, (ex_Position / _volumeTextureParams._size).xyz).xyz;
            ex_Velocity = mix(in_Velocity, v, 0.5);//smoothstep(0.5, 2.0, v/in_Velocity));
            ex_StartTime = in_StartTime;
        }
    }
    else {
        ex_Position = in_Position;
        ex_Velocity = in_Velocity;
        ex_StartTime = in_StartTime;
    }
}


subroutine (RenderPassType)
void render() {
    float age = _time - in_StartTime;
    ex_Transparency = (age >= 0.0) ? 1.0 - (age / _lifetime) : 0.0;

    gl_Position = _projectionMatrix * (_viewMatrix * (_modelMatrix * vec4(in_Position, 1.0)));
    //ex_Position = in_Position;
    mat4 normalMatrix = transpose(inverse(_modelMatrix));

    //vec4 normalTmp = (normalMatrix * vec4(in_Normal, 0.0));
    // ex_Normal = normalize((normalTmp).xyz);
}

void main() {
    // call the current subroutine
    RenderPass();
}
