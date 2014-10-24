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

#ifndef CGT_NAVIGATION_H
#define CGT_NAVIGATION_H

#include "cgt/cgt_gl.h"

#include "cgt/bounds.h"
#include "cgt/camera.h"
#include "cgt/glcanvas.h"
#include "cgt/types.h"
#include "cgt/vector.h"
#include "cgt/event/mouseevent.h"
#include "cgt/event/eventlistener.h"

/**
    This is the base class for navigation metaphores.
    Derived classes offer solutions to use typical high-level camera movements in object space
    (e.g. moving camera upon a trackball by mouse, make tracking shot following a spline) in a
    simple way.

    In addition, class it implements basic camera operations like moving or rotating the camera
    in object space. Most deriving classes use these operations to do advanced navigations.
*/

namespace cgt {

/**
 * Intermediate wrapper class for easier integration of cgt::Navigation into projects using
 * separated cameras and canvases.
 */
class CGT_API IHasCamera {
public:
    /**
     * Pure virtual destructor
     */
    virtual ~IHasCamera() {};

    /**
     * Returns the camera to modify by Navigation.
     * Once a navigation is finished and the scene shall be rerendered, update() is called.
     */
    virtual Camera* getCamera() = 0;

    /**
     * Notifies the camera holding object, that the navigation is finished.
     */
    virtual void update() = 0;
};


class CGT_API Navigation : virtual public EventListener {

protected:

    /**
        * Updates the near-/far clipping planes. 
        * Does not issue an update command to the camera.
        */
    void updateClippingPlanes();;
        
    Bounds _sceneBounds;

    // navigation manipulates the camera of a certain canvas.
    // we only need a pointer to this canvas, not to the camera (see getCamera).
    IHasCamera* hcam_;


public:

    Navigation(IHasCamera* hcam) {
        hcam_ = hcam;
    }

    virtual ~Navigation() {}


    /**
     * Sets the scene bounds for automatic near/far clipping plane adjustment.
     * \param   bounds  New bounds for the rendered scene.
     */
    void setSceneBounds(const cgt::Bounds& bounds);
    
    /**
        * Returns the current scene bounds for this trackball.
        * \return  _sceneBounds
        */
    const cgt::Bounds& getSceneBounds() const;

    //     void setCanvas(GLCanvas* hcam) { hcam_ = hcam; }
//     GLCanvas* getCanvas() const { return hcam_->; }

    Camera* getCamera() const { return hcam_->getCamera(); }

    /// The following functions may be used to rotate the Camera about
    /// an arbitrary axis.
    void rotateView(float angle, float x, float y, float z);
    void rotateView(float angle, const vec3& axis);

    /// This function rotates the view about the up- and strafe-vector
    void rotateViewHV(float anglehorz, float anglevert);

    /// The following functions may be used to rotate the Camera-View about
    /// the Up- and the Strafe-Vector; they exist for reasons of convenience.
    void rotateViewVert(float angle);
    void rotateViewHorz(float angle);

    /// The following functions may be used to rotate the Up-Vector about
    /// the Strafe- and the Look-Vector.  Use this with care since it may
    /// leave the Camera with a "strange" orientation.
    void rollCameraVert(float angle);
    void rollCameraHorz(float angle);

    /// The following functions may be used to move the camera a given
    /// distance in a certain direction.
    void moveCameraForward(float length);
    void moveCameraBackward(float length);
    void moveCameraUp(float length);
    void moveCameraDown(float length);
    void moveCameraRight(float length);
    void moveCameraLeft(float length);
    void moveCamera(float length, float x, float y, float z);
    void moveCamera(float length, const vec3& axis);
    void moveCamera(const vec3& motionvector);

};

} // namespace cgt

#endif //CGT_NAVIGATION_H
