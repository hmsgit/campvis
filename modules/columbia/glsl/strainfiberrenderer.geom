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

layout (lines) in;

#ifdef DO_STRIPES
layout (triangle_strip, max_vertices = 4) out;
#endif

#ifdef DO_TUBES
#define NUM_SIDES 6
#define MAX_VERTICES 14 // 2*(NUM_SIDES + 1)
layout (triangle_strip, max_vertices = MAX_VERTICES) out;
#endif


in vec3 vert_TexCoord[];           ///< incoming texture coordinate
in vec4 vert_Position[];
in vec4 vert_Color[];

out vec3 geom_Normal;
out vec4 geom_Position;
out vec4 geom_Color;         ///< outgoing fragment color
out float geom_SineFlag;

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

uniform vec3 _cameraPosition;
uniform float _fiberWidth;

const float PI = 3.1415926535897932384626433832795;

void main() {

#ifdef DO_STRIPES
    vec4 displacement[2];
    vec3 normal[2];

    for (int i = 0; i < 2; ++i) {
        // compute normal
        normal[i] = normalize((vert_Position[i].xyz / vert_Position[i].w) - _cameraPosition);

        // gather displacement vector from tangent vector (stored in normal vector)
        displacement[i] = normalize(vec4(cross(normal[i], vert_TexCoord[i]), 0.0));
        displacement[i] *= _fiberWidth;
    }

    geom_Color = vec4(vert_TexCoord[0], 1.0);

    geom_Position = _projectionMatrix * (_viewMatrix * (_modelMatrix * (vert_Position[0] + displacement[0])));
    geom_Normal = normal[0];
    geom_SineFlag = 0.0;
    gl_Position = geom_Position;
    EmitVertex();

    geom_Position = _projectionMatrix * (_viewMatrix * (_modelMatrix * (vert_Position[0] - displacement[0])));
    geom_Normal = normal[0];
    geom_SineFlag = 1.0;
    gl_Position = geom_Position;
    EmitVertex();

    geom_Color = vec4(vert_TexCoord[1], 1.0);

    geom_Position = _projectionMatrix * (_viewMatrix * (_modelMatrix * (vert_Position[1] + displacement[1])));
    geom_Normal = normal[1];
    geom_SineFlag = 0.0;
    gl_Position = geom_Position;
    EmitVertex();

    geom_Position = _projectionMatrix * (_viewMatrix * (_modelMatrix * (vert_Position[1] - displacement[1])));
    geom_Normal = normal[1];
    geom_SineFlag = 1.0;
    gl_Position = geom_Position;
    EmitVertex();

    EndPrimitive();
#endif

#ifdef DO_TUBES
    mat3 rotMatrix[2];
    for (int i = 0; i < 2; ++i) {
        // calculate correct rotation matrix for pseudo-cylinder footprint:
        vec3 rotBotZ = vert_TexCoord[i]; // already normalized!
        bool degenerated = (rotBotZ.x == 0 && rotBotZ.z == 0);
        vec3 rotBotX = degenerated ? vec3(1.f, 0.f, 0.f) : normalize(vec3(rotBotZ.z, 0.f, -rotBotZ.x));
        vec3 rotBotY = degenerated ? vec3(0.f, 0.f, 1.f) : normalize(cross(rotBotZ, rotBotX));
        rotMatrix[i] = mat3(rotBotX, rotBotY, rotBotZ);
    }

    vec3 normals[NUM_SIDES];
    for (int i = 0; i < NUM_SIDES; ++i) {
        float angle = float(i)/float(NUM_SIDES) * 2.0 * PI;
        normals[i] = normalize(vec3(cos(angle) - sin(angle), sin(angle) + cos(angle), 0.0));
    }

    for (int i = 0; i < NUM_SIDES; ++i) {
        geom_Color = vec4(vert_TexCoord[0], 1.0);
        geom_Normal = normals[i] * rotMatrix[0];
        geom_Position = _projectionMatrix * (_viewMatrix * (_modelMatrix * (vert_Position[0] + (vec4(geom_Normal, 0.0) * _fiberWidth))));
        gl_Position = geom_Position;
        EmitVertex();

        geom_Color = vec4(vert_TexCoord[1], 1.0);
        geom_Normal = normals[i] * rotMatrix[1];
        geom_Position = _projectionMatrix * (_viewMatrix * (_modelMatrix * (vert_Position[1] + (vec4(geom_Normal, 0.0) * _fiberWidth))));
        gl_Position = geom_Position;
        EmitVertex();
    }

    geom_Color = vec4(vert_TexCoord[0], 1.0);
    geom_Normal = normals[0] * rotMatrix[0];
    geom_Position = _projectionMatrix * (_viewMatrix * (_modelMatrix * (vert_Position[0] + (vec4(geom_Normal, 0.0) * _fiberWidth))));
    gl_Position = geom_Position;
    EmitVertex();

    geom_Color = vec4(vert_TexCoord[1], 1.0);
    geom_Normal = normals[0] * rotMatrix[1];
    geom_Position = _projectionMatrix * (_viewMatrix * (_modelMatrix * (vert_Position[1] + (vec4(geom_Normal, 0.0) * _fiberWidth))));
    gl_Position = geom_Position;
    EmitVertex();

    EndPrimitive;

#endif
}