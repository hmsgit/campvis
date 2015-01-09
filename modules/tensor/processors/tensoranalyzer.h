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

#ifndef TENSORANALYZER_H__
#define TENSORANALYZER_H__

#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/buttonproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/optionproperty.h"

#include "core/datastructures/genericimagerepresentationlocal.h"

#include "modules/modulesapi.h"

namespace campvis {
    /**
     * Performs eigensystem decomposition of a tensor image and also computes different anisotropy measures.
     */
    class CAMPVIS_MODULES_API TensorAnalyzer : public AbstractProcessor {
    public:
        enum AdditionalInvalidationLevels {
            EIGENSYSTEM_INVALID = AbstractProcessor::FIRST_FREE_TO_USE_INVALIDATION_LEVEL
        };

        /// Eigenvalue handling of degenerated tensors (i.e. having (partially) negative eigenvalues)
        enum DegeneratedEvHandling {
            NONE, MASK, INVERT, SHIFT
        };

        // Pair of DataNameProperty for output image ID and OptionProperty for image type
        struct OutputPropertyPair {
            OutputPropertyPair(size_t index);;

            DataNameProperty _imageId;
            GenericOptionProperty<std::string> _imageType;
        };

        /**
         * Constructs a new TensorAnalyzer Processor
         **/
        TensorAnalyzer();

        /**
         * Destructor
         **/
        virtual ~TensorAnalyzer();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "TensorAnalyzer"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Performs eigensystem decomposition of a tensor image and also computes different anisotropy measures."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };
        /// \see AbstractProcessor::deinit()
        virtual void deinit();

        /**
         * Adds another output for this processor (i.e. adds another OutputPropertyPair).
         */
        void addOutput();

        DataNameProperty p_inputImage;   ///< ID for input volume
        DataNameProperty p_evalsImage;   ///< ID for output eigenvalue volume
        DataNameProperty p_evecsImage;   ///< ID for output eigenvector volume

        GenericOptionProperty<DegeneratedEvHandling> p_degeneratedHandling; ///< Handling of degenerated tensors
        BoolProperty p_maskMixedTensors;

        ButtonProperty p_addOutputButton;
        std::vector<OutputPropertyPair*> p_outputProperties;

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        /**
         * Computes the eigensystem for the given tensor image \a tensorImage.
         * Computed eigenvalues and eigenvectors are stored in this' class local members.
         * \param   data    DataContainer to work on.
         */
        void computeEigensystem(DataContainer& data);

        /**
         * Computes the derived measurement for output number \a index.
         * \param   data    DataContainer to store output image in.
         * \param   index   Index of output to compute.
         */
        void computeOutput(DataContainer& data, size_t index);

        DataHandle _eigenvalues;    ///< Current eigenvalues cached
        DataHandle _eigenvectors;   ///< Current eigenvectors cached

        static const std::string loggerCat_;
    };

}

#endif // TENSORANALYZER_H__
