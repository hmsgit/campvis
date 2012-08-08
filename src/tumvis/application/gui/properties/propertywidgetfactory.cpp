#include "propertywidgetfactory.h"

#include "application/gui/properties/abstractpropertywidget.h"
#include "application/gui/properties/intpropertywidget.h"
#include "application/gui/properties/floatpropertywidget.h"
#include "application/gui/properties/stringpropertywidget.h"
#include "application/gui/properties/transferfunctionpropertywidget.h"

#include "core/properties/abstractproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/transferfunctionproperty.h"


namespace TUMVis {

    AbstractPropertyWidget* PropertyWidgetFactory::createWidget(AbstractProperty* property) {
        tgtAssert(property != 0, "Property must not be 0.");

        if (IntProperty* tester = dynamic_cast<IntProperty*>(property)) {
            return new IntPropertyWidget(tester);
        }

        if (FloatProperty* tester = dynamic_cast<FloatProperty*>(property)) {
            return new FloatPropertyWidget(tester);
        }

        if (StringProperty* tester = dynamic_cast<StringProperty*>(property)) {
            return new StringPropertyWidget(tester);
        }

        if (TransferFunctionProperty* tester = dynamic_cast<TransferFunctionProperty*>(property)) {
            return new TransferFunctionPropertyWidget(tester);
        }

        return 0;
    }

}