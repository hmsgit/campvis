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

#include "ivustcdemo.h"

#include "cgt/filesystem.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

#include "core/classification/simpletransferfunction.h"

#include "modules/ivus_tc/processors/ivustcsliceextractor.h"
#include "modules/ivus_tc/processors/ivustcraycaster.h"

namespace campvis {

    IvusTcDemo::IvusTcDemo(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , p_sourceDirectory("SoruceDirectory", "Source Directory", "D:/Medical Data/IVUS/H52 LAD1", StringProperty::DIRECTORY)
        , p_readImagesButton("ReadImagesButton", "Read Images")
        , p_predicateHistogram("PredicateHistogram", "Voxel Predicate Selection")
        , _lsp()
        , _imageReader(&_canvasSize)
        , _ve(&_canvasSize, new IvusTcSliceExtractor(nullptr), new IvusTcRaycaster(nullptr))
    {
        // Use AbstractPipeline overload to avoid automatic evaluation of _imageReader processor
//        AbstractPipeline::addProcessor(&_imageReader);

        addProcessor(&_lsp);
        addProcessor(&_ve);

        addProperty(p_sourceDirectory);
        addProperty(p_readImagesButton);
        addProperty(*(_ve.getSliceRenderer()->getProperty("MixWithIvus")));

        addEventListenerToBack(&_ve);

        p_predicateHistogram.getPredicateHistogram()->setPredicateFunctionArgumentString("in float ivus, in float cm, in vec4 tc, in float plaque");
    }

    IvusTcDemo::~IvusTcDemo() {
    }

    void IvusTcDemo::init() {
        AutoEvaluationPipeline::init();
        _imageReader.init();
        p_readImagesButton.s_clicked.connect(this, &IvusTcDemo::readAndProcessImages);

        _lsp.p_ambientColor.setValue(cgt::vec3(.75f));
        _lsp.p_shininess.setValue(8.f);

        _ve.p_inputVolume.setValue("image.ivus");
        _ve.p_outputImage.setValue("combine");
        _ve.getVolumeRenderer()->getRaycastingProcessor()->p_samplingRate.setValue(.5f);

        _renderTargetID.setValue("combine");
        
        _imageReader.p_fileExtension.setValue("bmp");
        _imageReader.p_imageSpacing.setValue(cgt::vec3(.1f, .1f, .6f));

        // initialize predicates with default config
        PointPredicateHistogramProperty* php = &p_predicateHistogram;
        if (php != nullptr) {
            PointPredicateHistogram* histogram = php->getPredicateHistogram();
            histogram->_glslModulationHackForIvus = true;

            AbstractPointPredicate* vpToAdd = 0;

            vpToAdd = new RangePointPredicate("ivus", "IvusIntensity", "IVUS Intensity");
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setValue(cgt::vec2(0.f, 1.f));
            histogram->addPredicate(vpToAdd);

//             vpToAdd = new RangePointPredicate("cm", "ConfidenceMap", "Confidence");
//             static_cast<RangePointPredicate*>(vpToAdd)->p_range.setValue(cgt::vec2(0.25f, 1.f));
//             histogram->addPredicate(vpToAdd);
// 
            vpToAdd = new RangePointPredicate("tc.r", "Calcified", "Calcified Tissue");
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setValue(cgt::vec2(0.15f, 1.f));
            vpToAdd->p_color.setValue(cgt::vec2(0.667f, 1.f));
            histogram->addPredicate(vpToAdd);

            vpToAdd = new RangePointPredicate("tc.g", "Fibrotic", "Fibrotic Tissue");
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setValue(cgt::vec2(0.15f, 1.f));
            vpToAdd->p_color.setValue(cgt::vec2(0.165f, 1.f));
            histogram->addPredicate(vpToAdd);

            vpToAdd = new RangePointPredicate("tc.b", "Lipidic", "Lipidic Tissue");
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setValue(cgt::vec2(0.15f, 1.f));
            vpToAdd->p_color.setValue(cgt::vec2(0.9f, 1.f));
            histogram->addPredicate(vpToAdd);

            vpToAdd = new RangePointPredicate("tc.a", "Necrotic", "Necrotic Tissue");
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setValue(cgt::vec2(0.15f, 1.f));
            vpToAdd->p_color.setValue(cgt::vec2(0.f, 1.f));
            histogram->addPredicate(vpToAdd);

            vpToAdd = new RangePointPredicate("plaque", "Plaque", "Plaque Mask");
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setValue(cgt::vec2(0.5f, 1.f));
            vpToAdd->p_color.setValue(cgt::vec2(0.35f, 1.f));
            histogram->addPredicate(vpToAdd);

            histogram->resetPredicates(false);
            std::vector<float> adjustment(histogram->getPredicates().size(), 0.f);
            adjustment[0] = -1.f;
            php->adjustImportances(adjustment, php->getCurrentHistogramDistribution());
            addProperty(*php);

            php->addSharedProperty(_ve.getNestedProperty("VolumeRendererProperties::RaycasterProps::PredicateHistogram"));
            php->addSharedProperty(_ve.getNestedProperty("SliceExtractorProperties::PredicateHistogram"));
        }

        SimpleTransferFunction* stf = new SimpleTransferFunction(128, cgt::vec2(.1f, 1.f));
        stf->setLeftColor(cgt::vec4(0.f));
        stf->setRightColor(cgt::vec4(255.f));
        static_cast<TransferFunctionProperty*>(_ve.getNestedProperty("VolumeRendererProperties::RaycasterProps::TransferFunction"))->replaceTF(stf);

    }

    void IvusTcDemo::deinit() {
        _imageReader.deinit();
        p_readImagesButton.s_clicked.disconnect(this);
        AutoEvaluationPipeline::deinit();
    }

    void IvusTcDemo::readAndProcessImages() {
        std::string baseDir = cgt::FileSystem::cleanupPath(p_sourceDirectory.getValue());

        std::string ivusDir = baseDir + "/IVUS";
        std::string cmDir = baseDir + "/Confidence_Map";
        std::string calcifiedDir = baseDir + "/Calcified_Map";
        std::string fibroticDir = baseDir + "/Fibrotic_Map";
        std::string lipidicDir = baseDir + "/Lipidic_Map";
        std::string necroticDir = baseDir + "/Necrotic_Map";
        std::string plaqueDir = baseDir + "/Plaque_Mask";

        // read images
        readImage(ivusDir, "image.ivus");
        readImage(cmDir, "image.cm");

        readImage(calcifiedDir, "image.calcified");
        readImage(fibroticDir , "image.fibrotic" );
        readImage(lipidicDir  , "image.lipidic"  );
        readImage(necroticDir , "image.necrotic" );

        readImage(plaqueDir, "image.plaque");

        // fuse TC maps into single 4-channel image
        typedef uint8_t ivus_t;
        GenericImageRepresentationLocal<ivus_t, 1>::ScopedRepresentation calcifiedRep(getDataContainer(), "image.calcified");
        GenericImageRepresentationLocal<ivus_t, 1>::ScopedRepresentation fibroticRep(getDataContainer(), "image.fibrotic");
        GenericImageRepresentationLocal<ivus_t, 1>::ScopedRepresentation lipidicRep(getDataContainer(), "image.lipidic");
        GenericImageRepresentationLocal<ivus_t, 1>::ScopedRepresentation necroticRep(getDataContainer(), "image.necrotic");

        if (   (calcifiedRep && fibroticRep && lipidicRep && necroticRep) 
            && (calcifiedRep->getSize() == fibroticRep->getSize() && fibroticRep->getSize() == lipidicRep->getSize() && lipidicRep->getSize() == necroticRep->getSize())) {
            ImageData* id = new ImageData(3, calcifiedRep->getSize(), 4);
            cgt::Vector4<ivus_t>* data = new cgt::Vector4<ivus_t>[cgt::hmul(calcifiedRep->getSize())];

            for (size_t i = 0; i < calcifiedRep->getNumElements(); ++i) {
                data[i] = cgt::Vector4<ivus_t>(calcifiedRep->getElement(i), fibroticRep->getElement(i), lipidicRep->getElement(i), necroticRep->getElement(i));
            }

            GenericImageRepresentationLocal<ivus_t, 4>::create(id, data);
            id->setMappingInformation(calcifiedRep->getParent()->getMappingInformation());
            getDataContainer().addData("image.tc", id);
        }
        else {
            LERROR("Could not create fused TC image.");
        }

        getDataContainer().removeData("image.calcified");
        getDataContainer().removeData("image.fibrotic" );
        getDataContainer().removeData("image.lipidic"  );
        getDataContainer().removeData("image.necrotic" );
    }

    void IvusTcDemo::readImage(const std::string& baseDir, const std::string& id) {
        _imageReader.p_inputDirectory.setValue(baseDir);
        _imageReader.p_outputImage.setValue(id);
        _imageReader.process(getDataContainer());
    }


}