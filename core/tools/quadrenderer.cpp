#include "quadrenderer.h"
#include "core/datastructures/facegeometry.h"

namespace campvis {


    QuadRenderer::QuadRenderer()
        : tgt::Singleton<QuadRenderer>()
        , _quad(0)
    {
        std::vector<tgt::vec3> vertices, texCorods;

        vertices.push_back(tgt::vec3(-1.f, -1.f, 0.f));
        vertices.push_back(tgt::vec3( 1.f, -1.f, 0.f));
        vertices.push_back(tgt::vec3( 1.f,  1.f, 0.f));
        vertices.push_back(tgt::vec3(-1.f,  1.f, 0.f));
        texCorods.push_back(tgt::vec3(0.f, 0.f, 0.f));
        texCorods.push_back(tgt::vec3(1.f, 0.f, 0.f));
        texCorods.push_back(tgt::vec3(1.f, 1.f, 0.f));
        texCorods.push_back(tgt::vec3(0.f, 1.f, 0.f));

        _quad = new FaceGeometry(vertices, texCorods);
        _quad->createGLBuffers();
    }

    QuadRenderer::~QuadRenderer() {
        delete _quad;
    }

    void QuadRenderer::renderQuad() {
        _quad->render();

//         // TODO: get rid of intermediate mode and use VBOs as soon as they are ready.
//         glDepthFunc(GL_ALWAYS);
//         glBegin(GL_QUADS);
//             glTexCoord2f(0.f, 0.f);
//             glVertex2f(-1.f, -1.f);
// 
//             glTexCoord2f(1.f, 0.f);
//             glVertex2f(1.f, -1.f);
// 
//             glTexCoord2f(1.f, 1.f);
//             glVertex2f(1.f, 1.f);
// 
//             glTexCoord2f(0.f, 1.f);
//             glVertex2f(-1.f, 1.f);
//         glEnd();
//         glDepthFunc(GL_LESS);
    }


}