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

#ifndef TGT_PAINTER_H
#define TGT_PAINTER_H

#include "tgt/types.h"
#include "tgt/vector.h"

namespace tgt {

class GLCanvas;
class Camera;

/**
 * Base class for all rendering on a canvas. Every canvas has a painter and use its
 * render method for rendering content.
 *
 * A tgt application is supposed to derive it's own Painter(s) and override paint, initialize
 * and sizeChanged method
 */
class TGT_API Painter {
public:
    /// Constructor
    Painter(GLCanvas* canvas = 0);

    /**
     * The owned GLCanvas should be destroyed manually otherwise endless
     * recursions can occur.
     */
    virtual ~Painter() {}

    /**
     * This is meant be overridden to do the according openGL paintings
     * is not meant to be called directly, will be called by GLCanvas object
     */
    virtual void paint() = 0;

    /// This is meant be overridden to adjust camera settings to new canvas dimensions
    virtual void sizeChanged(const ivec2&) {};

    /// This is meant be overridden to prepare OpenGl context for painting (e.g. set background color).
    /// If the user's derived painter class has additional attributes, they may be initialized here as well.
    virtual void init() {};

    /// Set the Canvas on which painter will draw
    void setCanvas(GLCanvas* canvas);

    /// Get the associated Canvas
    GLCanvas* getCanvas() const;

    /// A wrapper to get the camera from the Canvas
    Camera* getCamera() const;

private:
    GLCanvas* canvas_;
};

} // namespace tgt

#endif  //TGT_PAINTER_H
