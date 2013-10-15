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

#ifndef OPENGLGARBAGECOLLECTOR_H__
#define OPENGLGARBAGECOLLECTOR_H_


#include "tgt/tgt_gl.h"
#include "tgt/singleton.h"
#include <tbb/atomic.h>
#include <tbb/concurrent_vector.h>
#include <tbb/spin_mutex.h>

#include <vector>

namespace tgt {
    /**
     * Singleton class for collecting OpenGL stuff that is to be deleted.
     * 
     * This class is to be considered as thread-safe.
     */
    class OpenGLGarbageCollector : public Singleton<OpenGLGarbageCollector> {
        friend class Singleton<OpenGLGarbageCollector>;

    public:

        /**
         * Destructor, deletes all unfinished jobs.
         */
        ~OpenGLGarbageCollector();

        void addGarbageTexture(GLuint id) {
            tbb::spin_mutex::scoped_lock lock(_addMutex);
            _texturesToDelete[_currentFrontindex].push_back(id);
        };

        void addGarbageFramebufferObject(GLuint id) {
            tbb::spin_mutex::scoped_lock lock(_addMutex);
            _fbosToDelete[_currentFrontindex].push_back(id);
        };

        void addGarbageBufferObject(GLuint id) {
            tbb::spin_mutex::scoped_lock lock(_addMutex);
            _buffersToDelete[_currentFrontindex].push_back(id);
        };

        void deleteGarbage();


    protected:
        OpenGLGarbageCollector();

        std::vector<GLuint> _texturesToDelete[2];
        std::vector<GLuint> _fbosToDelete[2];
        std::vector<GLuint> _buffersToDelete[2];

        tbb::atomic<size_t> _currentFrontindex;
        tbb::spin_mutex _mutex;
        tbb::spin_mutex _addMutex;
    };

}

#define GLGC tgt::Singleton<tgt::OpenGLGarbageCollector>::getRef()

#endif // OPENGLGARBAGECOLLECTOR_H__
