// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "tensoranalyzer.h"

#include "tgt/tgt_math.h"
#include "tgt/logmanager.h"

#include <Eigen/Eigenvalues>
#include <tbb/tbb.h>
#include <tbb/atomic.h>

#include "core/datastructures/imagedata.h"


namespace campvis {

    static const GenericOption<TensorAnalyzer::DegeneratedEvHandling> handlingModes[4] = {
        GenericOption<TensorAnalyzer::DegeneratedEvHandling>("none", "None", TensorAnalyzer::NONE),
        GenericOption<TensorAnalyzer::DegeneratedEvHandling>("mask", "Mask", TensorAnalyzer::MASK),
        GenericOption<TensorAnalyzer::DegeneratedEvHandling>("invert", "Invert", TensorAnalyzer::INVERT),
        GenericOption<TensorAnalyzer::DegeneratedEvHandling>("shift", "Shift", TensorAnalyzer::SHIFT)
    };

    const std::string TensorAnalyzer::loggerCat_ = "CAMPVis.modules.classification.TensorAnalyzer";

    TensorAnalyzer::TensorAnalyzer()
        : AbstractProcessor()
        , p_inputImage("InputImage", "Input Tensor Image", "tensors", DataNameProperty::READ, AbstractProcessor::INVALID_RESULT | EIGENSYSTEM_INVALID)
        , p_evalsImage("EvalsImage", "Output Eigenvalues Image", "eigenvalues", DataNameProperty::WRITE)
        , p_evecsImage("EvecsImage", "Output Eigenvectors Image", "eigenvectors", DataNameProperty::WRITE)
        , p_degeneratedHandling("DegeneratedHandling", "Handling of Degenerated Tensors", handlingModes, 4)
        , p_maskMixedTensors("MaskMixedTensors", "Mask Mixed Tensors", true)
        , _eigenvalues(0)
        , _eigenvectors(0)
    {
        addProperty(&p_inputImage);
        addProperty(&p_evalsImage);
        addProperty(&p_evecsImage);
        addProperty(&p_degeneratedHandling);
        addProperty(&p_maskMixedTensors);
    }

    TensorAnalyzer::~TensorAnalyzer() {

    }

    void TensorAnalyzer::process(DataContainer& data) {
        if (getInvalidationLevel() & EIGENSYSTEM_INVALID) {
            computeEigensystem(data);
        }

        if (_eigenvalues.getData() != 0 && _eigenvectors.getData() != 0) {

        }
        else {
            LERROR("Could not compute Eigensystem");
        }
        
        validate(INVALID_RESULT);
    }

    void TensorAnalyzer::computeEigensystem(DataContainer& data) {
        // get input tensor image
        GenericImageRepresentationLocal<float, 6>::ScopedRepresentation input(data, p_inputImage.getValue());

        if (input != 0) {
            // create output images for eigenvalues (stored as vec3) and eigenvectors (stored as mat3)
            ImageData* evals = new ImageData(input->getDimensionality(), input->getSize(), 3);
            GenericImageRepresentationLocal<float, 3>* evalRep = GenericImageRepresentationLocal<float, 3>::create(evals, 0);

            ImageData* evecs = new ImageData(input->getDimensionality(), input->getSize(), 9);
            GenericImageRepresentationLocal<float, 9>* evecRep = GenericImageRepresentationLocal<float, 9>::create(evals, 0);

            tbb::atomic<size_t> countDiscarded;
            countDiscarded = 0;
            DegeneratedEvHandling evh = p_degeneratedHandling.getOptionValue();

            // perform eigen decomposition in parallel
            tbb::parallel_for(tbb::blocked_range<size_t>(0, input->getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    const Tensor2<float>& t = input->getElement(i);

                    if (t.Dxx == 0 && t.Dxy == 0 && t.Dxz == 0 && t.Dyy == 0 && t.Dyz == 0 && t.Dzz == 0) {
                        evalRep->setElement(i, tgt::vec3::zero);
                        evecRep->setElement(i, tgt::mat3::zero);
                        continue;
                    }

                    // use Eigen library for eigensystem calculation
                    Eigen::Matrix3f tensorMatrix;
                    tensorMatrix << t.Dxx, t.Dxy, t.Dxz, t.Dxy, t.Dyy, t.Dyz, t.Dxz, t.Dyz, t.Dzz;

                    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> solver(tensorMatrix);

                    // gather data from eigen library
                    const Eigen::Matrix<float, 3, 1>& values = solver.eigenvalues();
                    const Eigen::Matrix3f& vectors = solver.eigenvectors();

                    // kill NaN values
                    if (tgt::isNaN(values(0)) || tgt::isNaN(values(1)) || tgt::isNaN(values(2))) {
                        evalRep->setElement(i, tgt::vec3::zero);
                        evecRep->setElement(i, tgt::mat3::zero);
                        continue;
                    }

                    // determine order of eigenvalues (i.e. sort'em)
                    int high, mid, low;
                    bool negative = false;
                    if (values(0) > values(1)) {
                        if (values(1) > values(2)) {
                            high = 0; mid = 1; low = 2;
                        }
                        else if (values(0) > values(2)) {
                            high = 0; mid = 2; low = 1;
                        }
                        else {
                            high = 2; mid = 0; low = 1;
                        }
                    }
                    else {
                        if (values(0) > values(2)) {
                            high = 1; mid = 0; low = 2;
                        }
                        else if (values(1) > values(2)) {
                            high = 1; mid = 2; low = 0;
                        }
                        else {
                            high = 2; mid = 1; low = 0;
                        }
                    }

                    // perform handling of degenerated tensors (i.e. negative/mixed eigenvalues):
                    if (values(0) < 0 && values(1) < 0 && values(2) < 0) {
                        negative = true;
                        if (evh == INVERT)
                            std::swap(high, low);
                    }
                    else if (p_maskMixedTensors.getValue() && (values(0) < 0 || values(1) < 0 || values(2) < 0)) {
                        // We assume that either all eigenvalues are positive or all eigenvalues are negative.
                        // If we encounter both positive and negative eigenvalues this must be due to severe noise
                        // (e.g. area outside brain) so it is reasonable to discard these voxels.
                        ++countDiscarded;
                        evalRep->setElement(i, tgt::vec3::zero);
                        evecRep->setElement(i, tgt::mat3::zero);
                        continue;
                    }

                    if (negative && evh == MASK) {
                        evalRep->setElement(i, tgt::vec3::zero);
                        evecRep->setElement(i, tgt::mat3::zero);
                    }
                    else {
                        evalRep->setElement(i, tgt::vec3(values(high), values(mid), values(low)));
                        evecRep->setElement(i, tgt::mat3(vectors(0, high), vectors(1, high), vectors(2, high),
                            vectors(0, mid),  vectors(1, mid),  vectors(2, mid),
                            vectors(0, low),  vectors(1, low),  vectors(2, low)));

                        if (negative && evh == SHIFT) {
                            evalRep->getElement(i) -= (evalRep->getElement(i).x + evalRep->getElement(i).z);
                        }
                    }
                }
            });

            // write results to DataContainer and also cache them in local members
            _eigenvalues = data.addData(p_evalsImage.getValue(), evals);
            _eigenvectors = data.addData(p_evecsImage.getValue(), evecs);
        }
        else {
            LDEBUG("No suitable input image found.");
        }

        validate(EIGENSYSTEM_INVALID);
    }

}
