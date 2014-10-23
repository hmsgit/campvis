/**********************************************************************
 *                                                                    *
 * cgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
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

#ifndef CGT_CGT_H
#define CGT_CGT_H

//Used Libs
#include "cgt/tgt_gl.h"

//Basics
#include "cgt/types.h"
#include "cgt/singleton.h"
#include "cgt/assert.h"
#include "cgt/init.h"

//Math
#include "cgt/vector.h"
#include "cgt/matrix.h"
#include "cgt/quaternion.h"
#include "cgt/plane.h"
#include "cgt/spline.h"
#include "cgt/bounds.h"
#include "cgt/frustum.h"

//Interaction
//#include "glcanvas.h"
// FIXME: include glcanvas and guiapplication, include glut/qt/sdl-canvas/application/libs depending on macros
#include "cgt/mouse.h"
#include "cgt/camera.h"
#include "cgt/navigation/navigation.h"

//Renderable
#include "cgt/renderable.h"
#include "cgt/quadric.h"
#include "cgt/skybox.h"
#include "cgt/curve.h"

//Light and Material
#include "cgt/light.h"
#include "cgt/material.h"

//Hardware Detection
#include "cgt/gpucapabilities.h"

//Managers
#include "cgt/manager.h"
#include "cgt/tesselator.h"
#include "cgt/texturemanager.h"
#include "cgt/shadermanager.h"
#include "cgt/modelmanager.h"

//Utils
#include "cgt/tesselator.h"
#include "cgt/stopwatch.h"
#include "cgt/framecounter.h"
#include "cgt/vertex.h"
#include "cgt/quadtree.h"

#endif //CGT_CGT_H
