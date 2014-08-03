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

//#define MATRIX_PROCESSOR_DEBUGGING

#include <string>
#include <set>

#include <tgt/matrix.h>

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/allproperties.h"

namespace campvis {
    /**
     * Matrix processor to perform some basic matrix arithmatic like combining two matrices.
	 *
	 * Takes two matrices as an input either from a string or from the data container 
	 * (see \a processMatrixString()), preprocesses them according to the specified modifiers (see \a processModiferString())
	 * and puts the result of multiplying A*B into the data container as a \a TransformData entry.
	 *
     * As an alternative, the "Parser Mode" is offered. This offers a possibility to evaluate more complicated formulas
     * without the need for multiple MatrixProcessor instances. For details on the syntax, see \a parseString(). The parsing is 
     * slower than the "basic" mode, but for most applications this performance hit should not be a problem. 
     * Performance could be reduced in a future iteration by pre-processing the input string instead of parsing it on the fly in each update call. 
     * Please note that this features is EXPERIMENTAL and has not been extensively tested.
     * An additional feature of the Parser Mode is the option to export a matrix through the \a m_cameraProperty field. Please
     * make sure that the m_cameraProperty is initialized and updated correctly regarding viewport changes (for example by setting
     * the property as a target for a TrackballEventHandler) as this feature only modifies the position, lookat and up vector of the
     * camera.
     *
	 * Example use case: OpenIGTLink client outputs matrices TrackerToReference and TrackerToProbe. Configure
	 * matrixA as "TrackerToProbe" with modifier "I" and matrixB as "TrackerToReference" with empty modifier
	 * to compute the "ProbeToReference" matrix. If an additional calibration matrix is needed, this can be achieved by
	 * adding a new MatrixProcessor that multiplies a hardcoded calibration matrix to the result or the inputs.
     */
    class MatrixProcessor : public AbstractProcessor {
    public:
        enum SourceType {
            FIXED = 0,
            DATA = 1
        };

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
        virtual const std::string getDescription() const { return "Matrix Processor to process/combine matrices and write the result into the data container"; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Jakob Weiss <weissj@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        BoolProperty p_parserMode; ///< toggle parsing mode. if enabled, a formula must be specified that should be parsed
        StringProperty p_parserInputString; ///< formula to be parsed when parsing mode is enabled. \see MatrixProcessor::parseString()

        GenericOptionProperty<std::string> p_matrixAType;
        DataNameProperty p_matrixAID;           ///< first Matrix input for the computation. \see MatrixProcessor::processMatrixString()
        StringProperty p_matrixAString;
        StringProperty p_matrixAModifiers;  ///< modifier string to be applied to matrix A. \see MatrixProcessor::processModifierString()
        

        GenericOptionProperty<std::string> p_matrixBType;
        DataNameProperty p_matrixBID;           ///< first Matrix input for the computation. \see MatrixProcessor::processMatrixString()
        StringProperty p_matrixBString;
        StringProperty p_matrixBModifiers;  ///< modifier string to be applied to matrix A. \see MatrixProcessor::processModifierString()

        DataNameProperty p_targetMatrixID;   ///< name for the output matrix

        CameraProperty p_cameraProperty;

		void DataContainerDataAdded(const std::string& name, DataHandle data);

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
         * \param localDefs map of local definitions. if supplied, matrix names will be looked for in this
         *    map prior to a datacontainer lookup
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
		 *  - if none of the above cases apply, the name is assumed to be a name of a data handle in the supplied data container
		 *    or the localDefs map, containing an entry of type \a TransformData
		 */
        tgt::mat4 processMatrixString(std::string matrixString, DataContainer& data, std::map<std::string, tgt::mat4> *localDefs = nullptr);


        /**
        * Parses and interprets a formula input string.
        * The string has to follow a given syntax to be parsed successfully:
        *  - whitespaces are not removed or skipped. if they appear in the wrong places the parsing will fail!
        *  - multiple computations are separated by a colon ';'
        *  - each computation is of the form <ResultName>=<Formula>
        *  - <Formula> is a combination of matrix multiplications: <MatrixId>*<MatrixId>*...
        *  - <MatrixId>s are always of the form '[<MatrixString>]_<Modifiers>
        *  - <MatrixString is a string parseable by \a MatrixProcessor::processMatrixString()
        *  - <Modifiers> is a string of modifiers as evaluated by \a MatrixProcessor::processModifierString()
        *  
        * Notes: 
        *  - Results of a previous computation can be reused in all following computations
        *  - If a result name starts with an underscore '_', the result will not be written to the data
        *    container, but it is still possible to use it in subsequent calulations
        *
        * Example:
        *  _ProbeToReference=[ReferenceToTracker]_I*[ProbeToTracker];_TipToProbe=[0.13758 0.0266467 0.00606382 -310.999 0.00447841 0.00887565 -0.137823 -18.5525 -0.0272137 0.133125 0.00797508 -105.741 0 0 0 1];TipToReference=[_ProbeToReference]*[_TipToProbe]
        *
        *  This will assume two matrices "ProbeToTracker" and "ReferenceToTracker" in the data container and use two
        *  intermediate results _ProbeToReference and _TipToProbe that are not pushed to the data container to compute the
        *  final result "TipToReference" that is then pushed to the data container
        */
        void parseString(const std::string & parserInput, DataContainer & dc);

        static const std::string loggerCat_;

    private:
        DataContainer * _lastdc;
        std::set<std::string> _dataDependencies; ///< set of data container element names that the current formula depends on
    };

}

#endif // MATRIXPROCESSOR_H__
