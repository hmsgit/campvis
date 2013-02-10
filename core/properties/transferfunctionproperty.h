// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#ifndef TRANSFERFUNCTIONPROPERTY_H__
#define TRANSFERFUNCTIONPROPERTY_H__

#include "sigslot/sigslot.h"
#include "core/properties/abstractproperty.h"
#include "core/classification/abstracttransferfunction.h"

namespace campvis {

    /**
     * \todo    Implement correct behavior if the TF changes during locked property state.
     */
    class TransferFunctionProperty : public AbstractProperty, public sigslot::has_slots<> {
    public:
        /**
         * Creates a new TransferFunctionProperty
         * \param name      Property name (unchangable!)
         * \param title     Property title (e.g. used for GUI)
         * \param tf        Transfer function to initialize the property with.
         * \param invalidationLevel  Invalidation level that this property triggers
         */
        TransferFunctionProperty(const std::string& name, const std::string& title, AbstractTransferFunction* tf, int invalidationLevel = AbstractProcessor::INVALID_RESULT);

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
         * Replaces the transfer function with \a tf.
         * TransferFunctionProperty takes ownership of \a tf.
         * \note    First triggers s_BeforeTFReplace, then deletes the current TF, replaces it with
         *          \a tf and finally triffers s_AfterTFReplace.
         * \param   tf  The new transfer function for this property. TransferFunctionProperty takes ownership.
         */
        void replaceTF(AbstractTransferFunction* tf);

        /**
         * Slot being called when \a _transferFunction has changed.
         */
        void onTFChanged();


        sigslot::signal1<AbstractTransferFunction*> s_BeforeTFReplace;
        sigslot::signal1<AbstractTransferFunction*> s_AfterTFReplace;

    protected:
        AbstractTransferFunction* _transferFunction;    ///< Transfer function of this property

        static const std::string loggerCat_;
    };

}

#endif // TRANSFERFUNCTIONPROPERTY_H__
