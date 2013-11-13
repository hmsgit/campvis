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

#ifndef SIMILARITYMEASURE_H__
#define SIMILARITYMEASURE_H__

#include <string>

#include "tgt/buffer.h"
#include "tgt/matrix.h"
#include "tgt/vertexarrayobject.h"

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/buttonproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/optionproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/transferfunctionproperty.h"

#include <nlopt.hpp>

namespace tgt {
    class Shader;
}

namespace campvis {
    class FaceGeometry;
    class ImageData;
    class ImageRepresentationGL;
    class GlReduction;

    /**
     * Computes a Similarity Measure using OpenGL
     */
    class SimilarityMeasure : public VisualizationProcessor {
    public:
        enum AdditionalInvalidationLevels {
            PERFORM_OPTIMIZATION = 1U << 6,
            COMPUTE_DIFFERENCE_IMAGE = 1U << 7
        };

        /**
         * Constructs a new SimilarityMeasure Processor
         **/
        SimilarityMeasure();

        /**
         * Destructor
         **/
        virtual ~SimilarityMeasure();

        /// \see AbstractProcessor::init
        virtual void init();
        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "SimilarityMeasure"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Extracts a single slice from the input image and renders it using a transfer function."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        /// \see AbstractProcessor::process()
        virtual void process(DataContainer& data);

        DataNameProperty p_referenceId;                 ///< image ID for reference image
        DataNameProperty p_movingId;                    ///< image ID for moving image

        IVec2Property p_clipX;                          ///< clip coordinates for x axis
        IVec2Property p_clipY;                          ///< clip coordinates for y axis
        IVec2Property p_clipZ;                          ///< clip coordinates for z axis

        BoolProperty p_applyMask;
        Vec3Property p_translation;                     ///< Moving image translation
        Vec3Property p_rotation;                        ///< Moving image rotation

        GenericOptionProperty<std::string> p_metric;
        ButtonProperty p_computeSimilarity;

        DataNameProperty p_differenceImageId;
        ButtonProperty p_computeDifferenceImage;

        GenericOptionProperty<nlopt::algorithm> p_optimizer;
        ButtonProperty p_performOptimization;
        ButtonProperty p_forceStop;

    private:
        struct MyFuncData_t {
            SimilarityMeasure* _object;
            const ImageRepresentationGL* _reference;
            const ImageRepresentationGL* _moving;
            size_t _count;
        };

        /// \see AbstractProcessor::updateProperties
        void updateProperties(DataContainer& dc);

        void performOptimization(const ImageRepresentationGL* referenceImage, const ImageRepresentationGL* movingImage);

        void forceStop();

        float computeSimilarity(const ImageRepresentationGL* referenceImage, const ImageRepresentationGL* movingImage, const tgt::vec3& translation, const tgt::vec3& rotation);

        void generateDifferenceImage(DataContainer* dc, const ImageRepresentationGL* referenceImage, const ImageRepresentationGL* movingImage, const tgt::vec3& translation, const tgt::vec3& rotation);

        static tgt::mat4 euleranglesToMat4(const tgt::vec3& eulerAngles);

        static double optimizerFunc(const std::vector<double>& x, std::vector<double>& grad, void* my_func_data);

        IVec2Property p_viewportSize;

        tgt::Shader* _costFunctionShader;                           ///< Shader for slice rendering
        tgt::Shader* _differenceShader;                 ///< Shader for computing the difference image
        GlReduction* _glr;
        nlopt::opt* _opt;

        static const std::string loggerCat_;
    };

}

#endif // SIMILARITYMEASURE_H__
