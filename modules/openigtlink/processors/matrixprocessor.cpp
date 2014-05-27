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

#define CAMPCOM_FAST_SERIALIZATION

#include "matrixprocessor.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/tools/stringutils.h"

#include "../datastructures/transformdata.h"

namespace campvis {
    const std::string MatrixProcessor::loggerCat_ = "CAMPVis.modules.core.MatrixProcessor";

	MatrixProcessor::MatrixProcessor()
		: AbstractProcessor()
		, p_matrixA("MatrixA", "Matrix A", "matrixA")
		, p_matrixB("MatrixB", "Matrix B", "matrixB")
		, p_matrixAModifiers("MatrixAModifiers", "Matrix A Modifiers")
		, p_matrixBModifiers("MatrixBModifiers", "Matrix B Modifiers")
		, p_targetMatrixID("TargetMatrixID", "Target Matrix ID", "result.matrix", DataNameProperty::WRITE)
    {

        addProperty(p_matrixA, VALID);
        addProperty(p_matrixAModifiers, VALID);
        addProperty(p_matrixB, VALID);
        addProperty(p_matrixBModifiers, VALID);
        addProperty(p_targetMatrixID, VALID);

    }

	MatrixProcessor::~MatrixProcessor() {

    }

	void MatrixProcessor::init() {
       
    }

	void MatrixProcessor::deinit() {
       
    }

	void MatrixProcessor::updateResult(DataContainer& data) {
		tgt::mat4 matA = processMatrixString(p_matrixA.getValue(), data);
		tgt::mat4 matB = processMatrixString(p_matrixB.getValue(), data);

		tgt::mat4 matAProcessed = processModifierString(matA, p_matrixAModifiers.getValue());
		tgt::mat4 matBProcessed = processModifierString(matB, p_matrixBModifiers.getValue());


		tgt::mat4 result = matAProcessed * matBProcessed;

		LDEBUG("Matrix A: " << std::endl << matA);
		LDEBUG("Matrix A':" << std::endl << matAProcessed);
		LDEBUG("Matrix B " << std::endl << matB);
		LDEBUG("Matrix B':" << std::endl << matBProcessed);
		LDEBUG("Result Matrix: " << std::endl << result);
		LDEBUG(std::endl);

		TransformData * td = new TransformData(result);

		data.addData(p_targetMatrixID.getValue(), td);

        validate(INVALID_RESULT);
    }

	tgt::mat4 MatrixProcessor::processMatrixString(std::string matrixString, DataContainer& data)
	{
		std::vector<std::string> tokens = StringUtils::split(matrixString, " ");

		if (tokens.size() == 0 || tokens[0] == "identity") {
			return tgt::mat4(tgt::mat4::identity);
		}
		// if we have exactly 16 tokens, we assume we have a matrix in numerical form
		else if (tokens.size() == 16) {
			tgt::mat4 mat;
			float * p = mat.elem;
			for (int i = 0; i < 16; i++) {
				*p = static_cast<float>(atof(tokens[i].c_str()));
				p++;
			}
			return mat;
		}
		// if the first token is "rot", we create an angle axis rotation matrix with the specified arguments
		else if (tokens[0] == "rot") {
			if (tokens.size() != 5) {
				LWARNING("Rotation matrix string does not have the correct number of arguments!");
				return tgt::mat4::createIdentity();
			}
			float angle;
			tgt::vec3 axis;
			angle = static_cast<float>(atof(tokens[1].c_str()));
			axis[0] = static_cast<float>(atof(tokens[2].c_str()));
			axis[1] = static_cast<float>(atof(tokens[3].c_str()));
			axis[2] = static_cast<float>(atof(tokens[4].c_str()));
			return tgt::mat4::createRotation(angle, axis);
		}
		else if (tokens[0] == "trans") {
			if (tokens.size() != 4) {
				LWARNING("Translation matrix string does not have the correct number of arguments!");
				return tgt::mat4::createIdentity();
			}

			tgt::vec3 translation;
			translation[0] = static_cast<float>(atof(tokens[1].c_str()));
			translation[1] = static_cast<float>(atof(tokens[2].c_str()));
			translation[2] = static_cast<float>(atof(tokens[3].c_str()));
			return tgt::mat4::createTranslation(translation);
		}
		else if (tokens[0] == "scale") {
			if (tokens.size() != 2 && tokens.size() != 4) {
				LWARNING("Scaling matrix string does not have the correct number of arguments!");
				return tgt::mat4::createIdentity();
			}

			tgt::vec3 scale;
			scale[0] = static_cast<float>(atof(tokens[1].c_str()));
			if (tokens.size() == 4) {
				scale[1] = static_cast<float>(atof(tokens[2].c_str()));
				scale[2] = static_cast<float>(atof(tokens[3].c_str()));
			}
			else {
				scale[1] = scale[2] = scale[0];
			}

			return tgt::mat4::createScale(scale);
		}
		// if we cannot find another pattern, we assume we have a data container ID
		else {
			ScopedTypedData<TransformData> td(data, matrixString);
			if (td == 0) {
				LWARNING("Data Container ID \"" << matrixString << "\" was not suitable as input Matrix");
				return tgt::mat4::createIdentity();
			}

			return td->getTransform();
		}

	}

	tgt::mat4 MatrixProcessor::processModifierString(tgt::mat4 matrix, std::string modifiers)
	{
		int pos = 0;
		tgt::mat4 result = matrix, tmp;

		while (pos < modifiers.size())
		{
			switch (modifiers[pos]) {
			case 'I':
				if (!result.invert(tmp)) {
					LWARNING("Matrix Inversion failed.");
				}
				else result = tmp;
				break;
			case 'T':
				result = tgt::transpose(result);
				break;
			case '-':
				result = tgt::mat4::zero - result;
				break;
			case 'r':
				result = result.getRotationalPart();
				break;
			case 's':
				result = tgt::mat4::createScale(result.getScalingPart());
				break;
			default:
				LWARNING("Ignoring unknown modifier: " << modifiers[pos]);
			}
		}
		return result;
	}
}