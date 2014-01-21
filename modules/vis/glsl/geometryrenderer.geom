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


// Code taken and inspired from the "OpenGL 4.0 Shading Language Cookbook", Chapter 6:

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 vert_Position[];
in vec3 vert_TexCoord[];           ///< incoming texture coordinate
in vec4 vert_Color[];
in vec3 vert_Normal[];

out vec3 geom_Position;
out vec3 geom_TexCoord;
out vec4 geom_Color;         ///< outgoing fragment color
out vec3 geom_Normal;
noperspective out vec3 geom_EdgeDistance;

/// Matrix defining viewport transformation
uniform mat4 _viewportMatrix;
uniform bool _computeNormals;

void main() {
    vec3 normal = normalize(cross(vert_Position[2].xyz - vert_Position[0].xyz, vert_Position[1].xyz - vert_Position[0].xyz));

    // Transform each vertex into viewport space
    vec3 p0 = vec3(_viewportMatrix * (gl_in[0].gl_Position / gl_in[0].gl_Position.w));
    vec3 p1 = vec3(_viewportMatrix * (gl_in[1].gl_Position / gl_in[1].gl_Position.w));
    vec3 p2 = vec3(_viewportMatrix * (gl_in[2].gl_Position / gl_in[2].gl_Position.w));

    // Find the altitudes (ha, hb and hc)
    float a = length(p1 - p2);
    float b = length(p2 - p0);
    float c = length(p1 - p0);
    float alpha = acos((b*b + c*c - a*a) / (2.0*b*c));
    float beta = acos((a*a + c*c - b*b) / (2.0*a*c));
    float ha = abs(c * sin(beta));
    float hb = abs(c * sin(alpha));
    float hc = abs(b * sin(alpha));

    // Send the triangle along with the edge distances
    geom_EdgeDistance = vec3(ha, 0.0, 0.0);
    geom_Normal = _computeNormals ? normal : vert_Normal[0];
    geom_Position = vert_Position[0];
    geom_TexCoord = vert_TexCoord[0];
    geom_Color = vert_Color[0];
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    geom_EdgeDistance = vec3(0.0, hb, 0.0);
    geom_Normal = _computeNormals ? normal : vert_Normal[1];
    geom_Position = vert_Position[1];
    geom_TexCoord = vert_TexCoord[1];
    geom_Color = vert_Color[1];
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    geom_EdgeDistance = vec3(0.0, 0.0, hc);
    geom_Normal = _computeNormals ? normal : vert_Normal[2];
    geom_Position = vert_Position[2];
    geom_TexCoord = vert_TexCoord[2];
    geom_Color = vert_Color[2];
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();

    EndPrimitive();
}