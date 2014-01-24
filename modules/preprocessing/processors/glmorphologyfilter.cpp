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

#include "glmorphologyfilter.h"

#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorgradient.h"

#include "core/classification/simpletransferfunction.h"
#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

#include "core/tools/quadrenderer.h"
#include "core/tools/stringutils.h"

namespace campvis {
    GenericOption<std::string> structuringElementOptions[2] = {
        GenericOption<std::string>("cross", "Cross", "CROSS_ELEMENT"),
        GenericOption<std::string>("cube", "Cube", "CUBE_ELEMENT")
    };

    const std::string GlMorphologyFilter::loggerCat_ = "CAMPVis.modules.classification.GlMorphologyFilter";

    GlMorphologyFilter::GlMorphologyFilter(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputImage("InputImage", "Input Image", "", DataNameProperty::READ, AbstractProcessor::INVALID_RESULT)
        , p_outputImage("OutputImage", "Output Image", "GlMorphologyFilter.out", DataNameProperty::WRITE)
        , p_filterOperation("FilterOperation", "Operations to Apply ([edoc]+)", "ed")
        , p_structuringElement("StructuringElement", "Structuring Element", structuringElementOptions, 2, INVALID_SHADER | INVALID_RESULT)
        , _erosionFilter(nullptr)
        , _dilationFilter(nullptr)
    {
        addProperty(&p_inputImage);
        addProperty(&p_outputImage);
        addProperty(&p_filterOperation);
        addProperty(&p_structuringElement);
    }

    GlMorphologyFilter::~GlMorphologyFilter() {

    }

    void GlMorphologyFilter::init() {
        VisualizationProcessor::init();

        _erosionFilter = ShdrMgr.load("core/glsl/passthrough.vert", "modules/preprocessing/glsl/GlMorphologyFilter.frag", generateGlslHeader("min"));
        _erosionFilter->setAttributeLocation(0, "in_Position");
        _erosionFilter->setAttributeLocation(1, "in_TexCoord");

        _dilationFilter = ShdrMgr.load("core/glsl/passthrough.vert", "modules/preprocessing/glsl/GlMorphologyFilter.frag", generateGlslHeader("max"));
        _dilationFilter->setAttributeLocation(0, "in_Position");
        _dilationFilter->setAttributeLocation(1, "in_TexCoord");
    }

    void GlMorphologyFilter::deinit() {
        ShdrMgr.dispose(_erosionFilter);
        ShdrMgr.dispose(_dilationFilter);
        VisualizationProcessor::deinit();
    }

    void GlMorphologyFilter::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_inputImage.getValue());

        if (img != 0) {
            if (img->getDimensionality() == 3) {
                std::string ops = p_filterOperation.getValue();
                ops = StringUtils::replaceAll(ops, "o", "ed"); // opening := erosion, dilation
                ops = StringUtils::replaceAll(ops, "c", "de"); // closing := dilation, erosion

                const tgt::Texture* inputTexture = img->getTexture();
                tgt::Texture* outputTexture = nullptr;

                for (size_t i = 0; i < ops.length(); ++i) {
                    if (ops[i] == 'e') {
                        outputTexture = applyFilter(inputTexture, _erosionFilter);
                    }
                    else if (ops[i] == 'd') {
                        outputTexture = applyFilter(inputTexture, _dilationFilter);
                    }
                    else {
                        continue;
                    }

                    if (inputTexture != img->getTexture())
                        delete inputTexture;
                    inputTexture = outputTexture;
                }

                // put resulting image into DataContainer
                if (outputTexture != 0) {
                    ImageData* id = new ImageData(3, img->getSize(), 1);
                    ImageRepresentationGL::create(id, outputTexture);
                    id->setMappingInformation(img->getParent()->getMappingInformation());
                    data.addData(p_outputImage.getValue(), id);
                }
                else {
                    data.addDataHandle(p_outputImage.getValue(), img.getDataHandle());
                }

                tgt::TextureUnit::setZeroUnit();
                LGL_ERROR;
            }
            else {
                // yes I was too lazy... But have a look in the shader: Half of the 2D support is already there. :)
                LERROR("Sorry, currently only 3D images are supported.");
            }
        }
        else {
            LERROR("No suitable input image found.");
        }

        validate(INVALID_RESULT);
    }

    tgt::Texture* GlMorphologyFilter::applyFilter(const tgt::Texture* inputTexture, tgt::Shader* filter) const {
        tgtAssert(inputTexture != 0, "Input texture must not be 0.");
        const tgt::ivec3& size = inputTexture->getDimensions();

        tgt::TextureUnit inputUnit;
        inputUnit.activate();

        // create texture for result
        tgt::Texture* resultTexture = new tgt::Texture(0, size, inputTexture->getFormat(), inputTexture->getInternalFormat(), inputTexture->getDataType(), inputTexture->getFilter());
        resultTexture->uploadTexture();

        // activate shader and bind textures
        inputTexture->bind();
        filter->activate();
        filter->setUniform("_texture", inputUnit.getUnitNumber());
        filter->setUniform("_textureSize", size);

        // activate FBO and attach texture
        _fbo->activate();
        glViewport(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));

        // render quad to compute difference measure by shader
        for (int z = 0; z < size.z; ++z) {
            filter->setUniform("_zTexCoord", z);
            _fbo->attachTexture(resultTexture, GL_COLOR_ATTACHMENT0, 0, z);
            QuadRdr.renderQuad();
        }

        _fbo->detachAll();
        _fbo->deactivate();
        filter->deactivate();

        return resultTexture;
    }

    void GlMorphologyFilter::updateShader() {
        _erosionFilter->setHeaders(generateGlslHeader("min"));
        _erosionFilter->rebuild();
        _dilationFilter->setHeaders(generateGlslHeader("max"));
        _dilationFilter->rebuild();
        validate(INVALID_SHADER);
    }

    std::string GlMorphologyFilter::generateGlslHeader(const std::string& filerOp) const {
        std::string toReturn =  "#define FILTER_OP " + filerOp + "\n";
        toReturn += "#define " + p_structuringElement.getOptionValue() + "\n";

        return toReturn;
    }

}
