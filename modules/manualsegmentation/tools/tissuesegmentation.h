#ifndef TISSUESEGMENTATION_H__
#define TISSUESEGMENTATION_H__

#include "tgt/vector.h"

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
