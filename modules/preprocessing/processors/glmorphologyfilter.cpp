// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#include "glmorphologyfilter.h"

#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"

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
        , p_inputImage("InputImage", "Input Image", "", DataNameProperty::READ)
        , p_outputImage("OutputImage", "Output Image", "GlMorphologyFilter.out", DataNameProperty::WRITE)
        , p_filterOperation("FilterOperation", "Operations to Apply ([edoc]+)", "ed", StringProperty::BASIC_STRING)
        , p_structuringElement("StructuringElement", "Structuring Element", structuringElementOptions, 2)
        , _erosionFilter(nullptr)
        , _dilationFilter(nullptr)
    {
        addProperty(p_inputImage);
        addProperty(p_outputImage);
        addProperty(p_filterOperation);
        addProperty(p_structuringElement, INVALID_SHADER | INVALID_RESULT);
    }

    GlMorphologyFilter::~GlMorphologyFilter() {

    }

    void GlMorphologyFilter::init() {
        VisualizationProcessor::init();

        _erosionFilter = ShdrMgr.load("core/glsl/passthrough.vert", "modules/preprocessing/glsl/GlMorphologyFilter.frag", generateGlslHeader("min"));
        _dilationFilter = ShdrMgr.load("core/glsl/passthrough.vert", "modules/preprocessing/glsl/GlMorphologyFilter.frag", generateGlslHeader("max"));
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

                const cgt::Texture* inputTexture = img->getTexture();
                cgt::Texture* outputTexture = nullptr;

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
                    ImageData* id = new ImageData(3, img->getSize(), img->getParent()->getNumChannels());
                    ImageRepresentationGL::create(id, outputTexture);
                    id->setMappingInformation(img->getParent()->getMappingInformation());
                    data.addData(p_outputImage.getValue(), id);
                }
                else {
                    data.addDataHandle(p_outputImage.getValue(), img.getDataHandle());
                }

                cgt::TextureUnit::setZeroUnit();
                LGL_ERROR;
            }
            else {
                // yes I was too lazy... But have a look in the shader: Half of the 2D support is already there. :)
                LERROR("Sorry, currently only 3D images are supported.");
            }
        }
        else {
            LDEBUG("No suitable input image found.");
        }
    }

    cgt::Texture* GlMorphologyFilter::applyFilter(const cgt::Texture* inputTexture, cgt::Shader* filter) const {
        cgtAssert(inputTexture != 0, "Input texture must not be 0.");
        const cgt::ivec3& size = inputTexture->getDimensions();

        cgt::TextureUnit inputUnit;
        inputUnit.activate();

        // create texture for result
        cgt::Texture* resultTexture = new cgt::Texture(inputTexture->getType(), size, inputTexture->getInternalFormat(), inputTexture->getFilter());

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
    }

    std::string GlMorphologyFilter::generateGlslHeader(const std::string& filerOp) const {
        std::string toReturn =  "#define FILTER_OP " + filerOp + "\n";
        toReturn += "#define " + p_structuringElement.getOptionValue() + "\n";

        return toReturn;
    }

}
