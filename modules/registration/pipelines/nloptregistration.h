// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
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

#ifndef NLOPTREGISTRATION_H__
#define NLOPTREGISTRATION_H__

#include "core/datastructures/imagerepresentationlocal.h"
#include "core/eventhandlers/mwheeltonumericpropertyeventlistener.h"
#include "core/eventhandlers/transfuncwindowingeventlistener.h"
#include "core/pipeline/autoevaluationpipeline.h"

#include "modules/base/processors/lightsourceprovider.h"
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


        LightSourceProvider _lsp;
        MhdImageReader _referenceReader;
        MhdImageReader _movingReader;
        SimilarityMeasure _sm;
        VolumeExplorer _ve;
        
        nlopt::opt* _opt;                               ///< Pointer to nlopt Optimizer object
    };

}

#endif // NLOPTREGISTRATION_H__
