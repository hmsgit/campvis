#ifndef ABSTRACTPROPERTYWIDGET_H__
#define ABSTRACTPROPERTYWIDGET_H__

#include "sigslot/sigslot.h"
#include <QWidget>
#include <QBoxLayout>

namespace TUMVis {
    class AbstractProperty;

    /**
     * Abstract base class for property widgets.
     */
    class AbstractPropertyWidget : public QWidget, public sigslot::has_slots<> {
        Q_OBJECT;

    public:
        /**
         * Creates a new PropertyWidget for the property \a property.
         * \param   property    The property the widget shall handle
         * \param   parent      Parent Qt widget
         */
        AbstractPropertyWidget(AbstractProperty* property, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~AbstractPropertyWidget();


    public slots:

    protected:
        /**
         * Adds a widget to the local Qt layout.
         */
        void addWidget(QWidget* widget);

        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty() = 0;

        AbstractProperty* _property;    ///< The property this widget handles

        // TODO: This flag is not thread-safe, it probably should be...
        bool _ignorePropertyUpdates;    ///< Flag whether the widget shall ignore incoming signals from properties being updated.

    private:
        /// Slot getting called when the property has changed, so that the widget can be updated.
        virtual void onPropertyChanged(const AbstractProperty* property);

        QBoxLayout* _layout;
    };
}

#endif // ABSTRACTPROPERTYWIDGET_H__
