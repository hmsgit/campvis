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

#ifndef MATRIXPROCESSOR_H__
#define MATRIXPROCESSOR_H__


#include <string>

#include <tgt/matrix.h>

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/buttonproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/stringproperty.h"

namespace campvis {
    /**
     * Experimental demo implementation how to receive MHD files via CAMPCom, convert it to
     * CAMPVis ImageData and store it into the DataContainer.
     */
    class MatrixProcessor : public AbstractProcessor {
    public:
        /**
         * Constructs a new CampcomMhdReceiver Processor
         **/
        MatrixProcessor();

        /**
         * Destructor
         **/
        virtual ~MatrixProcessor();

        /// \see AbstractProcessor::init()
        virtual void init();
        /// \see AbstractProcessor::deinit()
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "MatrixProcessor"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Matrix Processor to process/combine one or two matrices and write the result into the data container"; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Jakob Weiss <weissj@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        StringProperty p_matrixA;           ///< first Matrix input for the computation. \see MatrixProcessor::processMatrixString()
		StringProperty p_matrixB;			///< second Matrix input for the computation. \see MatrixProcessor::processMatrixString()

		StringProperty p_matrixAModifiers;  ///< modifier string to be applied to matrix A. \see MatrixProcessor::processModifiers()
		StringProperty p_matrixBModifiers;  ///< modifier string to be applied to matrix B. \see MatrixProcessor::processModifiers()
       
        DataNameProperty p_targetMatrixID;   ///< image ID for read image


    protected:
        /// \see AbstractProcessor::updateResult()
        virtual void updateResult(DataContainer& dataContainer);

		tgt::mat4 processModifierString(tgt::mat4 matrix, std::string modifiers);

		tgt::mat4 processMatrixString(std::string matrixString, DataContainer& data);

        static const std::string loggerCat_;
    };

}

#endif // MATRIXPROCESSOR_H__
