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

#include "glintensityquantizer.h"

#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"
#include "tgt/texture.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"

#include "core/tools/quadrenderer.h"

namespace campvis {

    const std::string GlIntensityQuantizer::loggerCat_ = "CAMPVis.modules.classification.GlIntensityQuantizer";

    GlIntensityQuantizer::GlIntensityQuantizer(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputImage("InputImage", "Input Image", "", DataNameProperty::READ)
        , p_outputImage("OutputImage", "Output Image", "GlIntensityQuantizer.out", DataNameProperty::WRITE)
        , p_numberOfBins("NumberOfBins", "Number of Bins", 8, 1, 256)
        , p_transferFunction("TransferFunction", "Transfer Function", new Geometry1DTransferFunction(256))
        , _shader(0)
    {
        addProperty(p_inputImage);
        addProperty(p_outputImage);
        addProperty(p_numberOfBins);
        addProperty(p_transferFunction);

        Geometry1DTransferFunction* gtf = static_cast<Geometry1DTransferFunction*>(p_transferFunction.getTF());
        gtf->addGeometry(TFGeometry1D::createQuad(tgt::vec2(0.f, 1.f), tgt::col4(0, 0, 0, 0), tgt::col4(255, 255, 255, 255)));
    }

    GlIntensityQuantizer::~GlIntensityQuantizer() {

    }

    void GlIntensityQuantizer::init() {
        VisualizationProcessor::init();

        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/preprocessing/glsl/glintensityquantizer.frag", "");
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void GlIntensityQuantizer::deinit() {
        ShdrMgr.dispose(_shader);
        VisualizationProcessor::deinit();
    }

    void GlIntensityQuantizer::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_inputImage.getValue());

        if (img != 0) {
            tgt::ivec3 size = img->getSize();

            tgt::TextureUnit inputUnit, tfUnit;
            inputUnit.activate();

            // create texture for result
            tgt::Texture* resultTexture = new tgt::Texture(0, size, img->getTexture()->getFormat(), img->getTexture()->getInternalFormat(), img->getTexture()->getDataType(), tgt::Texture::LINEAR);
            resultTexture->uploadTexture();

            // activate shader and bind textures
            _shader->activate();
            _shader->setUniform("_numberOfBins", p_numberOfBins.getValue());
            img->bind(_shader, inputUnit);
            p_transferFunction.getTF()->bind(_shader, tfUnit);

            // activate FBO and attach texture
            _fbo->activate();
            glViewport(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));

            // render quad to compute difference measure by shader
            for (int z = 0; z < size.z; ++z) {
                float zTexCoord = static_cast<float>(z)/static_cast<float>(size.z) + .5f/static_cast<float>(size.z);
                _shader->setUniform("_zTexCoord", zTexCoord);
                _fbo->attachTexture(resultTexture, GL_COLOR_ATTACHMENT0, 0, z);
                QuadRdr.renderQuad();
            }
            _fbo->detachAll();
            _fbo->deactivate();
            _shader->deactivate();

            // put resulting image into DataContainer
            ImageData* id = new ImageData(3, size, img.getImageData()->getNumChannels());
            ImageRepresentationGL::create(id, resultTexture);
            const ImageMappingInformation& imi = img->getParent()->getMappingInformation();
            id->setMappingInformation(ImageMappingInformation(img->getSize(), imi.getOffset(), imi.getVoxelSize(), imi.getRealWorldMapping()));
            data.addData(p_outputImage.getValue(), id);

            tgt::TextureUnit::setZeroUnit();
            LGL_ERROR;
        }
        else {
            LERROR("No suitable input image found.");
        }

        validate(INVALID_RESULT);
    }

}
