#include "propertywidgetfactory.h"

#include "application/gui/properties/abstractpropertywidget.h"
#include "application/gui/properties/stringpropertywidget.h"

#include "core/properties/abstractproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/transferfunctionproperty.h"


namespace TUMVis {

    AbstractPropertyWidget* PropertyWidgetFactory::createWidget(AbstractProperty* property) {
        tgtAssert(property != 0, "Property must not be 0.");

        if (StringProperty* tester = dynamic_cast<StringProperty*>(property)) {
        	return new StringPropertyWidget(tester);
        }

        return 0;
    }

}