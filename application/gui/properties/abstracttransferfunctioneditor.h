// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
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

#ifndef ABSTRACTTRANSFERFUNCTIONEDITOR_H__
#define ABSTRACTTRANSFERFUNCTIONEDITOR_H__

#include "sigslot/sigslot.h"

#include "core/properties/transferfunctionproperty.h"

#include <QBoxLayout>
#include <QLabel>
#include <QWidget>

namespace campvis {
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
         * \param   prop        TransferFunctionProperty to generate the editor for.
         * \param   tf          The transfer function the editor shall handle.
         * \param   parent      Parent Qt widget
         */
        AbstractTransferFunctionEditor(TransferFunctionProperty* prop, AbstractTransferFunction* tf, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~AbstractTransferFunctionEditor();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty() = 0;

        /**
         * Tries to get the intensity histogram of the image stored in the Tf's property. May return 0.
         * \return  The intensity histogram of the image stored in the TF's property's data handle.
         */
        const TransferFunctionProperty::IntensityHistogramType* getIntensityHistogram() const;

        TransferFunctionProperty* _tfProperty;          ///< The parent TransferFunctionProperty of this editor
        AbstractTransferFunction* _transferFunction;    ///< The transfer function this widget handles

        /// Semaphore acts as flag whether the widget shall ignore incoming signals from properties being updated.
        tbb::atomic<int> _ignorePropertyUpdates;

    private:
        /// Slot getting called when the transfer function has changed, so that the widget can be updated.
        virtual void onTFChanged();
    };
}

#endif // ABSTRACTTRANSFERFUNCTIONEDITOR_H__
