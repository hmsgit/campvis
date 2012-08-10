#ifndef PROPERTYWIDGETFACTORY_H__
#define PROPERTYWIDGETFACTORY_H__

namespace TUMVis {
    class AbstractPropertyWidget;
    class AbstractProperty;

    /**
     * Factory class offering the static method createWidget to create property widgets for a given
     * property (depending on its type).
     */
    class PropertyWidgetFactory {
    public:
        /**
         * Creates the corresponding property widget for the given property \a property.
         * \note    The callee has to take the ownership of the returned pointer.
         * \param   property    Property to generate the widget for.
         * \return  A new property widget for the given property (depending on its type).
         */
        static AbstractPropertyWidget* createWidget(AbstractProperty* property);
    };
}
#endif // PROPERTYWIDGETFACTORY_H__
