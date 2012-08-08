#ifndef BOOLPROPERTYWIDGET_H__
#define BOOLPROPERTYWIDGET_H__

#include "application/gui/properties/abstractpropertywidget.h"
#include "core/properties/genericproperty.h"

class QCheckBox;

namespace TUMVis {
    /**
     * Widget for a IntProperty
     */
    class BoolPropertyWidget : public AbstractPropertyWidget {
        Q_OBJECT;

    public:
        /**
         * Creates a new BoolPropertyWidget for the property \a property.
         * \param   property    The property the widget shall handle
         * \param   parent      Parent Qt widget
         */
        BoolPropertyWidget(BoolProperty* property, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~BoolPropertyWidget();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();

    private slots:
        void onStateChanged(int value);

    private:
        QCheckBox* _spinBox;

    };
}

#endif // BOOLPROPERTYWIDGET_H__
