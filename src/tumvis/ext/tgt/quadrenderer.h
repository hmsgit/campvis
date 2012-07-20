#ifndef QUADRENDERER_H__
#define QUADRENDERER_H__

#include "tgt/exception.h"
#include "tgt/singleton.h"
#include "tgt/tgt_gl.h"

namespace tgt {

    class QuadRenderer {
    public:
        QuadRenderer();
        virtual ~QuadRenderer();

        static void renderQuad();

    private:

    
    };


}

#endif // QUADRENDERER_H__
