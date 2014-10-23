/**********************************************************************
 *                                                                    *
 * cgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2011 Visualization and Computer Graphics Group, *
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

#ifndef CGT_INIT_H
#define CGT_INIT_H

#include "cgt/types.h"
#include "cgt/logmanager.h"

namespace cgt {

class CGT_API InitFeature {
public:
    enum Features {
        NONE            =      0,
        LOG_MANAGER     = 1 << 0,
        FILE_SYSTEM     = 1 << 1,
        GPU_PROPERTIES  = 1 << 2,
        SCRIPT_MANAGER  = 1 << 3,
        SHADER_MANAGER  = 1 << 4,
        TEXTURE_MANAGER = 1 << 6,
        TESSELATOR      = 1 << 7,
        LOG_TO_CONSOLE  = 1 << 30,
        ALL             = (
                            LOG_MANAGER | FILE_SYSTEM | GPU_PROPERTIES |
                            SCRIPT_MANAGER | SHADER_MANAGER |
                            TEXTURE_MANAGER | TESSELATOR |
                            LOG_TO_CONSOLE
                            )
    };
};

/// init general purpose singletons of cgt
CGT_API void init(InitFeature::Features featureset = InitFeature::ALL, LogLevel logLevel = Info);
/// init GLEW and OpenGL-dependent singletons of cgt.
/// to be called when OpenGL context already exists.
CGT_API void initGL(InitFeature::Features featureset = InitFeature::ALL);

/// deinit the singletons of cgt
CGT_API void deinit();
/// deinit the singletons of cgt
CGT_API void deinitGL();

};

#endif //CGT_INIT_H
