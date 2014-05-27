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
     * Matrix processor to perform some basic matrix arithmatic like combining two matrices.
	 *
	 * Takes two matrices as an input either from a string or from the data container 
	 * (see \a processMatrixString()), preprocesses them according to the specified modifiers (see \a processModiferString())
	 * and puts the result of multiplying A*B into the data container as a \a TransformData entry.
	 *
	 * Example use case: OpenIGTLink client outputs matrices TrackerToReference and TrackerToProbe. Configure
	 * matrixA as "TrackerToProbe" with modifier "I" and matrixB as "TrackerToReference" with empty modifier
	 * to compute the "ProbeToReference" matrix. If an additional calibration matrix is needed, this can be achieved by
	 * adding a new MatrixProcessor that multiplies a hardcoded calibration matrix to the result or the inputs.
     */
    class MatrixProcessor : public AbstractProcessor {
    public:
        /// Constructor
        MatrixProcessor();

        /// Destructor
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

		StringProperty p_matrixAModifiers;  ///< modifier string to be applied to matrix A. \see MatrixProcessor::processModifierString()
		StringProperty p_matrixBModifiers;  ///< modifier string to be applied to matrix B. \see MatrixProcessor::processModifierString()
       
        DataNameProperty p_targetMatrixID;   ///< image ID for read image


		void DataContainerDataAdded(const std::string& name, const DataHandle& data);

    protected:
        /// \see AbstractProcessor::updateResult()
        virtual void updateResult(DataContainer& dataContainer);

		/// \see AbstractProcessor::updateProperties()
		virtual void updateProperties(DataContainer& dataContainer);

		/**
		 * Processes a modifier string and returns the modified matrix.
		 *
		 * \param matrix the input matrix to be modified
		 * \param modifiers a string containing modifiers that will be applied to the matrix from left to right.
		 *		Possible Modifiers are:
		 *		 - _I_: invert matrix
		 *		 - _T_: transpose matrix
		 *		 - _r_: extract rotational part \see tgt::mat4::getRotationalPart()
		 *		 - _s_: extract scaling part \see tgt::mat4::getScalingPart()
		 *		 - _-_: negate componentwise
		 *
		 * i.e. a call with a modifier string "IT" will calculate the transpose of the inverse.
		 */
		tgt::mat4 processModifierString(tgt::mat4 matrix, std::string modifiers);

		/**
		 * Processes a matrix string and returns the resulting matrix.
		 *
		 * \param matrixString the matrix string to be parsed
		 * \param data the data container that is used to read data from
		 * 
		 * The matrix string can either be a name to a data handle or a string specifying a matrix directly.
		 * The string is split into tokens with space as a delimiter and is parsed according to the following rules:
		 *  - an empty string or "identity" creates an identity matrix
		 *  - if the string contains exactly 16 tokens, a direct matrix input is assumed: all
		 *    tokens are converted to floats and are used as coefficients. The parsing is done row-major: a
		 *    string "1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 creates a matrix with first row 1,2,3,4, second row 
		 *    5,6,7,8 and so on
		 *  - "rot <angle> <ax> <ay> <az>" creates a rotation matrix around axis (ax,ay,az) with specified angle in radians.
		 *    \see tgt::mat4::createRotation()
		 *  - "trans <tx> <ty> <tz>" creates a translation matrix with translation (tx,ty,tz)
		 *  - "scale <sx> [<sy> <sz>]" creates a scaling matrix. if only one coefficient is specified, a uniform scaling
		 *    is created, otherwise all three scaling factors are used.
		 *  - if any of the above fails, a warning is emitted and identity is returned
		 *  - if none of the above cases apply, the name is assumed to be a name of a data handle in the supplied data container,
		 *    containing an entry of type \a TransformData
		 */
		tgt::mat4 processMatrixString(std::string matrixString, DataContainer& data);

		DataContainer * lastdc_;

        static const std::string loggerCat_;
    };

}

#endif // MATRIXPROCESSOR_H__
