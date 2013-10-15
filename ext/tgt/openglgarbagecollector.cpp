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

#include "openglgarbagecollector.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"

namespace tgt {
    
    OpenGLGarbageCollector::OpenGLGarbageCollector() {
        _currentFrontindex = 0;
    }

    OpenGLGarbageCollector::~OpenGLGarbageCollector() {
        deleteGarbage();
        deleteGarbage();
    }

    void OpenGLGarbageCollector::deleteGarbage() {
        tbb::spin_mutex::scoped_lock lock(_mutex);
        size_t backIndex;
        {
            backIndex = _currentFrontindex;
            tbb::spin_mutex::scoped_lock lock(_addMutex);
            _currentFrontindex = (_currentFrontindex+1) % 2;
        }
        
        // delete textures
        if (! _texturesToDelete[backIndex].empty()) {
            glDeleteTextures(static_cast<GLsizei>(_texturesToDelete[backIndex].size()), &(_texturesToDelete[backIndex].front()));
            //LDEBUGC("tgt.OpenGLGarbageCollector", "Deleted " << _texturesToDelete[backIndex].size() << " Textures.");
            _texturesToDelete[backIndex].clear();
        }

        // delete FBOs
        if (! _fbosToDelete[backIndex].empty()) {
            glDeleteFramebuffers(static_cast<GLsizei>(_fbosToDelete[backIndex].size()), &(_fbosToDelete[backIndex].front()));
            //LDEBUGC("tgt.OpenGLGarbageCollector", "Deleted " << _fbosToDelete[backIndex].size() << " FBOs.");
            _fbosToDelete[backIndex].clear();
        }

        // delete buffers
        if (! _buffersToDelete[backIndex].empty()) {
            glDeleteBuffers(static_cast<GLsizei>(_buffersToDelete[backIndex].size()), &(_buffersToDelete[backIndex].front()));
           // LDEBUGC("tgt.OpenGLGarbageCollector", "Deleted " << _buffersToDelete[backIndex].size() << " Buffers.");
            _buffersToDelete[backIndex].clear();
        }

        LGL_ERROR;
    }

}

