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

#ifndef GLREDUCTION_H__
#define GLREDUCTION_H__

#include "tgt/exception.h"
#include "tgt/tgt_gl.h"
#include "tgt/vector.h"

#include <string>

namespace tgt {
    class FramebufferObject;
    class Shader;
    class Texture;
}

namespace campvis {
    class ImageData;
    class FaceGeometry;

    /**
     * Class performing a reduction of 2D image data using OpenGL.
     * TODO: implement 3D reduction - shouldn't be that difficult
     */
    class GlReduction {
    public:
        GlReduction();

        ~GlReduction();



        float reduce(const ImageData* image);

        float reduce(const tgt::Texture* texture);


    private:
        /**
         * Performs a reduction of \a currentSize in each dimension and adjusts \a texCoordMultiplier.
         * \param   currentSize         Current size to reduce (ceiling division by 2)
         * \param   texCoordMultiplier  Tex coordinates multiplier to adjust (divide by 2 if dimension got reduced)
         */
        static void reduceSizes(tgt::ivec2& currentSize, tgt::vec2& texCoordMultiplier);

        size_t _readTex;
        size_t _writeTex;

        tgt::Shader* _shader;
        tgt::FramebufferObject* _fbo;
        tgt::Texture* _tempTextures[2];
        FaceGeometry* _renderQuad;

        const static std::string loggerCat_;
    };

}

#endif // GLREDUCTION_H__
