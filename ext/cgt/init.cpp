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

#include "cgt/init.h"
#include "cgt/cgt_gl.h"

#include "cgt/assert.h"
#include "cgt/glcanvas.h"
#include "cgt/glcontextmanager.h"
#include "cgt/gpucapabilities.h"
#ifdef _MSC_VER
    #include "cgt/gpucapabilitieswindows.h"
#endif
#include "cgt/opengljobprocessor.h"
#include "cgt/shadermanager.h"
#include "cgt/singleton.h"
#include "cgt/texturereadertga.h"

#include "cgt/event/eventhandler.h"

#ifdef CGT_HAS_DEVIL
#include <IL/il.h>
#include <IL/ilu.h>
#endif



namespace cgt {

void init(InitFeature::Features featureset, LogLevel logLevel) {
    if (featureset & InitFeature::SHADER_MANAGER) {
        featureset = (InitFeature::Features) (featureset | InitFeature::GPU_PROPERTIES | InitFeature::FILE_SYSTEM);
    }

    if (featureset & InitFeature::TEXTURE_MANAGER) {
        featureset = (InitFeature::Features) (featureset | InitFeature::GPU_PROPERTIES | InitFeature::FILE_SYSTEM);
    }

    if (featureset & InitFeature::LOG_MANAGER) {
        LogManager::init();
        if (featureset & InitFeature::LOG_TO_CONSOLE) {
            ConsoleLog* log = new ConsoleLog();
            log->addCat("", true, logLevel);
            LogMgr.addLog(log);
        }
        // LogMgr disposes all its logs
    }

    if (featureset & InitFeature::FILE_SYSTEM)
        FileSystem::init();

#ifdef CGT_HAS_DEVIL
    ilInit();
    iluInit();
#endif
}

void deinit() {
    if (FileSystem::isInited())
        FileSystem::deinit();

    if (LogManager::isInited())
        LogManager::deinit();
}

void initGL(GLCanvas* backgroundGlContext, InitFeature::Features featureset) {
    if (featureset & InitFeature::SHADER_MANAGER) {
        featureset = (InitFeature::Features) (featureset | InitFeature::GPU_PROPERTIES | InitFeature::FILE_SYSTEM);
    }
    if (featureset & InitFeature::TEXTURE_MANAGER) {
        featureset = (InitFeature::Features) (featureset | InitFeature::GPU_PROPERTIES | InitFeature::FILE_SYSTEM);
    }

    // init and register background context
    GlContextManager::init();
    OpenGLJobProcessor::init();
    GlContextManager::getRef().registerContextAndInitGlew(backgroundGlContext, "CGT Background Context");

    if (featureset & InitFeature::GPU_PROPERTIES )
        GpuCapabilities::init();
#ifdef _MSC_VER
        GpuCapabilitiesWindows::init();
#endif

    // starting shadermanager
    ShaderManager::init();
}

void deinitGL() {
    GLCanvas* backgroundGlContext = GLJobProc.getContext();
    {
        // Deinit everything OpenGL related using the local context.
        GLContextScopedLock lock(backgroundGlContext);

        if (GpuCapabilities::isInited())
            GpuCapabilities::deinit();
#ifdef _MSC_VER
        if (GpuCapabilitiesWindows::isInited())
            GpuCapabilitiesWindows::deinit();
#endif
        if (ShaderManager::isInited())
            ShaderManager::deinit();
    }

    GLJobProc.stop();
    OpenGLJobProcessor::deinit();
    GlContextManager::deinit();
}

} // namespace
