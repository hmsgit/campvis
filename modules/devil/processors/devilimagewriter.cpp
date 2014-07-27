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

#include "devilimagewriter.h"


#include <cstring>

#include "tgt/logmanager.h"
#include "tgt/filesystem.h"
#include "tgt/shadermanager.h"
#include "tgt/texturereaderdevil.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationlocal.h"
#include "core/datastructures/renderdata.h"

#include "core/tools/stringutils.h"


namespace campvis {
    const std::string DevilImageWriter::loggerCat_ = "CAMPVis.modules.io.DevilImageWriter";

    DevilImageWriter::DevilImageWriter()
        : AbstractProcessor()
        , p_inputImage("InputImage", "Input Image ID", "DevilImageWriter.input", DataNameProperty::READ)
        , p_url("url", "Image URL", "", StringProperty::SAVE_FILENAME)
    {
        addProperty(p_inputImage);
        addProperty(p_url);
    }

    DevilImageWriter::~DevilImageWriter() {
    }

    void DevilImageWriter::updateResult(DataContainer& data) {
        ScopedTypedData<RenderData> image(data, p_inputImage.getValue());

        if (image != 0) {
            std::string filename = p_url.getValue();
            std::string extension = tgt::FileSystem::fileExtension(filename);
            std::string filebase = tgt::FileSystem::fullBaseName(filename);
            if (extension.empty()) {
                LINFO("Filename has no extension, defaulting to .PNG.");
                extension = ".png";
            }

            for (size_t i = 0; i < image->getNumColorTextures(); ++i) {
                const ImageRepresentationLocal* id = image->getColorTexture(i)->getRepresentation<ImageRepresentationLocal>(true);
                if (id == 0) {
                    LERROR("Could not download color texture " << i << " from RenderData, skipping.");
                    continue;
                }

                WeaklyTypedPointer wtp = id->getWeaklyTypedPointer();
                writeIlImage(wtp, id->getSize().xy(), filebase + ".color" + ((image->getNumColorTextures() > 1) ? StringUtils::toString(i) : "") + extension);
            }
            if (image->hasDepthTexture()) {
                const ImageRepresentationLocal* id = image->getDepthTexture()->getRepresentation<ImageRepresentationLocal>(true);
                if (id == 0) {
                    LERROR("Could not download depth texture from RenderData, skipping.");
                }
                else {
                    WeaklyTypedPointer wtp = id->getWeaklyTypedPointer();
                    writeIlImage(wtp, id->getSize().xy(), filebase + ".depth" + extension);
                }
            }
        }
        else {
            LDEBUG("Could not load RenderData with given ID from pipeline.");
        }
    }

    void DevilImageWriter::writeIlImage(const WeaklyTypedPointer& wtp, const tgt::ivec2& size, const std::string& filename) const {
        // create Devil image from image data and write it to file
        ILuint img;
        ilGenImages(1, &img);
        ilBindImage(img);

        // put pixels into IL-Image
        ilTexImage(size.x, size.y, 1, static_cast<ILubyte>(wtp._numChannels), wtp.getIlFormat(), wtp.getIlDataType(), wtp._pointer);
        ilEnable(IL_FILE_OVERWRITE);
        ilResetWrite();
        ILboolean success = ilSaveImage(filename.c_str());
        ilDeleteImages(1, &img);

        if (success != IL_NO_ERROR) {
            ILenum errorcode;
            while ((errorcode = ilGetError()) != IL_NO_ERROR) {
                LERROR("Error while writing '" << filename << "': "<< iluErrorString(errorcode));
            } 
        }
    }

}