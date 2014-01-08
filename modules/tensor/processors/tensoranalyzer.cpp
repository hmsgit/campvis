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
#include "core/tools/stringutils.h"

namespace campvis {

    static const GenericOption<TensorAnalyzer::DegeneratedEvHandling> handlingModes[4] = {
        GenericOption<TensorAnalyzer::DegeneratedEvHandling>("none", "None", TensorAnalyzer::NONE),
        GenericOption<TensorAnalyzer::DegeneratedEvHandling>("mask", "Mask", TensorAnalyzer::MASK),
        GenericOption<TensorAnalyzer::DegeneratedEvHandling>("invert", "Invert", TensorAnalyzer::INVERT),
        GenericOption<TensorAnalyzer::DegeneratedEvHandling>("shift", "Shift", TensorAnalyzer::SHIFT)
    };

    static const GenericOption<std::string> measurementOptions[15] = {
        GenericOption<std::string>("Disabled", "Disabled"),
        GenericOption<std::string>("EigenValue1", "Eigenvalue 1"),
        GenericOption<std::string>("EigenValue2", "Eigenvalue 2"),
        GenericOption<std::string>("EigenValue3", "Eigenvalue 3"),
        GenericOption<std::string>("MainEigenvector", "Main Eigenvector"),
        GenericOption<std::string>("VolumeRatio", "Volume Ratio"),
        GenericOption<std::string>("FractionalAnisotropy", "Fractional Anisotropy"),
        GenericOption<std::string>("RelativeAnisotropy", "Relative Anisotropy"),
        GenericOption<std::string>("MeanDiffusivity", "Mean Diffusivity"),
        GenericOption<std::string>("Trace", "Trace"),
        GenericOption<std::string>("AxialDiffusivity", "Axial Diffusivity"),
        GenericOption<std::string>("RadialDiffusivity", "Radial Diffusivity"),
        GenericOption<std::string>("LinearAnisotropy", "Linear Anisotropy"),
        GenericOption<std::string>("PlanarAnisotropy", "Planar Anisotropy"),
        GenericOption<std::string>("Isotropy", "Isotropy")
    };

    const std::string TensorAnalyzer::loggerCat_ = "CAMPVis.modules.classification.TensorAnalyzer";

    TensorAnalyzer::OutputPropertyPair::OutputPropertyPair(size_t index) 
        : _imageId("OutputId" + StringUtils::toString(index), "Output " + StringUtils::toString(index) + " Image", "TensorAnalyzer.output" + StringUtils::toString(index), DataNameProperty::WRITE)
        , _imageType("OutputType" + StringUtils::toString(index), "Output " + StringUtils::toString(index) + " Image Type", measurementOptions, 15) 
    {}


    TensorAnalyzer::TensorAnalyzer()
        : AbstractProcessor()
        , p_inputImage("InputImage", "Input Tensor Image", "tensors", DataNameProperty::READ, AbstractProcessor::INVALID_RESULT | EIGENSYSTEM_INVALID)
        , p_evalsImage("EvalsImage", "Output Eigenvalues Image", "TensorAnalyzer.eigenvalues", DataNameProperty::WRITE)
        , p_evecsImage("EvecsImage", "Output Eigenvectors Image", "TensorAnalyzer.eigenvectors", DataNameProperty::WRITE)
        , p_degeneratedHandling("DegeneratedHandling", "Handling of Degenerated Tensors", handlingModes, 4)
        , p_maskMixedTensors("MaskMixedTensors", "Mask Mixed Tensors", true)
        , p_addOutputButton("AddOutputButton", "Add Output", AbstractProcessor::VALID)
        , _eigenvalues(0)
        , _eigenvectors(0)
    {
        addProperty(&p_inputImage);
        addProperty(&p_evalsImage);
        addProperty(&p_evecsImage);
        addProperty(&p_degeneratedHandling);
        addProperty(&p_maskMixedTensors);
        addProperty(&p_addOutputButton);
        addOutput();

        p_addOutputButton.s_clicked.connect(this, &TensorAnalyzer::addOutput);
    }

    TensorAnalyzer::~TensorAnalyzer() {

    }

    void TensorAnalyzer::updateResult(DataContainer& data) {
        if (getInvalidationLevel() & EIGENSYSTEM_INVALID) {
            computeEigensystem(data);
        }

        if (_eigenvalues.getData() != 0 && _eigenvectors.getData() != 0) {
            for (size_t i = 0; i < p_outputProperties.size(); ++i) {
                computeOutput(data, i);
            }
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
            GenericImageRepresentationLocal<float, 9>* evecRep = GenericImageRepresentationLocal<float, 9>::create(evecs, 0);

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

    void TensorAnalyzer::computeOutput(DataContainer& data, size_t index) {
        // sanity check
        if (index >= p_outputProperties.size()) {
            LERROR("Index out of bounds while computing output #" << index);
            return;
        }

        // gather eigensystem
        const GenericImageRepresentationLocal<float, 3>* evalRep = 0;
        const GenericImageRepresentationLocal<float, 9>* evecRep = 0;
        if (_eigenvalues.getData() != 0 && _eigenvectors.getData() != 0) {
            evalRep = static_cast<const ImageData*>(_eigenvalues.getData())->getRepresentation< GenericImageRepresentationLocal<float, 3> >(false);
            evecRep = static_cast<const ImageData*>(_eigenvectors.getData())->getRepresentation< GenericImageRepresentationLocal<float, 9> >(false);
        }
        if (evalRep == 0 || evecRep == 0) {
            LERROR("Could not compute output, no eigensystem present.");
            return;
        }

        OutputPropertyPair* opp = p_outputProperties[index];
        const std::string& type = opp->_imageType.getOptionValue();
        

        if (type == "Disabled") {
            return;
        }
        else if (type == "EigenValue1") {
            ImageData* id = new ImageData(evalRep->getDimensionality(), evalRep->getSize(), 1);
            GenericImageRepresentationLocal<float, 1>* output = GenericImageRepresentationLocal<float, 1>::create(id, 0);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, id->getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    const tgt::vec3& vals = evalRep->getElement(i);

                    float ev = vals.x;
                    if (ev == 0.0f || tgt::isNaN(ev))
                        output->setElement(i, 0.0f);
                    else
                        output->setElement(i, ev);
                }
            });
            data.addData(opp->_imageId.getValue(), id);
        }
        else if (type == "EigenValue2") {
            ImageData* id = new ImageData(evalRep->getDimensionality(), evalRep->getSize(), 1);
            GenericImageRepresentationLocal<float, 1>* output = GenericImageRepresentationLocal<float, 1>::create(id, 0);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, id->getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    const tgt::vec3& vals = evalRep->getElement(i);

                    float ev = vals.y;
                    if (ev == 0.0f || tgt::isNaN(ev))
                        output->setElement(i, 0.0f);
                    else
                        output->setElement(i, ev);
                }
            });
            data.addData(opp->_imageId.getValue(), id);
        }
        else if (type == "EigenValue3") {
            ImageData* id = new ImageData(evalRep->getDimensionality(), evalRep->getSize(), 1);
            GenericImageRepresentationLocal<float, 1>* output = GenericImageRepresentationLocal<float, 1>::create(id, 0);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, id->getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    const tgt::vec3& vals = evalRep->getElement(i);

                    float ev = vals.z;
                    if (ev == 0.0f || tgt::isNaN(ev))
                        output->setElement(i, 0.0f);
                    else
                        output->setElement(i, ev);
                }
            });
            data.addData(opp->_imageId.getValue(), id);
        }

        else if (type == "MainEigenvector") {
            ImageData* id = new ImageData(evalRep->getDimensionality(), evalRep->getSize(), 3);
            GenericImageRepresentationLocal<float, 3>* output = GenericImageRepresentationLocal<float, 3>::create(id, 0);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, id->getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    output->setElement(i, evecRep->getElement(i)[0]);
                }
            });
            data.addData(opp->_imageId.getValue(), id);
        }

        // = Anisotropy Measures: =========================================================================
        else if (type == "VolumeRatio") {
            ImageData* id = new ImageData(evalRep->getDimensionality(), evalRep->getSize(), 1);
            GenericImageRepresentationLocal<float, 1>* output = GenericImageRepresentationLocal<float, 1>::create(id, 0);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, id->getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    const tgt::vec3& vals = evalRep->getElement(i);

                    if (vals == tgt::vec3::zero || tgt::isNaN(vals))
                        output->getElement(i) = 0;
                    else
                        output->getElement(i) = (vals.x * vals.y * vals.z) / pow((vals.x + vals.y + vals.z)/3.f, 3);
                }
            });
            data.addData(opp->_imageId.getValue(), id);
        }
        else if (type == "FractionalAnisotropy") {
            ImageData* id = new ImageData(evalRep->getDimensionality(), evalRep->getSize(), 1);
            GenericImageRepresentationLocal<float, 1>* output = GenericImageRepresentationLocal<float, 1>::create(id, 0);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, id->getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    const tgt::vec3& vals = evalRep->getElement(i);
                    const float root = sqrt(.5f);

                    if (vals == tgt::vec3::zero || tgt::isNaN(vals))
                        output->getElement(i) = 0;
                    else
                        output->getElement(i) = root * sqrt((vals.x-vals.y)*(vals.x-vals.y) + (vals.y-vals.z)*(vals.y-vals.z) + (vals.z-vals.x)*(vals.z-vals.x)) / sqrt(vals.x*vals.x + vals.y*vals.y + vals.z*vals.z);
                }
            });
            data.addData(opp->_imageId.getValue(), id);
        }
        else if (type == "RelativeAnisotropy") {
            ImageData* id = new ImageData(evalRep->getDimensionality(), evalRep->getSize(), 1);
            GenericImageRepresentationLocal<float, 1>* output = GenericImageRepresentationLocal<float, 1>::create(id, 0);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, id->getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    const tgt::vec3& vals = evalRep->getElement(i);
                    const float root = sqrt(.5f);

                    if (vals == tgt::vec3::zero || tgt::isNaN(vals))
                        output->getElement(i) = 0;
                    else
                        output->getElement(i) = root * sqrt((vals.x-vals.y)*(vals.x-vals.y) + (vals.y-vals.z)*(vals.y-vals.z) + (vals.z-vals.x)*(vals.z-vals.x)) / (vals.x + vals.y + vals.z);
                }
            });
            data.addData(opp->_imageId.getValue(), id);
        }
        else if (type == "MeanDiffusivity") {
            ImageData* id = new ImageData(evalRep->getDimensionality(), evalRep->getSize(), 1);
            GenericImageRepresentationLocal<float, 1>* output = GenericImageRepresentationLocal<float, 1>::create(id, 0);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, id->getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    const tgt::vec3& vals = evalRep->getElement(i);

                    if (vals == tgt::vec3::zero || tgt::isNaN(vals))
                        output->getElement(i) = 0;
                    else
                        output->getElement(i) = (vals.x + vals.y + vals.z) / 3;
                }
            });
            data.addData(opp->_imageId.getValue(), id);
        }
        else if (type == "Trace") {
            ImageData* id = new ImageData(evalRep->getDimensionality(), evalRep->getSize(), 1);
            GenericImageRepresentationLocal<float, 1>* output = GenericImageRepresentationLocal<float, 1>::create(id, 0);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, id->getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    const tgt::vec3& vals = evalRep->getElement(i);

                    if (vals == tgt::vec3::zero || tgt::isNaN(vals))
                        output->getElement(i) = 0;
                    else
                        output->getElement(i) = vals.x + vals.y + vals.z;
                }
            });
            data.addData(opp->_imageId.getValue(), id);
        }
        else if (type == "AxialDiffusivity") {
            ImageData* id = new ImageData(evalRep->getDimensionality(), evalRep->getSize(), 1);
            GenericImageRepresentationLocal<float, 1>* output = GenericImageRepresentationLocal<float, 1>::create(id, 0);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, id->getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    const tgt::vec3& vals = evalRep->getElement(i);

                    if (vals == tgt::vec3::zero || tgt::isNaN(vals))
                        output->getElement(i) = 0;
                    else
                        output->getElement(i) = vals.x;
                }
            });
            data.addData(opp->_imageId.getValue(), id);
        }
        else if (type == "RadialDiffusivity") {
            ImageData* id = new ImageData(evalRep->getDimensionality(), evalRep->getSize(), 1);
            GenericImageRepresentationLocal<float, 1>* output = GenericImageRepresentationLocal<float, 1>::create(id, 0);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, id->getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    const tgt::vec3& vals = evalRep->getElement(i);

                    if (vals == tgt::vec3::zero || tgt::isNaN(vals))
                        output->getElement(i) = 0;
                    else
                        output->getElement(i) = (vals.y + vals.z) / 2;
                }
            });
            data.addData(opp->_imageId.getValue(), id);
        }
        else if (type == "LinearAnisotropy") {
            ImageData* id = new ImageData(evalRep->getDimensionality(), evalRep->getSize(), 1);
            GenericImageRepresentationLocal<float, 1>* output = GenericImageRepresentationLocal<float, 1>::create(id, 0);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, id->getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    const tgt::vec3& vals = evalRep->getElement(i);

                    if (vals == tgt::vec3::zero || tgt::isNaN(vals))
                        output->getElement(i) = 0;
                    else
                        output->getElement(i) = (vals.x - vals.y) / (vals.x + vals.y + vals.z);
                }
            });
            data.addData(opp->_imageId.getValue(), id);
        }
        else if (type == "PlanarAnisotropy") {
            ImageData* id = new ImageData(evalRep->getDimensionality(), evalRep->getSize(), 1);
            GenericImageRepresentationLocal<float, 1>* output = GenericImageRepresentationLocal<float, 1>::create(id, 0);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, id->getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    const tgt::vec3& vals = evalRep->getElement(i);

                    if (vals == tgt::vec3::zero || tgt::isNaN(vals))
                        output->getElement(i) = 0;
                    else
                        output->getElement(i) = 2.f*(vals.y - vals.z) / (vals.x + vals.y + vals.z);
                }
            });
            data.addData(opp->_imageId.getValue(), id);
        }
        else if (type == "Isotropy") {
            ImageData* id = new ImageData(evalRep->getDimensionality(), evalRep->getSize(), 1);
            GenericImageRepresentationLocal<float, 1>* output = GenericImageRepresentationLocal<float, 1>::create(id, 0);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, id->getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    const tgt::vec3& vals = evalRep->getElement(i);

                    if (vals == tgt::vec3::zero || tgt::isNaN(vals))
                        output->getElement(i) = 0;
                    else
                        output->getElement(i) = (3.f*vals.z) / (vals.x + vals.y + vals.z);
                }
            });
            data.addData(opp->_imageId.getValue(), id);
        }

    }

    void TensorAnalyzer::addOutput() {
        OutputPropertyPair* opp = new OutputPropertyPair(p_outputProperties.size() + 1);
        p_outputProperties.push_back(opp);
        addProperty(&opp->_imageId);
        addProperty(&opp->_imageType);
    }

    void TensorAnalyzer::deinit() {
        _eigenvectors = DataHandle(0);
        _eigenvalues = DataHandle(0);
        AbstractProcessor::deinit();
    }

}
