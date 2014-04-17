// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#ifndef GLREDUCTION_H__
#define GLREDUCTION_H__

#include "tgt/exception.h"
#include "tgt/tgt_gl.h"
#include "tgt/vector.h"

#include "core/coreapi.h"

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
    class CAMPVIS_CORE_API GlReduction {
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
        GlReduction(ReductionOperator reductionOperator, bool isForTesting = false);

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
         * Generates the GLSL header corresponding to the given reduction operator.
         * \param   reductionOperator   Operation to be performed by reduction
         * \return  A std::string with the corresponding GLSL header.
         */
        static std::string generateGlslHeader(ReductionOperator reductionOperator);


        ReductionOperator _reductionOperator;   ///< Operation to be performed by reduction
        tgt::Shader* _shader1d;                 ///< OpenGL shader performing 1D reduction
        tgt::Shader* _shader2d;                 ///< OpenGL shader performing 2D reduction
        tgt::Shader* _shader3d;                 ///< OpenGL shader performing 3D reduction
        tgt::FramebufferObject* _fbo;           ///< FBO performing the reduction

        const static std::string loggerCat_;
    };

}

#endif // GLREDUCTION_H__
