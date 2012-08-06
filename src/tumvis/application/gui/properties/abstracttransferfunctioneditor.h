#ifndef ABSTRACTTRANSFERFUNCTIONEDITOR_H__
#define ABSTRACTTRANSFERFUNCTIONEDITOR_H__

#include "sigslot/sigslot.h"
#include <QBoxLayout>
#include <QLabel>
#include <QWidget>

namespace TUMVis {
    class AbstractProperty;
    class AbstractTransferFunction;

    /**
     * Abstract base class for transfer function editors.
     */
    class AbstractTransferFunctionEditor : public QWidget, public sigslot::has_slots<> {
        Q_OBJECT;

    public:
        /**
         * Creates a new editor widget for the for the AbstractTransferFunction \a tf.
         * \param   tf          The transfer function the editor shall handle.
         * \param   parent      Parent Qt widget
         */
        AbstractTransferFunctionEditor(AbstractTransferFunction* tf, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~AbstractTransferFunctionEditor();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty() = 0;

        AbstractTransferFunction* _transferFunction;    ///< The transfer function this widget handles

        // TODO: This flag is not thread-safe, it probably should be...
        bool _ignorePropertyUpdates;    ///< Flag whether the widget shall ignore incoming signals from properties being updated.

    private:
        /// Slot getting called when the transfer function has changed, so that the widget can be updated.
        virtual void onTFChanged();
    };
}

#endif // ABSTRACTTRANSFERFUNCTIONEDITOR_H__
