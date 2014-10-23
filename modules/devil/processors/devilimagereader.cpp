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

#include "devilimagereader.h"


#include <IL/il.h>
#include <cstring>
#include <vector>

#include "cgt/logmanager.h"
#include "cgt/filesystem.h"
#include "cgt/shadermanager.h"
#include "cgt/texturereaderdevil.h"
#include "cgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

#include "core/tools/quadrenderer.h"
#include "core/tools/stringutils.h"
#include "core/tools/mapping.h"


namespace campvis {
    const std::string DevilImageReader::loggerCat_ = "CAMPVis.modules.io.DevilImageReader";

    GenericOption<std::string> importOptions[3] = {
        GenericOption<std::string>("rt", "Render Target"),
        GenericOption<std::string>("localIntensity", "Local Intensity Image"),
        GenericOption<std::string>("localIntensity3", "Local Intensity Image RGB")
    };

    DevilImageReader::DevilImageReader()
        : AbstractImageReader()
        , p_importType("ImportType", "Import Type", importOptions, 3)
        , p_importSimilar("ImportSimilar", "Import All Similar Files", false)
        , _shader(nullptr)
        , _devilTextureReader(nullptr)
    {
        this->_ext.push_back(".bmp");
        this->_ext.push_back(".jpg");
        this->_ext.push_back(".png");
        this->_ext.push_back(".tif");
        this->p_targetImageID.setValue("DevilImageReader.output");

        addProperty(p_url);
        addProperty(p_targetImageID);
        addProperty(p_importType);
        addProperty(p_importSimilar);

        _devilTextureReader = new cgt::TextureReaderDevil();
    }

    DevilImageReader::~DevilImageReader() {
        delete _devilTextureReader;
    }

    void DevilImageReader::init() {
        AbstractImageReader::init();
        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "core/glsl/copyimage.frag", "#define NO_DEPTH\n");
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void DevilImageReader::deinit() {
        AbstractImageReader::deinit();
        ShdrMgr.dispose(_shader);
    }

    void DevilImageReader::updateResult(DataContainer& data) {
        const std::string& url = p_url.getValue();
        std::string directory = cgt::FileSystem::dirName(url);
        std::string base = cgt::FileSystem::baseName(url);
        std::string ext = cgt::FileSystem::fileExtension(url);

        // check whether we open an image series
        size_t suffixPos = base.find_last_not_of("0123456789");
        if (suffixPos != std::string::npos)
            ++suffixPos;
        size_t suffixLength = (suffixPos == std::string::npos) ? 0 : base.length() - suffixPos;

        // assemble the list of files to read
        std::vector<std::string> files;
        if (suffixLength == 0 || !p_importSimilar.getValue()) {
            files.push_back(url);
        }
        else {
            std::string prefix = base.substr(0, suffixPos);
            int index = StringUtils::fromString<int>(base.substr(suffixPos));

            while (cgt::FileSystem::fileExists(directory + "/" + prefix + StringUtils::toString(index, suffixLength, '0') + "." + ext)) {
                files.push_back(directory + "/" + prefix + StringUtils::toString(index, suffixLength, '0') + "." + ext);
                ++index;
            }
        }

		if (files.empty())
			return;

        cgt::ivec3 imageSize(0, 0, static_cast<int>(files.size()));
        uint8_t* buffer = nullptr;

        ILint devilFormat = 0;
        if (p_importType.getOptionValue() == "localIntensity") 
            devilFormat = IL_LUMINANCE;
        else if (p_importType.getOptionValue() == "localIntensity3") 
            devilFormat = IL_RGB;
        else if (p_importType.getOptionValue() == "rt") 
            devilFormat = IL_RGBA;

        ILint devilDataType = 0;
		WeaklyTypedPointer::BaseType campvisDataType = WeaklyTypedPointer::UINT8;
        size_t numChannels = 1;

        // start reading
        for (size_t i = 0; i < files.size(); ++i) {
            // prepare DevIL
            ILuint img;
            ilGenImages(1, &img);
            ilBindImage(img);

            // try load file
            if (! ilLoadImage(files[i].c_str())) {
                LERROR("Could not load image: " << files[i]);
                delete [] buffer;
                return;
            }

            // prepare buffer and perform dimensions check
            if (i == 0) {
                imageSize.x = ilGetInteger(IL_IMAGE_WIDTH);
                imageSize.y = ilGetInteger(IL_IMAGE_HEIGHT);

                if (devilFormat == 0)
                    devilFormat = ilGetInteger(IL_IMAGE_FORMAT);

                switch (ilGetInteger(IL_IMAGE_TYPE)) {
                    case IL_UNSIGNED_BYTE:
                        devilDataType = IL_UNSIGNED_BYTE;
                        campvisDataType = WeaklyTypedPointer::UINT8;
                        break;
                    case IL_BYTE:
                        devilDataType = IL_BYTE;
                        campvisDataType = WeaklyTypedPointer::INT8;
                        break;
                    case IL_UNSIGNED_SHORT:
                        devilDataType = IL_UNSIGNED_SHORT;
                        campvisDataType = WeaklyTypedPointer::UINT16;
                        break;
                    case IL_SHORT:
                        devilDataType = IL_SHORT;
                        campvisDataType = WeaklyTypedPointer::INT16;
                        break;
                    case IL_UNSIGNED_INT:
                        devilDataType = IL_UNSIGNED_INT;
                        campvisDataType = WeaklyTypedPointer::UINT32;
                        break;
                    case IL_INT:
                        devilDataType = IL_INT;
                        campvisDataType = WeaklyTypedPointer::INT32;
                        break;
                    case IL_FLOAT:
                        devilDataType = IL_FLOAT;
                        campvisDataType = WeaklyTypedPointer::FLOAT;
                        break;
                    default:
                        LERROR("unsupported data type: " << ilGetInteger(IL_IMAGE_TYPE) << " (" << files[i] << ")");
                        return;
                }

                switch (devilFormat) {
                    case IL_LUMINANCE:
                        numChannels = 1;
                        break;
                    case IL_LUMINANCE_ALPHA:
                        numChannels = 2;
                        break;
                    case IL_RGB:
                        numChannels = 3;
                        break;
                    case IL_RGBA:
                        numChannels = 4;
                        break;
                    default:
                        LERROR("unsupported image format: " << devilFormat << " (" << files[i] << ")");
                        return;
                }
                buffer = new uint8_t[cgt::hmul(imageSize) * WeaklyTypedPointer::numBytes(campvisDataType, numChannels)];
            }
            else {
                if (imageSize.x != ilGetInteger(IL_IMAGE_WIDTH)) {
                    LERROR("Could not load images: widths do not match!");
                    delete [] buffer;
                    return;
                }
                if (imageSize.y != ilGetInteger(IL_IMAGE_HEIGHT)) {
                    LERROR("Could not load images: heights do not match!");
                    delete [] buffer;
                    return;
                }
            }

            // get data from image and transform to single intensity image:
            ilCopyPixels(0, 0, 0, imageSize.x, imageSize.y, 1, devilFormat, devilDataType, buffer + (WeaklyTypedPointer::numBytes(campvisDataType, numChannels) * i * imageSize.x * imageSize.y));
            ILint err = ilGetError();
            if (err != IL_NO_ERROR) {
                LERROR("Error during conversion: " << iluErrorString(err));
                delete [] buffer;
                return;
            }

            ilDeleteImage(img);
        }

        size_t dimensionality = 3;
        if (imageSize.z == 1)
            dimensionality = 2;
        if (imageSize.y == 1)
            dimensionality = 1;


        ImageData* id = new ImageData(dimensionality, imageSize, numChannels);
        WeaklyTypedPointer wtp(campvisDataType, numChannels, buffer);
        ImageRepresentationLocal::create(id, wtp);
        //id->setMappingInformation(ImageMappingInformation(imageSize, p_imageOffset.getValue(), p_voxelSize.getValue()));

        if (p_importType.getOptionValue() == "rt") {
            RenderData* rd = new RenderData();
            rd->addColorTexture(id);

            // create fake depth image
			// TODO: think of a better solution to this...
            ImageData* idDepth = new ImageData(dimensionality, imageSize, 1);
            float* ptr = new float[cgt::hmul(imageSize)];
			memset(ptr, 0, cgt::hmul(imageSize) * sizeof(float));
            WeaklyTypedPointer wtpDepth(campvisDataType, 1, ptr);
            ImageRepresentationLocal::create(idDepth, wtpDepth);
            rd->setDepthTexture(idDepth);

            data.addData(p_targetImageID.getValue(), rd);
        }
        else {
            data.addData(p_targetImageID.getValue(), id);
        }
    }
}