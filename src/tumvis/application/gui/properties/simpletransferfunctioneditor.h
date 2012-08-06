#ifndef SIMPLETRANSFERFUNCTIONEDITOR_H__
#define SIMPLETRANSFERFUNCTIONEDITOR_H__

#include "application/gui/properties/abstracttransferfunctioneditor.h"
#include <QWidget>

class QGridLayout;
class QLabel;

namespace TUMVis {
    class ColorPickerWidget;
    class SimpleTransferFunction;

    /**
     * Editor widget for a SimpleTransferFunction.
     */
    class SimpleTransferFunctionEditor : public AbstractTransferFunctionEditor {
        Q_OBJECT;

    public:
        /**
         * Creates a new editor widget for the for the TransferFunctionProperty \a property.
         * \param   tf          The transfer function the editor shall handle.
         * \param   parent      Parent Qt widget
         */
        SimpleTransferFunctionEditor(SimpleTransferFunction* tf, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~SimpleTransferFunctionEditor();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();

    private slots:
        void onLeftColorChanged(const QColor& color);
        void onRightColorChanged(const QColor& color);

    private:
        QGridLayout* _layout;

        QLabel* _lblColorLeft;
        QLabel* _lblColorRight;
        ColorPickerWidget* _cpwColorLeft;
        ColorPickerWidget* _cpwColorRight;
    };
}

#endif // SIMPLETRANSFERFUNCTIONEDITOR_H__
