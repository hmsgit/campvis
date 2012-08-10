#ifndef TRANSFERFUNCTIONPROPERTY_H__
#define TRANSFERFUNCTIONPROPERTY_H__

#include "sigslot/sigslot.h"
#include "core/properties/abstractproperty.h"
#include "core/classification/abstracttransferfunction.h"

namespace TUMVis {

    class TransferFunctionProperty : public AbstractProperty, public sigslot::has_slots<> {
    public:
        /**
         * Creates a new TransferFunctionProperty
         * \param name      Property name (unchangable!)
         * \param title     Property title (e.g. used for GUI)
         * \param tf        Transfer function to initialize the property with.
         * \param il        Invalidation level that this property triggers
         */
        TransferFunctionProperty(const std::string& name, const std::string& title, AbstractTransferFunction* tf, InvalidationLevel il = InvalidationLevel::INVALID_RESULT);

        /**
         * Virtual Destructor
         **/
        virtual ~TransferFunctionProperty();

        /// \see AbstractProperty::deinit()
        virtual void deinit();

        /**
         * Gets the TransferFunction of this property.
         * \note    This is a non-const pointer, if you mess with it be sure you know what you're doing!
         * \return  _transferFunction
         */
        AbstractTransferFunction* getTF();

        /**
         * Slot being called when \a _transferFunction has changed.
         */
        void onTFChanged();


    protected:
        AbstractTransferFunction* _transferFunction;    ///< Transfer function of this property

        static const std::string loggerCat_;
    };

}

#endif // TRANSFERFUNCTIONPROPERTY_H__
