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

#ifndef DATANAMEPROPERTYWIDGET_H__
#define DATANAMEPROPERTYWIDGET_H__

#include "sigslot/sigslot.h"
#include "core/properties/datanameproperty.h"
#include "application/gui/qtdatahandle.h"
#include "application/gui/properties/abstractpropertywidget.h"
#include "application/gui/properties/propertywidgetfactory.h"

class QComboBox;
class QLineEdit;

namespace campvis {
    class DataNameProperty;
    class DataContainer;
    class DataHandle;

    /**
     * Widget for a DataNameProperty
     */
    class DataNamePropertyWidget : public AbstractPropertyWidget {
        Q_OBJECT

    public:
        /**
         * Creates a new PropertyWidget for the property \a property.
         * \param   property    The property the widget shall handle
         * \param   dc          DataContainer this property uses to populate the combo box (may be 0).
         * \param   parent      Parent Qt widget
         */
        DataNamePropertyWidget(DataNameProperty* property, DataContainer* dc, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~DataNamePropertyWidget();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();

        void onDataAdded(std::string key, DataHandle dh);

    private slots:
        void onTextChanged(const QString& text);
        void onDataAddedQt(const QString& key, QtDataHandle dh);

    signals:
        void s_dataAddedQt(const QString& key, QtDataHandle dh);

    private:
        /**
         * Set the combo box's current text to the provided value.
         *
         * If \c text can already be found in the combo box, the corresponding item is made active. Otherwise, \c text
         * is set as the contents of the combo box's text edit.
         *
         * \param   text      text to be set in the combo box
         */
        void setCurrentComboBoxText(const QString& text);

        QLineEdit* _lineEdit;
        QComboBox* _combobox;

    };

    // explicitly instantiate template, so that it gets registered also over DLL boundaries.
    template class PropertyWidgetRegistrar<DataNamePropertyWidget, DataNameProperty, 10>;
}

#endif // DATANAMEPROPERTYWIDGET_H__
