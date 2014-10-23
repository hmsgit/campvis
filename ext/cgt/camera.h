/**********************************************************************
 *                                                                    *
 * cgt - CAMP Graphics Toolbox, Copyright (C) 2012-2014               *
 *     Chair for Computer Aided Medical Procedures                    *
 *     Technische Universitaet Muenchen, Germany.                     *
 *     <http://campar.in.tum.de/>                                     *
 *                                                                    *
 * forked from tgt - Tiny Graphics Toolbox, Copyright (C) 2006-2011   *
 *     Visualization and Computer Graphics Group, Department of       *
 *     Computer Science, University of Muenster, Germany.             *
 *     <http://viscg.uni-muenster.de>                                 *
 *                                                                    *
 * This file is part of the cgt library. This library is free         *
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

#ifndef CGT_CAMERA_H
#define CGT_CAMERA_H

#include <cmath>
#include <vector>

#include "cgt/frustum.h"
#include "cgt/types.h"
#include "cgt/vector.h"
#include "cgt/matrix.h"
#include "cgt/quaternion.h"
#include "cgt/glcanvas.h"

namespace cgt {

/**
 * This class implements a standard Camera with a position, a focus point
 * and an up-vector which make up its orientation.
 */
class CGT_API Camera {
public:
    enum ProjectionMode {
        ORTHOGRAPHIC,
        PERSPECTIVE,
        FRUSTUM
    };

    /**
     * Constructor.  A standard starting speed and orientation are given.
     *
     * @param position coordinates of the point the camera should be located at
     * @param focus coordinates of the point the camera should look to
     * @param up the up-vector
     * @param fovy the field of view angle, in degrees, in the y direction
     * @param ratio the aspect ratio, ratio of x (width) to y (height)
     * @param distn distance to nearplane
     * @param distf distance to farplane
     */
    Camera(const vec3& position =  vec3(0.f, 0.f,  0.f),
           const vec3& focus    =  vec3(0.f, 0.f, -1.f),
           const vec3& up       =  vec3(0.f, 1.f,  0.f),
           float fovy           =  45.f,
           float ratio          =  static_cast<float>(GLCanvas::DEFAULT_WINDOW_WIDTH) /
                                   GLCanvas::DEFAULT_WINDOW_HEIGHT,
           float distn          =  0.1f,
           float distf          =  50.f,
           ProjectionMode pm    =  PERSPECTIVE);

    virtual ~Camera();

    /**
     * Creates a copy.
     */
    Camera* clone() const;

    /// Setter / Getter
    void setPosition(const vec3& pos) {
        position_ = pos;
        invalidateVM();
    }
    void setFocus(const vec3& foc) {
        focus_  = foc;
        invalidateVM();
    }
    void setUpVector(const vec3& up) {
        upVector_ = normalize(up);
        invalidateVM();
    }

    void setFrustum(const Frustum& frust) { frust_ = frust; }

    /// get Camera's strafe vector - a vector directing to the 'right'
    vec3 getStrafe()   const { return normalize(cross(getLook(), getUpVector())); }

    vec3 getUpVector() const { return upVector_; }
    vec3 getLook()     const { return normalize(getFocus() - getPosition()); }
    vec3 getPosition() const { return position_; }
    vec3 getFocus()    const { return focus_; }

    Frustum& getFrustum() { return frust_; }
    const Frustum& getFrustum() const { return frust_; }

    float getFovy() const { return frust_.getFovy(); }
    float getRatio() const { return frust_.getRatio(); }
    float getNearDist() const { return frust_.getNearDist(); }
    float getFarDist() const { return frust_.getFarDist(); }
    float getFrustLeft() const { return frust_.getLeft(); }
    float getFrustRight() const { return frust_.getRight(); }
    float getFrustTop() const { return frust_.getTop(); }
    float getFrustBottom() const { return frust_.getBottom(); }

    /// Set vertical viewing angle of camera.
    ///
    /// Normals of frustum do not get updated by this method, call updateFrustum manually before
    /// using e.g. culling methods!
    /// @param fovy angle in degree
    void setFovy(float fovy) {
        if(projectionMode_ == PERSPECTIVE) {
            float oldRatio = frust_.getRatio();
            frust_.setFovy(fovy);
            frust_.setRatio(oldRatio);
        }
    }

    /// Set aspect ratio.
    ///
    /// Normals of frustum do not get updated by this method, call updateFrustum manually before
    /// using e.g. culling methods!
    void setRatio(float ratio) {
        if(projectionMode_ == PERSPECTIVE)
            frust_.setRatio(ratio);
    }

    /// Set distance from camera to nearplane.
    ///
    /// Normals of frustum do not get updated by this method, call updateFrustum manually before
    /// using e.g. culling methods!  Also, in perspective mode, changing the near distance also internally
    //  changes the paramters "Field of Vision" and thereby "Aspect ratio", so we have to take care not to damage
    //  those values.
    void setNearDist(float neardist) {
        if(projectionMode_ == PERSPECTIVE) {
            float oldFovy = frust_.getFovy();
            float oldRatio = frust_.getRatio();
            frust_.setNearDist(neardist);
            frust_.setFovy(oldFovy);
            frust_.setRatio(oldRatio);
        } else
            frust_.setNearDist(neardist);
    }

    /// Set distance from camera to farplane.
    ///
    /// Normals of frustum do not get updated by this method, call updateFrustum manually before
    /// using e.g. culling methods!
    void setFarDist(float fardist) {
        frust_.setFarDist(fardist);
    }

    /// Set coordiante of left clipping plane.
    ///
    /// Normals of frustum do not get updated by this method, call updateFrustum manually before
    /// using e.g. culling methods!
    void setFrustLeft(float v) {
        frust_.setLeft(v);
        if(projectionMode_ == PERSPECTIVE)
            frust_.setRight(-v);
    }

    /// Set coordiante of right clipping plane.
    ///
    /// Normals of frustum do not get updated by this method, call updateFrustum manually before
    /// using e.g. culling methods!
    void setFrustRight(float v) {
        frust_.setRight(v);
        if(projectionMode_ == PERSPECTIVE)
            frust_.setLeft(-v);
    }

    /// Set coordiante of top clipping plane.
    ///
    /// Normals of frustum do not get updated by this method, call updateFrustum manually before
    /// using e.g. culling methods!
    void setFrustTop(float v) {
        frust_.setTop(v);
        if(projectionMode_ == PERSPECTIVE)
            frust_.setBottom(-v);
    }

    /// Set coordiante of bottom clipping plane.
    ///
    /// normals of frustum do not get updated by this method, call updateFrustum manually before
    /// using e.g. culling methods!
    void setFrustBottom(float v) {
        frust_.setBottom(v);
        if(projectionMode_ == PERSPECTIVE)
            frust_.setTop(-v);
    }

    void setFocalLength(float f)   {
        setFocus(getPosition() + f * getLook());
    }

    float getFocalLength() const   {
        return distance(getFocus(), getPosition());
    }

    void setWindowRatio(float r) {
        windowRatio_ = r;
    }

    float getWindowRatio() const {
        return windowRatio_;
    }

    quat getQuat() const {
        updateVM();
        return cgt::generateQuatFromMatrix(viewMatrix_.getRotationalPart());
    }

    /// Used to reposition the Camera.
    void positionCamera(const vec3& pos, const vec3& focus, const vec3& up) {
        setPosition(pos);
        setFocus(focus);
        setUpVector(up);
    }

    /// set projection mode
    void setProjectionMode(ProjectionMode pm) {
        projectionMode_ = pm;
        // reintroduce symmetry for perspective and orthogonal mode
        if(pm == PERSPECTIVE) {
            setFrustRight(-getFrustLeft());
            setFrustTop(-getFrustBottom());
        }
    }

    /// get projection mode
    ProjectionMode getProjectionMode() const {
        return projectionMode_;
    }

    /// actually turns on the Camera.
    void look();

    /// Update the frustum with the current camera parameters.
    /// This method MUST be called before a culling-method is used.  The reason this is not called
    /// in the culling-methods themselves is that we may have a lot of culling-querys without
    /// acutally changing the camera and thus, the frustum; in that case, frequently updating the
    /// frustum would be a waste of time.
    void updateFrustum();

    /// If you need the view matrix that would be used if look() was called, but without
    /// actually setting it in OpenGL, use this function.
    mat4 getViewMatrix() const;

    /// Sets a new view matrix; "reverse engineers" \a position_ and other parameters
    void setViewMatrix(const mat4& mvMat);

    /// This returns the inverse of the current ViewMatrix.
    mat4 getViewMatrixInverse() const;

    /// If you need only the matrix that represents the current rotation of the camera-tripod,
    /// use this function.
    mat4 getRotateMatrix() const;

    /// This method returns the frustum matrix
    virtual mat4 getFrustumMatrix() const;

    /// This method returns the projection matrix
    virtual mat4 getProjectionMatrix() const;

    vec3 project(ivec2 vp, vec3 point) const;

    bool operator==(const Camera& rhs) const;
    bool operator!=(const Camera& rhs) const;

protected:
    /// viewMatrix will not always be up to date according to position-, focus- and upVector.
    /// Make sure it is up to date.
    void updateVM() const {
        if (!viewMatrixValid_) {
            viewMatrix_ = mat4::createLookAt(getPosition(), getFocus(), getUpVector() );
        }
        viewMatrixValid_ = true;
    }

    /// Mark viewMatrix as outdated.
    void invalidateVM() const {
        viewMatrixValid_ = false;
    }

    vec3 position_; /// location of the camera
    vec3 focus_;    /// location, the camera looks at
    vec3 upVector_; /// up vector, always normalized

    /// A frustum is saved in order to cull objects that are not lying within the view of the
    /// camera
    Frustum frust_;

    float windowRatio_; /// Keep window ratio separate from frustum ratio

    /// This is the actual matrix that holds the current orientation and position of the
    /// Camera.
    mutable mat4 viewMatrix_;
    mutable bool viewMatrixValid_; /// if the model-view matrix is up-to-date

    ProjectionMode projectionMode_;
};

} //namespace cgt

#endif // CGT_CAMERA_H
