#ifndef PROPERTYCOLLECTIONWIDGET_H__
#define PROPERTYCOLLECTIONWIDGET_H__

#include <QList>
#include <QVBoxLayout>
#include <QWidget>

namespace TUMVis {
    class HasPropertyCollection;

    /**
     * Main Window for the TUMVis application.
     * Wraps a nice Qt GUI around the TumVisApplication instance given during creation.
     */
    class PropertyCollectionWidget : public QWidget {
        Q_OBJECT;

    public:
        /**
         * Creates a new PropertyCollectionWidget.
         * \param   parent  Parent widget, may be 0.
         */
        PropertyCollectionWidget(QWidget* parent = 0);

        /**
         * Destructor.
         */
        ~PropertyCollectionWidget();

    public slots:
        /**
         * Updates the property collection this widget works on.
         * \param   propertyCollection  New HasPropertyCollection instance for this widget, may be 0.
         */
        void updatePropCollection(HasPropertyCollection* propertyCollection);


    private:
        /**
         * Sets up this widget
         */
        void setupWidget();

        HasPropertyCollection* _propCollection;    ///< The HasPropertyCollection instance this widget is currently working on.
        QVBoxLayout* _layout;
        QList<QWidget*> _widgetList;
    };
}

#endif // PROPERTYCOLLECTIONWIDGET_H__
