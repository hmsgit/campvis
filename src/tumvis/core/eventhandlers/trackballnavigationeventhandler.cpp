#include "trackballnavigationeventhandler.h"
#include "tgt/assert.h"
#include "tgt/event/mouseevent.h"
#include "core/properties/cameraproperty.h"

namespace TUMVis {
    const std::string TrackballNavigationEventHandler::loggerCat_ = "TUMVis.core.eventhandler.TrackballNavigationEventHandler";

    TrackballNavigationEventHandler::TrackballNavigationEventHandler(CameraProperty* property)
        : AbstractEventHandler()
        , _cameraProperty(property)
    {
        tgtAssert(_cameraProperty != 0, "Assigned property must not be 0.");
    }

    TrackballNavigationEventHandler::~TrackballNavigationEventHandler() {

    }

    bool TrackballNavigationEventHandler::accept(tgt::Event* e) {
        return false;
    }

    void TrackballNavigationEventHandler::execute(tgt::Event* e) {

    }

}
