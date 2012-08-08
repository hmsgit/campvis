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

    void QuadRenderer::renderCube(const tgt::Bounds& bounds, const tgt::Bounds& texBounds) {
        const tgt::vec3& llf = bounds.getLLF();
        const tgt::vec3& urb = bounds.getURB();
        const tgt::vec3& tLlf = texBounds.getLLF();
        const tgt::vec3& tUrb = texBounds.getURB();

        // TODO: get fuckin' rid of intermediate mode, it simply sucks...
        glColor3f(1.f, 0.f, 1.f);
        glBegin(GL_QUADS);
            // front
            glTexCoord3f(tLlf.x, tUrb.y, tLlf.z);
            glVertex3f(llf.x, urb.y, llf.z);
            glTexCoord3f(tUrb.x, tUrb.y, tLlf.z);
            glVertex3f(urb.x, urb.y, llf.z);
            glTexCoord3f(tUrb.x, tLlf.y, tLlf.z);
            glVertex3f(urb.x, llf.y, llf.z);
            glTexCoord3f(tLlf.x, tLlf.y, tLlf.z);
            glVertex3f(llf.x, llf.y, llf.z);

            // right
            glTexCoord3f(tUrb.x, tUrb.y, tLlf.z);
            glVertex3f(urb.x, urb.y, llf.z);
            glTexCoord3f(tUrb.x, tUrb.y, tUrb.z);
            glVertex3f(urb.x, urb.y, urb.z);
            glTexCoord3f(tUrb.x, tLlf.y, tUrb.z);
            glVertex3f(urb.x, llf.y, urb.z);
            glTexCoord3f(tUrb.x, tLlf.y, tLlf.z);
            glVertex3f(urb.x, llf.y, llf.z);

            // top
            glTexCoord3f(tLlf.x, tUrb.y, tUrb.z);
            glVertex3f(llf.x, urb.y, urb.z);
            glTexCoord3f(tUrb.x, tUrb.y, tUrb.z);
            glVertex3f(urb.x, urb.y, urb.z);
            glTexCoord3f(tUrb.x, tUrb.y, tLlf.z);
            glVertex3f(urb.x, urb.y, llf.z);
            glTexCoord3f(tLlf.x, tUrb.y, tLlf.z);
            glVertex3f(llf.x, urb.y, llf.z);

            // left
            glTexCoord3f(tLlf.x, tUrb.y, tUrb.z);
            glVertex3f(llf.x, urb.y, urb.z);
            glTexCoord3f(tLlf.x, tUrb.y, tLlf.z);
            glVertex3f(llf.x, urb.y, llf.z);
            glTexCoord3f(tLlf.x, tLlf.y, tLlf.z);
            glVertex3f(llf.x, llf.y, llf.z);
            glTexCoord3f(tLlf.x, tLlf.y, tUrb.z);
            glVertex3f(llf.x, llf.y, urb.z);

            // bottom
            glTexCoord3f(tLlf.x, tLlf.y, tLlf.z);
            glVertex3f(llf.x, llf.y, llf.z);
            glTexCoord3f(tUrb.x, tLlf.y, tLlf.z);
            glVertex3f(urb.x, llf.y, llf.z);
            glTexCoord3f(tUrb.x, tLlf.y, tUrb.z);
            glVertex3f(urb.x, llf.y, urb.z);
            glTexCoord3f(tLlf.x, tLlf.y, tUrb.z);
            glVertex3f(llf.x, llf.y, urb.z);

            // back
            glTexCoord3f(tUrb.x, tUrb.y, tUrb.z);
            glVertex3f(urb.x, urb.y, urb.z);
            glTexCoord3f(tLlf.x, tUrb.y, tUrb.z);
            glVertex3f(llf.x, urb.y, urb.z);
            glTexCoord3f(tLlf.x, tLlf.y, tUrb.z);
            glVertex3f(llf.x, llf.y, urb.z);
            glTexCoord3f(tUrb.x, tLlf.y, tUrb.z);
            glVertex3f(urb.x, llf.y, urb.z);
        glEnd();
    }

}