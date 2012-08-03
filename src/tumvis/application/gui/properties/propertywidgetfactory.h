#ifndef PROPERTYWIDGETFACTORY_H__
#define PROPERTYWIDGETFACTORY_H__

namespace TUMVis {
    class AbstractPropertyWidget;
    class AbstractProperty;

    class PropertyWidgetFactory {
    public:
        /**
         * 
         * \param property 
         * \return 
         */
        static AbstractPropertyWidget* createWidget(AbstractProperty* property);
    };
}
#endif // PROPERTYWIDGETFACTORY_H__
