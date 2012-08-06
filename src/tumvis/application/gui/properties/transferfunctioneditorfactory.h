#ifndef TRANSFERFUNCTIONEDITORFACTORY_H__
#define TRANSFERFUNCTIONEDITORFACTORY_H__

namespace TUMVis {
    class AbstractTransferFunction;
    class AbstractTransferFunctionEditor;

    class TransferFunctionEditorFactory {
    public:
        /**
         * 
         * \param property 
         * \return 
         */
        static AbstractTransferFunctionEditor* createEditor(AbstractTransferFunction* tf);
    };
}

#endif // TRANSFERFUNCTIONEDITORFACTORY_H__
