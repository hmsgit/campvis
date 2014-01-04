// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
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
    class TransferFunctionProperty : public AbstractProperty {
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
        /// \see AbstractProperty::addSharedProperty()
        virtual void addSharedProperty(AbstractProperty* prop);


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
        
        /**
         * Returns a DataHandle to the image for this transfer function, its pointer may be 0.
         * \note    If the data in \a imageHandle is not 0, it points to a valid ImageData object.
         * \return  _imageHandle, its pointer may be 0.
         */
        DataHandle getImageHandle() const;

        /**
         * Sets the DataHandle for this transfer function, its pointer may be 0.
         * \note    If the data in \a imageHandle is not 0, it must point to a valid ImageData object.
         * \param   imageHandle     The new DataHandle for this transfer function, if its pointer is 
         *                          not 0 it must point to a valid ImageData object.
         */
        void setImageHandle(DataHandle imageHandle);

        /**
         * Returns the flag whether to automatically fit the TF window to the data in the image handle.
         * \return  _autoFitWindowToData
         */
        bool getAutoFitWindowToData() const;

        /**
         * Sets the flag whether to automatically fit the TF window to the data in the image handle.
         * \param   newValue    New value of the flag whether to automatically fit the TF window to the data in the image handle.
         */
        void setAutoFitWindowToData(bool newValue);

        /// Signal emmitted directly before replacing the entire transfer function
        sigslot::signal1<AbstractTransferFunction*> s_BeforeTFReplace;
        /// Signal emmitted directly after replacing the entire transfer function
        sigslot::signal1<AbstractTransferFunction*> s_AfterTFReplace;

        /// Signal emitted when the image DataHandle for this TF has changed.
        sigslot::signal0<> s_imageHandleChanged;
        /// Signal emitted when the flag whether to automatically fit the TF window to the data in the image handle.
        sigslot::signal0<> s_autoFitWindowToDataChanged;

    protected:
        AbstractTransferFunction* _transferFunction;    ///< Transfer function of this property
        DataHandle _imageHandle;                        ///< DataHandle to the image for this transfer function. May be 0.
        bool _autoFitWindowToData;                      ///< Flag whether to automatically fit the TF window to the data in the image handle.

        static const std::string loggerCat_;
    };

}

#endif // TRANSFERFUNCTIONPROPERTY_H__
