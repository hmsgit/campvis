#include "tgt/logmanager.h"
#include "tgt/glcanvas.h"
#include "tgt/painter.h"

namespace TUMVis {

    /**
     * 
     **/
    class TumVisPainter : public tgt::Painter {
    public:
        TumVisPainter(tgt::GLCanvas* canvas);

        virtual void paint();  
        virtual void sizeChanged(const tgt::ivec2& size);  
        virtual void init();  

    private:
        static const std::string loggerCat_;
    };

}
