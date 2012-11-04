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

#ifndef DEPTHDARKENING_H__
#define DEPTHDARKENING_H__

#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/numericproperty.h"

namespace tgt {
    class Shader;
}

namespace campvis {
    class ImageData;

    /**
     * Extracts a slice from a 3D image and renders it into a rendertarget.
     */
    class DepthDarkening : public VisualizationProcessor {
    public:
        /**
         * Constructs a new DepthDarkening Processor
         **/
        DepthDarkening(GenericProperty<tgt::ivec2>& canvasSize);

        /**
         * Destructor
         **/
        virtual ~DepthDarkening();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "DepthDarkening"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Applies depth darkening post processing to simulate shadows."; };

        virtual void process(DataContainer& data);

        DataNameProperty p_inputImage;               ///< image ID for input image
        DataNameProperty p_outputImage;              ///< image ID for output image

        FloatProperty p_sigma;                       ///< sigma, standard deviation of the gaussian filter
        FloatProperty p_lambda;                      ///< strength of depth effect

        BoolProperty p_useColorCoding;               ///< Flag whether to use depth color coding
        Vec3Property p_coldColor;                    ///< Cold color (color for far objects)
        Vec3Property p_warmColor;                    ///< Warm color (color for near objects)

    protected:
        /**
         * Generates the GLSL Header
         * \return  #define uSE_COLORCODING if p_useColorCoding is set
         */
        std::string generateHeader() const;

        tgt::Shader* _shader;                           ///< Shader for slice rendering

        static const std::string loggerCat_;
    };

}

#endif // DEPTHDARKENING_H__
