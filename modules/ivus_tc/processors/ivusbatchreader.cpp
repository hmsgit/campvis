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

#include "ivusbatchreader.h"

#include <IL/il.h>

#include "tgt/logmanager.h"
#include "tgt/filesystem.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/tools/stringutils.h"

namespace campvis {
    const std::string IvusBatchReader::loggerCat_ = "CAMPVis.modules.vis.IvusBatchReader";

    IvusBatchReader::IvusBatchReader(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputDirectory("InputDirectory", "Input Directory", "", StringProperty::DIRECTORY)
        , p_fileExtension("FileExtension", "File Extension", "bmp", StringProperty::BASIC_STRING)
        , p_imageSpacing("ImageSpacing", "Image Spacing", tgt::vec3(.1f, .1f, .5f), tgt::vec3(0.f), tgt::vec3(10.f), tgt::vec3(.1f), tgt::ivec3(2))
        , p_outputImage("OutputImage", "Output Image", "dd.output", DataNameProperty::WRITE)
    {
        addProperty(p_inputDirectory);
        addProperty(p_fileExtension);
        addProperty(p_outputImage);
    }

    IvusBatchReader::~IvusBatchReader() {

    }

    void IvusBatchReader::init() {
        VisualizationProcessor::init();
    }

    void IvusBatchReader::deinit() {
        VisualizationProcessor::deinit();
    }

    void IvusBatchReader::updateResult(DataContainer& data) {
        std::string ext = StringUtils::lowercase(p_fileExtension.getValue());

        std::vector<std::string> files = tgt::FileSystem::listFiles(p_inputDirectory.getValue(), true);
        files.erase(std::remove_if(files.begin(), files.end(), [&] (const std::string& s) -> bool { return tgt::FileSystem::fileExtension(s, true) != ext; } ), files.end());

        tgt::ivec3 imageSize(0, 0, static_cast<int>(files.size()));
        uint8_t* buffer = nullptr;

        for (size_t i = 0; i < files.size(); ++i) {
            std::string filename = p_inputDirectory.getValue() + "/" + files[i];

            // prepare DevIL
            ILuint img;
            ilGenImages(1, &img);
            ilBindImage(img);

            // try load file
            if (! ilLoadImage(filename.c_str())) {
                LERROR("Could not load image: " << filename);
                delete [] buffer;
                return;
            }

            // prepare buffer and perform dimensions check
            if (i == 0) {
                imageSize.x = ilGetInteger(IL_IMAGE_WIDTH);
                imageSize.y = ilGetInteger(IL_IMAGE_HEIGHT);

                buffer = new uint8_t[tgt::hmul(imageSize)];
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
            ILubyte *data = ilGetData(); 

            ilCopyPixels(0, 0, 0, imageSize.x, imageSize.y, 1, IL_LUMINANCE, IL_UNSIGNED_BYTE, buffer + (sizeof(uint8_t) * i * imageSize.x * imageSize.y));
            ILint err = ilGetError();
            if (err != IL_NO_ERROR) {
                LERROR("Error during conversion: " << iluErrorString(err));
                delete [] buffer;
                return;
            }

            ilDeleteImage(img);
        }

        ImageData* id = new ImageData(3, imageSize, 1);
        GenericImageRepresentationLocal<uint8_t, 1>::create(id, buffer);
        data.addData(p_outputImage.getValue(), id);
        ImageMappingInformation imi(imageSize, tgt::vec3(0.f), p_imageSpacing.getValue());
        id->setMappingInformation(imi);
        validate(AbstractProcessor::INVALID_RESULT);
    }

}

