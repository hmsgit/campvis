/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2011 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#include "cgt/camera.h"

#include "cgt/assert.h"
#include "cgt/glmath.h"
#include "cgt/quaternion.h"
#include "cgt/tgt_gl.h"

#include <cmath>
#include <iostream>

namespace tgt {

// Constructor
Camera::Camera(const vec3& position, const vec3& focus, const vec3& up,
               float fovy, float ratio, float distn, float distf, ProjectionMode pm)
    : position_(position),
      focus_(focus),
      upVector_(normalize(up)),
      frust_(Frustum(fovy, ratio, distn, distf)),
      windowRatio_(1.f),
      viewMatrixValid_(false),
      projectionMode_(pm)
{
    viewMatrix_ = mat4::createLookAt(position, focus, up);
}

Camera::~Camera() {
}

Camera* Camera::clone() const {
    Camera* cam = new Camera(position_, focus_, upVector_, frust_.getFovy(),
        frust_.getRatio(), frust_.getNearDist(), frust_.getFarDist());
    cam->setWindowRatio(windowRatio_);
    return cam;
}

bool Camera::operator==(const Camera& rhs) const {
    return (rhs.position_ == position_) && (rhs.focus_ == focus_) && (rhs.upVector_ == upVector_) && (rhs.frust_ == frust_) &&
        (rhs.windowRatio_ == windowRatio_) && (rhs.projectionMode_ == projectionMode_);
}

bool Camera::operator!=(const Camera& rhs) const {
    return !(*this == rhs);
}

// This is called to set up the Camera-View
void Camera::look() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    updateFrustum();
    loadMatrix(getFrustumMatrix());
    //getProjectionMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    updateVM();
    loadMatrix(viewMatrix_);
}

// Private method that updates the relevant frustum parameters
void Camera::updateFrustum() {
    frust_.update(this);
}

mat4 Camera::getViewMatrix() const {
    updateVM();
    return viewMatrix_;
}

void Camera::setViewMatrix(const mat4& mvMat) {
    mat4 inv;
    if (mvMat.invert(inv)) {
        // preserve the focallength
        float focallength = length(focus_ - position_);

        // calculate world-coordinates
        vec4 pos   = (inv * vec4(0.f, 0.f,  0.f, 1.f));
        vec4 look  = (inv * vec4(0.f, 0.f, -1.f, 0.f));
        vec4 focus = pos + focallength * look;
        vec4 up    = (inv * vec4(0.f, 1.f,  0.f, 0.f));

        positionCamera(pos.xyz(), focus.xyz(), up.xyz());

        viewMatrix_ = mvMat;
    }
}

mat4 Camera::getRotateMatrix() const {
    updateVM();
    return viewMatrix_.getRotationalPart();
}

mat4 Camera::getViewMatrixInverse() const {
    updateVM();
    mat4 inv;
    if (viewMatrix_.invert(inv))
        return inv;
    else
        return mat4::identity;
}

mat4 Camera::getFrustumMatrix() const {
    return mat4::createFrustum(frust_.getLeft() * windowRatio_, frust_.getRight() * windowRatio_,
                               frust_.getBottom(), frust_.getTop(),
                               frust_.getNearDist(), frust_.getFarDist());
}

mat4 Camera::getProjectionMatrix() const {
    if(projectionMode_ == ORTHOGRAPHIC) {
        if(windowRatio_ > 1.0f)
            return mat4::createOrtho(frust_.getLeft() * windowRatio_, frust_.getRight() * windowRatio_,
                                     frust_.getTop(), frust_.getBottom(),
                                    -frust_.getNearDist(), frust_.getFarDist());
        else
            return mat4::createOrtho(frust_.getLeft(), frust_.getRight(),
                                     frust_.getTop() * (1.0f/windowRatio_), frust_.getBottom() * (1.0f/windowRatio_),
                                    -frust_.getNearDist(), frust_.getFarDist());
    } else if(projectionMode_ == PERSPECTIVE) {
        float fovy = frust_.getFovy();
        if(fovy < 6.f)
            fovy = 6.f;
        if(fovy > 175.f)
            fovy = 175.f;

        return mat4::createPerspective(deg2rad(fovy), frust_.getRatio() * windowRatio_, frust_.getNearDist(), frust_.getFarDist());
    }
    else
        return getFrustumMatrix();
}

vec3 Camera::project(ivec2 vp, vec3 point) const {
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];

    tgt::mat4 projection_tgt = getProjectionMatrix();
    tgt::mat4 modelview_tgt = getViewMatrix();
    for (int i = 0; i < 4; ++i) {
        modelview[i+0]   = modelview_tgt[i].x;
        modelview[i+4]   = modelview_tgt[i].y;
        modelview[i+8]   = modelview_tgt[i].z;
        modelview[i+12]  = modelview_tgt[i].w;
        projection[i+0]  = projection_tgt[i].x;
        projection[i+4]  = projection_tgt[i].y;
        projection[i+8]  = projection_tgt[i].z;
        projection[i+12] = projection_tgt[i].w;
    }
    viewport[0] = 0;
    viewport[1] = 0;
    viewport[2] = vp.x;
    viewport[3] = vp.y;

    GLdouble pointProjectedGL[3];
    gluProject(point.x, point.y, point.z, modelview, projection, viewport,
               &pointProjectedGL[0], &pointProjectedGL[1], &pointProjectedGL[2]);

    return tgt::vec3(static_cast<float>(pointProjectedGL[0]),
                     static_cast<float>(pointProjectedGL[1]),
                     static_cast<float>(pointProjectedGL[2]));
}

} // namespace tgt
