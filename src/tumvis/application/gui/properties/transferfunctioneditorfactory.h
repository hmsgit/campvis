#ifndef TRANSFERFUNCTIONEDITORFACTORY_H__
#define TRANSFERFUNCTIONEDITORFACTORY_H__

namespace TUMVis {
    class AbstractTransferFunction;
    class AbstractTransferFunctionEditor;

    /**
     * Factory class offering the static method createEditor to create transfer function editors
     * for a given transfer function (depending on its type).
     */
    class TransferFunctionEditorFactory {
    public:
        /**
         * Creates the corresponding TransferFunctionEditor for the given transfer function \a tf.
         * \note    The callee has to take the ownership of the returned pointer.
         * \param   tf  Transfer function to generate the editor for.
         * \return  A new transfer function editor for the given transfer function (depending on its type).
         */
        static AbstractTransferFunctionEditor* createEditor(AbstractTransferFunction* tf);

    private:
        /// Shall not instantiate
        TransferFunctionEditorFactory();
    };
}

#endif // TRANSFERFUNCTIONEDITORFACTORY_H__
