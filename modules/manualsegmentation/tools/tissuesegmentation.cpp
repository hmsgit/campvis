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

#include "tissuesegmentation.h"

namespace campvis {

TissueSegmentation::TissueSegmentation() {

}

TissueSegmentation::~TissueSegmentation() {

}

size_t TissueSegmentation::getNumLayers() const {
    return _layers.size();
}

TissueSegmentation::Layer& TissueSegmentation::getLayer(size_t i) {
    return _layers[i];
}

const TissueSegmentation::Layer& TissueSegmentation::getLayer(size_t i) const {
    return _layers[i];
}

void TissueSegmentation::addLayer(const Layer& l) {
    _layers.push_back(l);
}

void TissueSegmentation::setLayer(size_t index, const Layer& l) {
    if (_layers.size() < index+1)
        _layers.resize(index+1);

    _layers[index] = l;
}

void TissueSegmentation::clear() {
    _layers.clear();
}

void TissueSegmentation::loadFromStream(std::istream& s) {
    clear();

    size_t numLayers = 0;
    s.read(reinterpret_cast<char*>(&numLayers), sizeof(size_t));
    for (size_t l = 0; l < numLayers; ++l) {
        size_t numFrames = 0;
        s.read(reinterpret_cast<char*>(&numFrames), sizeof(size_t));

        Layer layer;
        for (size_t i = 0; i < numFrames; ++i) {
            size_t numSamples = 0;
            s.read(reinterpret_cast<char*>(&numSamples), sizeof(size_t));

            Layer::Frame frame;
            frame._points.resize(numSamples);
            frame._gradients.resize(numSamples);

            if (numSamples > 0) {
                s.read(reinterpret_cast<char*>(&(frame._points.front())), numSamples * sizeof(cgt::vec2));
                s.read(reinterpret_cast<char*>(&(frame._gradients.front())), numSamples * sizeof(cgt::vec2));
            }

            layer._frames.push_back(frame);
        }
        _layers.push_back(layer);
    }
}

void TissueSegmentation::saveToStream(std::ostream& s) {
    size_t numLayers = _layers.size();
    s.write(reinterpret_cast<char*>(&numLayers), sizeof(size_t));

    for (size_t l = 0; l < numLayers; ++l) {
        Layer& layer = _layers[l];

        size_t numFrames = layer._frames.size();
        s.write(reinterpret_cast<char*>(&numFrames), sizeof(size_t));

        for (size_t i = 0; i < numFrames; ++i) {
            size_t numSamples = layer._frames[i]._points.size();
            s.write(reinterpret_cast<char*>(&numSamples), sizeof(size_t));

            if (numSamples > 0) {
                s.write(reinterpret_cast<char*>(&layer._frames[i]._points.front()), numSamples * sizeof(cgt::vec2));
                s.write(reinterpret_cast<char*>(&layer._frames[i]._gradients.front()), numSamples * sizeof(cgt::vec2));
            }
        }
    }
}

}