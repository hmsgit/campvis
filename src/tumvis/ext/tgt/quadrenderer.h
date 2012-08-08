#ifndef QUADRENDERER_H__
#define QUADRENDERER_H__

#include "tgt/bounds.h"
#include "tgt/exception.h"
#include "tgt/singleton.h"
#include "tgt/tgt_gl.h"

namespace tgt {

    class QuadRenderer {
    public:
        QuadRenderer();
        virtual ~QuadRenderer();

        static void renderQuad();

        static void renderCube(const tgt::Bounds& bounds, const tgt::Bounds& texBounds);

    private:

    
    };


}

#endif // QUADRENDERER_H__
