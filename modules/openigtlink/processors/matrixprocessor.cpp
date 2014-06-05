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

#include "matrixprocessor.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/tools/stringutils.h"

#include "../datastructures/transformdata.h"

namespace campvis {
    const std::string MatrixProcessor::loggerCat_ = "CAMPVis.modules.core.MatrixProcessor";

    GenericOption<std::string> typeOptions[2] = {
        GenericOption<std::string>("fixed", "Fixed Matrix"),
        GenericOption<std::string>("data", "Matrix from Data Container")
    };

    MatrixProcessor::MatrixProcessor()
        : AbstractProcessor()
        , p_matrixAType("MatrixA_Type", "Matrix A Source", typeOptions, 2)
        , p_matrixAID("MatrixA_ID", "Matrix A", "matrixA", DataNameProperty::READ)
        , p_matrixAString("MatrixA_String", "Matrix A String", "identity")
        , p_matrixAModifiers("MatrixAModifiers", "Matrix A Modifiers")
        , p_matrixBType("MatrixB_Type", "Matrix B Source", typeOptions, 2)
        , p_matrixBID("MatrixB_ID", "Matrix B", "matrixB", DataNameProperty::READ)
        , p_matrixBString("MatrixB_String", "Matrix B String", "identity")
        , p_matrixBModifiers("MatrixBModifiers", "Matrix B Modifiers")
        , p_targetMatrixID("TargetMatrixID", "Target Matrix ID", "ProbeToReference", DataNameProperty::WRITE)
        , _lastdc(nullptr)
    {
        addProperty(p_matrixAType, INVALID_PROPERTIES | INVALID_RESULT);
        addProperty(p_matrixAID, INVALID_RESULT);
        addProperty(p_matrixAString, INVALID_RESULT);
        addProperty(p_matrixAModifiers, INVALID_RESULT);

        addProperty(p_matrixBType, INVALID_PROPERTIES | INVALID_RESULT);
        addProperty(p_matrixBID, INVALID_RESULT);
        addProperty(p_matrixBString, INVALID_RESULT);
        addProperty(p_matrixBModifiers, INVALID_RESULT);

        addProperty(p_targetMatrixID, INVALID_RESULT);

        invalidate(INVALID_PROPERTIES);
    }

    MatrixProcessor::~MatrixProcessor() {
        if (_lastdc)
            _lastdc->s_dataAdded.disconnect(this);
    }

    void MatrixProcessor::init() {

    }

    void MatrixProcessor::deinit() {

    }

    void MatrixProcessor::updateResult(DataContainer& data) {
#ifdef MATRIX_PROCESSOR_DEBUGGING
        LINFO("Updating Result");
#endif

        if (&data != _lastdc) {
            if (_lastdc) {
                _lastdc->s_dataAdded.disconnect(this);
            }

            data.s_dataAdded.connect(this, &MatrixProcessor::DataContainerDataAdded);
            _lastdc = &data;
        }

        tgt::mat4 matA = tgt::mat4::createIdentity();
        if(p_matrixAType.getOptionValue() == "fixed")
            matA = processMatrixString(p_matrixAString.getValue(), data);
        else {
            ScopedTypedData<TransformData> td(data, p_matrixAID.getValue());
            if (td != 0) matA = td->getTransform();
        }
        
        tgt::mat4 matB = tgt::mat4::createIdentity();
        if (p_matrixBType.getOptionValue() == "fixed")
            matB = processMatrixString(p_matrixBString.getValue(), data);
        else {
            ScopedTypedData<TransformData> td(data, p_matrixBID.getValue());
            if (td != 0) matB = td->getTransform();
        }

        tgt::mat4 matAProcessed = processModifierString(matA, p_matrixAModifiers.getValue());
        tgt::mat4 matBProcessed = processModifierString(matB, p_matrixBModifiers.getValue());


        tgt::mat4 result = matAProcessed * matBProcessed;

#ifdef MATRIX_PROCESSOR_DEBUGGING
        LDEBUG("Matrix A: " << std::endl << matA);
        LDEBUG("Matrix A':" << std::endl << matAProcessed);
        LDEBUG("Matrix B: " << std::endl << matB);
        LDEBUG("Matrix B':" << std::endl << matBProcessed);
        LDEBUG("Result Matrix: " << std::endl << result);
        LDEBUG(std::endl);
#endif

        TransformData * td = new TransformData(result);

        data.addData(p_targetMatrixID.getValue(), td);

        validate(INVALID_RESULT);
    }

    void MatrixProcessor::updateProperties(DataContainer& dataContainer)
    {
#ifdef MATRIX_PROCESSOR_DEBUGGING
        LINFO("Updating Properties");
#endif
        if (p_matrixAType.getOptionValue() == "fixed") {
            p_matrixAID.setVisible(false);
            p_matrixAString.setVisible(true);
        }
        else {
            p_matrixAID.setVisible(true);
            p_matrixAString.setVisible(false);
        }

        if (p_matrixBType.getOptionValue() == "fixed") {
            p_matrixBID.setVisible(false);
            p_matrixBString.setVisible(true);
        }
        else {
            p_matrixBID.setVisible(true);
            p_matrixBString.setVisible(false);
        }

        validate(INVALID_PROPERTIES);
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
            ++pos;
        }
        return result;
    }

    void MatrixProcessor::DataContainerDataAdded(const std::string &name, const DataHandle &data)
    {
        if (name == p_matrixAID.getValue() || name == p_matrixBID.getValue())
            invalidate(INVALID_RESULT);
    }
}