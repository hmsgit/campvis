#ifndef FLOATPROPERTYWIDGET_H__
#define FLOATPROPERTYWIDGET_H__

#include "application/gui/properties/abstractpropertywidget.h"
#include "core/properties/numericproperty.h"

#include <QDoubleSpinBox>

namespace TUMVis {
    /**
     * Widget for a IntProperty
     */
    class FloatPropertyWidget : public AbstractPropertyWidget {
        Q_OBJECT;

    public:
        /**
         * Creates a new FloatPropertyWidget for the property \a property.
         * \param   property    The property the widget shall handle
         * \param   parent      Parent Qt widget
         */
        FloatPropertyWidget(FloatProperty* property, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~FloatPropertyWidget();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();

    private slots:
        void onValueChanged(double value);

    private:
        /// Slot getting called when the property's min or max value has changed, so that the widget can be updated.
        virtual void onPropertyMinMaxChanged(const AbstractProperty* property);

        QDoubleSpinBox* _spinBox;

    };
}

#endif // FLOATPROPERTYWIDGET_H__
