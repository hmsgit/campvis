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

#include "gradientvolumegenerator.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"

#include <tbb/tbb.h>

#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

namespace campvis {

    const std::string GradientVolumeGenerator::loggerCat_ = "CAMPVis.modules.classification.GradientVolumeGenerator";

    GradientVolumeGenerator::GradientVolumeGenerator()
        : AbstractProcessor()
        , p_sourceImageID("InputVolume", "Input Volume ID", "volume", DataNameProperty::READ)
        , p_targetImageID("OutputGradients", "Output Gradient Volume ID", "gradients", DataNameProperty::WRITE)
    {
        addProperty(p_sourceImageID);
        addProperty(p_targetImageID);
    }

    GradientVolumeGenerator::~GradientVolumeGenerator() {

    }

    void GradientVolumeGenerator::updateResult(DataContainer& data) {
        ImageRepresentationLocal::ScopedRepresentation input(data, p_sourceImageID.getValue());

        if (input != 0) {
            ImageData* id = new ImageData(input->getDimensionality(), input->getSize(), 4);
            GenericImageRepresentationLocal<float, 4>* output = GenericImageRepresentationLocal<float, 4>::create(id, 0);

            tbb::parallel_for(tbb::blocked_range<size_t>(0, input->getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    tgt::svec3 pos = input->getParent()->indexToPosition(i);
                    const tgt::svec3& size = input->getSize();

                    float dx, dy, dz, mdx, mdy, mdz;
                    dx = dy = dz = mdx = mdy = mdz = 0.f;

                    if (pos.x != size.x - 1)
                        dx = input->getElementNormalized(pos + tgt::svec3(1, 0, 0), 0);
                    if (pos.y != size.y - 1)
                        dy = input->getElementNormalized(pos + tgt::svec3(0, 1, 0), 0);
                    if (pos.z != size.z - 1)
                        dz = input->getElementNormalized(pos + tgt::svec3(0, 0, 1), 0);

                    if (pos.x != 0)
                        mdx = input->getElementNormalized(pos + tgt::svec3(-1, 0, 0), 0);
                    if (pos.y != 0)
                        mdy = input->getElementNormalized(pos + tgt::svec3(0, -1, 0), 0);
                    if (pos.z != 0)
                        mdz = input->getElementNormalized(pos + tgt::svec3(0, 0, -1), 0);


                    tgt::vec3 gradient(mdx - dx, mdy - dy, mdz - dz);
                    gradient /= input->getParent()->getMappingInformation().getVoxelSize() * tgt::vec3(2.f);

                    output->setElement(i, tgt::vec4(gradient, tgt::length(gradient)));
                }
            });

            data.addData(p_targetImageID.getValue(), id);
        }
        else {
            LDEBUG("No suitable input image found.");
        }


        validate(INVALID_RESULT);
    }

}
