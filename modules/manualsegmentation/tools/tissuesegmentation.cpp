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
                s.read(reinterpret_cast<char*>(&(frame._points.front())), numSamples * sizeof(tgt::vec2));
                s.read(reinterpret_cast<char*>(&(frame._gradients.front())), numSamples * sizeof(tgt::vec2));
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
                s.write(reinterpret_cast<char*>(&layer._frames[i]._points.front()), numSamples * sizeof(tgt::vec2));
                s.write(reinterpret_cast<char*>(&layer._frames[i]._gradients.front()), numSamples * sizeof(tgt::vec2));
            }
        }
    }
}

}