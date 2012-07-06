#include "tgt/logmanager.h"
#include "tgt/glcanvas.h"
#include "tgt/painter.h"
#include "tgt/event/eventhandler.h"

namespace TUMVis {

    /**
     * Painter class for TUMVis
     * 
     * \todo 
     */
    class TumVisPainter : public tgt::Painter {
    public:
        TumVisPainter(tgt::GLCanvas* canvas);

        virtual void paint();  
        virtual void sizeChanged(const tgt::ivec2& size);  
        virtual void init();  

    private:
        static const std::string loggerCat_;

        float _size;
    };

}
