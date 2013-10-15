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

#ifndef CONFIDENCEMAPGENERATOR_H__
#define CONFIDENCEMAPGENERATOR_H__

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/optionproperty.h"

namespace campvis {
    /**
     * Creates Confidence Maps for Ultrasound Images.
     * TODO: Clean up pre-MICCAI mess!
     */
    class ConfidenceMapGenerator : public AbstractProcessor {
    public:
        /**
         * Constructs a new ConfidenceMapGenerator Processor
         **/
        ConfidenceMapGenerator();

        /**
         * Destructor
         **/
        virtual ~ConfidenceMapGenerator();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "ConfidenceMapGenerator"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Creates Confidence Maps for Ultrasound Images."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual const ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        virtual void process(DataContainer& data);

        DataNameProperty p_sourceImageID;   ///< ID for input image
        DataNameProperty p_targetImageID;   ///< ID for output confidence map image

        FloatProperty p_alpha;                          ///< Alpha Parameter
        FloatProperty p_beta;                           ///< Beta Parameter
        FloatProperty p_gamma;                          ///< Gamma Parameter
        BoolProperty p_normalizeValues;                 ///< Flag whether to normalize the values before computation
        GenericOptionProperty<std::string> p_solver;    ///< Solver to use
        IntProperty p_numSteps;

        BoolProperty p_curvilinear;
        Vec2Property p_origin;
        Vec2Property p_angles;
        Vec2Property p_lengths;


    protected:

        static const std::string loggerCat_;
    };

}

#endif // CONFIDENCEMAPGENERATOR_H__
