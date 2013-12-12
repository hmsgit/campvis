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
#include <vector>

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
     * \note    All methods need to be called from a valid OpenGL context.
     */
    class GlReduction {
    public:
        /// Operation to be performed by reduction
        enum ReductionOperator {
            MIN,                ///< Minimum
            MAX,                ///< Maximum
            PLUS,               ///< Sum (plus)
            MULTIPLICATION,     ///< Product (multiplication)
            MIN_MAX_DEPTH_ONLY  ///< Minimum/Maximum (CAUTION: WORKS ONLY with depth images!!!)
        };

        /**
         * Constructor
         * \param   reductionOperator   Operation to be performed by reduction
         */
        GlReduction(ReductionOperator reductionOperator);

        /**
         * Destructor
         */
        ~GlReduction();

        /**
         * Performs the reduction on the given image.
         * \note    Image must be two-dimensional!
         * \note    Reduction will be performed on a copy, \a image will not change.
         * \param   image   Pointer to ImageData to perform reduction on, must be two-dimensional.
         * \return  A vector of floats containing the reduction results for each channel of the input image.
         */
        std::vector<float> reduce(const ImageData* image);

        /**
         * Performs the reduction on the given OpenGL texture.
         * \note    Image must be two-dimensional!
         * \note    Reduction will be performed on a copy, \a texture will not change.
         * \param   image   Pointer to a texture to perform reduction on, must be two-dimensional.
         * \return  A vector of floats containing the reduction results for each channel of the input texture.
         */
        std::vector<float> reduce(const tgt::Texture* texture);


    private:
        /**
         * Performs a reduction of \a currentSize in each dimension and adjusts \a texCoordMultiplier.
         * \param   currentSize         Current size to reduce (ceiling division by 2)
         * \param   texCoordMultiplier  Tex coordinates multiplier to adjust (divide by 2 if dimension got reduced)
         */
        static void reduceSizes(tgt::ivec2& currentSize, tgt::vec2& texCoordMultiplier);

        /**
         * Generates the GLSL header corresponding to the given reduction operator.
         * \param   reductionOperator   Operation to be performed by reduction
         * \return  A std::string with the corresponding GLSL header.
         */
        static std::string generateGlslHeader(ReductionOperator reductionOperator);


        ReductionOperator _reductionOperator;   ///< Operation to be performed by reduction
        tgt::Shader* _shader2d;                 ///< OpenGL shader performing 2D reduction
        tgt::Shader* _shader3d;                 ///< OpenGL shader performing 3D reduction
        tgt::FramebufferObject* _fbo;           ///< FBO performing the reduction

        const static std::string loggerCat_;
    };

}

#endif // GLREDUCTION_H__
