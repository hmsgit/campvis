// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#ifndef TRANSFERFUNCTIONPROPERTYWIDGET_H__
#define TRANSFERFUNCTIONPROPERTYWIDGET_H__

#include "application/applicationapi.h"
#include "application/gui/properties/abstractpropertywidget.h"
#include "application/gui/properties/propertywidgetfactory.h"
#include "core/properties/transferfunctionproperty.h"

class QDockWidget;
class QDoubleSpinBox;
class QGridLayout;
class QLabel;
class QPushButton;
class QCheckBox;

namespace campvis {
    class AbstractTransferFunctionEditor;

    /**
     * Widget for a TransferFunctionProperty
     */
    class CAMPVIS_APPLICATION_API TransferFunctionPropertyWidget : public AbstractPropertyWidget {
        Q_OBJECT;

    public:
        /**
         * Creates a new PropertyWidget for the property \a property.
         * \param   property        The property the widget shall handle
         * \param   dataContainer   DataContainer to use (optional), defaults to nullptr.
         * \param   parent          Parent Qt widget
         */
        TransferFunctionPropertyWidget(TransferFunctionProperty* property, DataContainer* dataContainer = nullptr, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~TransferFunctionPropertyWidget();


    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();
        
        /**
         * Slot to be called from property when the property's flag whether to auto fit the TF window has changed.
         */
        void onTransferFunctionAutoFitWindowToDataChanged();

    
    signals:
        void s_beforeTFReplace(AbstractTransferFunction* tf);
        void s_afterTFReplace(AbstractTransferFunction* tf);

    private slots:
        /// slot called when one of the intensity domain spin edits has changed
        void onDomainChanged(double value);
        /// slot called when edit TF button clicked
        void onEditClicked(bool checked);
        /// slot called when _btnFitDomainToImage clicked
        void onFitClicked(bool checked);
        /// slot called when _cbAutoFitDomainToImage changed
        void onAutoFitDomainToImageChanged(int state);

    protected slots:
        /// slot called when transfer function is about to be replaced
        void execBeforeTFReplace(AbstractTransferFunction *tf);
        /// slot called when transfer function just has replaced
        void execAfterTFReplace(AbstractTransferFunction *tf);
    
    private:
        /// catch slot for tf replace action
        void onBeforeTFReplace(AbstractTransferFunction *tf);
        /// catch slot for tf replace action
        void onAfterTFReplace(AbstractTransferFunction *tf);

    private:
        QWidget* _widget;                   ///< Widget grouping the widgets together
        QGridLayout* _gridLayout;           ///< Layout for _widget

        QLabel* _lblDomain;                 ///< intensity domain label
        QDoubleSpinBox* _spinDomainLeft;    ///< spin edit for intensity domain lower bound
        QDoubleSpinBox* _spinDomainRight;   ///< spin edit for intensity domain upper bound
        QCheckBox* _cbAutoFitDomainToImage; ///< Checkbox for the flag whether to automatically fit the TF domain to new image data
        QPushButton* _btnFitDomainToImage;  ///< button for fitting the intensity domain to the image 
        QPushButton* _btnEditTF;            ///< button for showing the TF editor widget

        QDockWidget* _dockWidget;                   ///< DockWidget for transfer function editor
        AbstractTransferFunctionEditor* _editor;    ///< Transfer function editor

        bool wasVisible;                    ///< remembers whether _docWidget was visible or not
    };

    // explicitly instantiate template, so that it gets registered also over DLL boundaries.
    template class PropertyWidgetRegistrar<TransferFunctionPropertyWidget, TransferFunctionProperty>;
}

#endif // TRANSFERFUNCTIONPROPERTYWIDGET_H__
