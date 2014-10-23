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

#include "cgt/tgt_gl.h"

#include "cgt/logmanager.h"

namespace tgt {

GLenum _lGLError(int line, const char* file) {
    GLenum err = glGetError();

    if (err != GL_NO_ERROR) {
        const GLubyte* exp = gluErrorString(err);

        std::ostringstream tmp1, tmp2, loggerCat;
        tmp2 << " File: " << file << "@" << line;
        tmp1 << (exp ? (const char*) exp : "unknown");
        loggerCat << "gl-error:" << file << ':' << line;

        LogMgr.log(loggerCat.str(), tgt::Error, tmp1.str(), tmp2.str());
    }

    return err;
}

GLboolean getGlBool(GLenum param) {
    GLboolean toReturn;
    glGetBooleanv(param, &toReturn);
    return toReturn;
}

GLint getGlInt(GLenum param) {
    GLint toReturn;
    glGetIntegerv(param, &toReturn);
    return toReturn;
}

GLfloat getGlFloat(GLenum param) {
    GLfloat toReturn;
    glGetFloatv(param, &toReturn);
    return toReturn;
}

} // namespace tgt
