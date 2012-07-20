#include "quadrenderer.h"


namespace tgt {


    QuadRenderer::QuadRenderer() {

    }

    QuadRenderer::~QuadRenderer() {

    }

    void QuadRenderer::renderQuad() {
        // TODO: get rid of intermediate mode and use VBOs as soon as they are ready.
        glDepthFunc(GL_ALWAYS);
        glBegin(GL_QUADS);
            glTexCoord2f(0.f, 0.f);
            glVertex2f(-1.f, -1.f);

            glTexCoord2f(1.f, 0.f);
            glVertex2f(1.f, -1.f);

            glTexCoord2f(1.f, 1.f);
            glVertex2f(1.f, 1.f);

            glTexCoord2f(0.f, 1.f);
            glVertex2f(-1.f, 1.f);
        glEnd();
        glDepthFunc(GL_LESS);
    }

}