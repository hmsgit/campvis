#ifndef STRINGPROPERTYWIDGET_H__
#define STRINGPROPERTYWIDGET_H__

#include "application/gui/properties/abstractpropertywidget.h"
#include "core/properties/genericproperty.h"

#include <QLineEdit>

namespace TUMVis {
    /**
     * Widget for a StringProperty
     */
    class StringPropertyWidget : public AbstractPropertyWidget {
        Q_OBJECT;

    public:
        /**
         * Creates a new PropertyWidget for the property \a property.
         * \param   property    The property the widget shall handle
         * \param   parent      Parent Qt widget
         */
        StringPropertyWidget(StringProperty* property, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~StringPropertyWidget();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();

    private slots:
        void onTextChanged(const QString& text);

    private:
        QLineEdit* _lineEdit;

    };
}
#endif // STRINGPROPERTYWIDGET_H__
