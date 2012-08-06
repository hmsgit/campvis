#ifndef INTPROPERTYWIDGET_H__
#define INTPROPERTYWIDGET_H__

#include "application/gui/properties/abstractpropertywidget.h"
#include "core/properties/numericproperty.h"

#include <QSpinBox>

namespace TUMVis {
    /**
     * Widget for a IntProperty
     */
    class IntPropertyWidget : public AbstractPropertyWidget {
        Q_OBJECT;

    public:
        /**
         * Creates a new IntPropertyWidget for the property \a property.
         * \param   property    The property the widget shall handle
         * \param   parent      Parent Qt widget
         */
        IntPropertyWidget(IntProperty* property, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~IntPropertyWidget();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();

    private slots:
        void onValueChanged(int value);

    private:
        /// Slot getting called when the property's min or max value has changed, so that the widget can be updated.
        virtual void onPropertyMinMaxChanged(const AbstractProperty* property);

        QSpinBox* _spinBox;

    };
}
#endif // INTPROPERTYWIDGET_H__
