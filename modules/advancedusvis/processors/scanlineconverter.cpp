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

#include "scanlineconverter.h"

#include "cgt/logmanager.h"
#include "cgt/cgt_math.h"
#include "cgt/vector.h"
#include "core/tools/interval.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

namespace campvis {

    const std::string ScanlineConverter::loggerCat_ = "CAMPVis.modules.classification.ScanlineConverter";

    ScanlineConverter::ScanlineConverter()
        : AbstractProcessor()
        , p_sourceImageID("InputImage", "Input Image ID", "image", DataNameProperty::READ)
        , p_targetImageID("OutputConfidenceMap", "Output Confidence Map Image ID", "confidencemap", DataNameProperty::WRITE)
        , p_targetSize("TargetSize", "Target Image Size", cgt::ivec2(512, 512), cgt::ivec2(16, 16), cgt::ivec2(2048, 2048))
        , p_origin("PolarOrigin", "Polar Origin", cgt::vec2(340.f, 536.f), cgt::vec2(-1000.f), cgt::vec2(1000.f), cgt::vec2(0.1f))
        , p_angles("PolarAngles", "Polar Angles", cgt::vec2(233.f, 308.f), cgt::vec2(0.f), cgt::vec2(360.f), cgt::vec2(0.1f))
        , p_lengths("PolarLengths", "Polar Lengths", cgt::vec2(116.f, 540.f), cgt::vec2(0.f), cgt::vec2(1000.f), cgt::vec2(0.1f))
    {
        addProperty(p_sourceImageID);
        addProperty(p_targetImageID);
        addProperty(p_targetSize);
        addProperty(p_origin);
        addProperty(p_angles);
        addProperty(p_lengths);
    }

    ScanlineConverter::~ScanlineConverter() {

    }

    std::vector<cgt::vec3> ScanlineConverter::generateLookupVertices(const ImageData* inputImage) const {
        cgtAssert(inputImage != nullptr, "Input image must not be 0!");
        std::vector<cgt::vec3> vertices;

        const cgt::ivec2& outputSize = p_targetSize.getValue();
        const float rarara = cgt::PIf / 180.f;
        Interval<float> fanAngles(p_angles.getValue().x * rarara, p_angles.getValue().y * rarara);
        Interval<float> fanSize(p_lengths.getValue().x, p_lengths.getValue().y);


        for (int y = 0; y < outputSize.y; ++y) {
            float r = fanSize.getLeft() + static_cast<float>(outputSize.y - 1 - y) / static_cast<float>(outputSize.y) * fanSize.size();

            for (int x = 0; x < outputSize.x; ++x) {
                float phi = fanAngles.getLeft() + (static_cast<float>(x) / static_cast<float>(outputSize.x) * fanAngles.size());
                const cgt::vec3 cc(r * cos(phi) + p_origin.getValue().x, r * sin(phi) + p_origin.getValue().y, 0.f);
                vertices.push_back(cc);
            }
        }

        return vertices;
    }

    void ScanlineConverter::updateResult(DataContainer& dataContainer) {
        ImageRepresentationLocal::ScopedRepresentation input(dataContainer, p_sourceImageID.getValue());

        if (input != 0 && input->getDimensionality() == 2) {
            // resample image
            size_t numChannels = input->getParent()->getNumChannels();
            auto outputImage = new ImageData(2, cgt::vec3(p_targetSize.getValue().x, p_targetSize.getValue().y, 1), numChannels);
            
            auto wtp = input->getWeaklyTypedPointer();
            wtp._pointer = nullptr;
            auto outputRep = ImageRepresentationLocal::create(outputImage, wtp);

            auto vertices = generateLookupVertices(input->getParent());
            for (size_t i = 0; i < vertices.size(); ++i) {
                for (size_t c = 0; c < numChannels; ++c) {
                    outputRep->setElementNormalized(i, c, input->getElementNormalizedLinear(vertices[i], c));
                }
            }

            dataContainer.addData(p_targetImageID.getValue(), outputImage);
        }
        else {
            LDEBUG("No suitable input image found.");
        }
    }

}
