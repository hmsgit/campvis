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

#ifndef POINTPREDICATEEVALUATOR_H__
#define POINTPREDICATEEVALUATOR_H__

#include <string>

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/pipeline/visualizationprocessor.h"

#include "core/properties/cameraproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/optionproperty.h"
#include "core/properties/transferfunctionproperty.h"

#include "modules/advancedusvis/properties/pointpredicatehistogramproperty.h"

namespace tgt {
    class Shader;
}

namespace campvis {
    class ImageData;

    /**
     * Evaluates the Point Predicate Histogram and stores the results in a bitmask image.
     */
    class PointPredicateEvaluator : public VisualizationProcessor {
    public:
        /**
         * Constructs a new PointPredicateEvaluator Processor
         **/
        PointPredicateEvaluator(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~PointPredicateEvaluator();

        /// \see AbstractProcessor::init
        virtual void init();
        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "PointPredicateEvaluator"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Evaluates the Point Predicate Histogram and stores the results in a bitmask image."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_inputImage;          ///< Input image
        DataNameProperty p_inputLabels;         ///< Input label image
        DataNameProperty p_inputSnr;            ///< Input SNR image
        DataNameProperty p_inputVesselness;     ///< Input Vesselness image
        DataNameProperty p_inputConfidence;     ///< Input confidence image
            
        DataNameProperty p_outputImage;         ///< Output bitmask image

        CameraProperty p_camera;                        ///< Camera for silhouette view predicate
        PointPredicateHistogramProperty p_histogram;    ///< Point Predicate Histogram to evaluate

    protected:
        /// \see AbstractProcessor::updateResult()
        virtual void updateResult(DataContainer& dataContainer);
        /// adapts the range of the p_sliceNumber property to the image
        virtual void updateProperties(DataContainer dataContainer);
        /// \see AbstractProcessor::updateShader()
        virtual void updateShader();

        /// Callback called when predicate histogram configuration has changed, so that the GLSL header needs to be recompiled.
        void onHistogramHeaderChanged();

        /// Generates the GLSL header
        std::string generateHeader() const;

        tgt::Shader* _shader;                           ///< Shader for slice rendering

    private:
        static const std::string loggerCat_;
    };

}

#endif // POINTPREDICATEEVALUATOR_H__
