#ifndef QUADRENDERER_H__
#define QUADRENDERER_H__

#include "tgt/exception.h"
#include "tgt/singleton.h"
#include "tgt/tgt_gl.h"


namespace TUMVis {
    class FaceGeometry;

    /**
     * Small helper singleton class for rendering the default [-1, 1]^2 quad.
     */
    class QuadRenderer : public tgt::Singleton<QuadRenderer> {
        friend class tgt::Singleton<QuadRenderer>;

    public:
        /**
         *
         */
        virtual ~QuadRenderer();

        void renderQuad();

    private:
        QuadRenderer();

        FaceGeometry* _quad;
    };

#define QuadRdr tgt::Singleton<TUMVis::QuadRenderer>::getRef()

}

#endif // QUADRENDERER_H__
