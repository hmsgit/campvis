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

#ifndef TISSUESEGMENTATION_H__
#define TISSUESEGMENTATION_H__

#include "cgt/vector.h"

#include <iostream>
#include <vector>

namespace campvis {

    class TissueSegmentation {
    public:
        // A segmentation layer consists of a vector of Frames, each containing a vector of points.
        struct Layer {
            struct Frame {
                std::vector<tgt::vec2> _points;
                std::vector<tgt::vec2> _gradients;
            };

            std::vector<Frame> _frames;
        };


        TissueSegmentation();
        ~TissueSegmentation();

        size_t getNumLayers() const;

        const Layer& getLayer(size_t i) const;
        Layer& getLayer(size_t i);

        void addLayer(const Layer& l);

        void setLayer(size_t index, const Layer& l);

        void clear();


        void loadFromStream(std::istream& s);

        void saveToStream(std::ostream& s);

        // vector of centerlines
        std::vector<tgt::vec4> _centerlines;

    protected:
        // Vector of segmentation layers
        std::vector<Layer> _layers;
    };

}



#endif // TISSUESEGMENTATION_H__
