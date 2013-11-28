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

#ifndef NLOPTREGISTRATION_H__
#define NLOPTREGISTRATION_H__

#include "core/datastructures/imagerepresentationlocal.h"
#include "core/eventhandlers/mwheeltonumericpropertyeventlistener.h"
#include "core/eventhandlers/transfuncwindowingeventlistener.h"
#include "core/pipeline/autoevaluationpipeline.h"
#include "modules/io/processors/mhdimagereader.h"
#include "modules/vis/processors/volumeexplorer.h"
#include "modules/preprocessing/processors/gradientvolumegenerator.h"
#include "modules/preprocessing/processors/lhhistogram.h"

#include "modules/registration/processors/similaritymeasure.h"

#include <nlopt.hpp>

namespace campvis {
    class NloptRegistration : public AutoEvaluationPipeline {
    public:
        /**
         * Creates a AutoEvaluationPipeline.
         */
        NloptRegistration(DataContainer* dc);

        /**
         * Virtual Destructor
         **/
        virtual ~NloptRegistration();

        /// \see AutoEvaluationPipeline::init()
        virtual void init();
        /// \see AutoEvaluationPipeline::deinit()
        virtual void deinit();

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const { return getId(); };
        static const std::string getId() { return "NloptRegistration"; };

        GenericOptionProperty<nlopt::algorithm> p_optimizer;    ///< Optimizer Algorithm
        BoolProperty p_liveUpdate;                              ///< Live Update of the difference image
        ButtonProperty p_performOptimization;                   ///< Start Optimization
        ButtonProperty p_forceStop;                             ///< Stop Optimization

        FloatProperty p_translationStepSize;                    ///< Initial Step Size for Translation
        FloatProperty p_rotationStepSize;                       ///< Initial Step Size for Rotation

    protected:
        /// Auxiliary data structure for nlopt
        struct MyFuncData_t {
            NloptRegistration* _object;
            const ImageRepresentationGL* _reference;
            const ImageRepresentationGL* _moving;
            size_t _count;
        };

        /**
         * Slot getting called when one of the observed processors got validated.
         * Updates the camera properties, when the input image has changed.
         * \param   processor   The processor that emitted the signal
         */
        virtual void onProcessorValidated(AbstractProcessor* processor);

        /**
         * Callback method called from p_performOptimization.
         * (Does not need an OpenGL context)
         */
        void onPerformOptimizationClicked();
        
        /**
         * Perform optimization to register \a movingImage to \a referenceImage.
         * \note    Needs to be called from a valid OpenGL context!
         */
        void performOptimization();

        /**
         * Free function to be called by nlopt optimizer computing the similarity.
         * \note    Needs to be called from a valid OpenGL context!
         * \param   x               Optimization vector
         * \param   grad            Gradient vector (currently ignored!)
         * \param   my_func_data    Auxiliary data structure0
         * \return  Result of the cost function.
         */
        static double optimizerFunc(const std::vector<double>& x, std::vector<double>& grad, void* my_func_data);

        /**
         * Stop the Optimization process.
         */
        void forceStop();


        MhdImageReader _referenceReader;
        MhdImageReader _movingReader;
        SimilarityMeasure _sm;
        VolumeExplorer _ve;
        
        nlopt::opt* _opt;                               ///< Pointer to nlopt Optimizer object
    };

}

#endif // NLOPTREGISTRATION_H__
