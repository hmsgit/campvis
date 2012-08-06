#ifndef TRANSFERFUNCTIONPROPERTYWIDGET_H__
#define TRANSFERFUNCTIONPROPERTYWIDGET_H__

#include "application/gui/properties/abstractpropertywidget.h"
#include "core/properties/transferfunctionproperty.h"

#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

namespace TUMVis {
    /**
     * Widget for a TransferFunctionProperty
     */
    class TransferFunctionPropertyWidget : public AbstractPropertyWidget {
        Q_OBJECT;

    public:
        /**
         * Creates a new PropertyWidget for the property \a property.
         * \param   property    The property the widget shall handle
         * \param   parent      Parent Qt widget
         */
        TransferFunctionPropertyWidget(TransferFunctionProperty* property, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~TransferFunctionPropertyWidget();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();

    private slots:
        /// slot called when one of the intensity domain spin edits has changed
        void onDomainChanged(double value);
        /// slot called when edit TF button clicked
        void onEditClicked(bool checked);

    private:
        QWidget* _widget;                   ///< Widget grouping the widgets together
        QGridLayout* _gridLayout;           ///< Layout for _widget

        QLabel* _lblDomain;                 ///< intensity domain label
        QDoubleSpinBox* _spinDomainLeft;    ///< spin edit for intensity domain lower bound
        QDoubleSpinBox* _spinDomainRight;   ///< spin edit for intensity domain upper bound
        QPushButton* _btnEditTF;            ///< button for showing the TF editor widget

    };
}

#endif // TRANSFERFUNCTIONPROPERTYWIDGET_H__
