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

#include "optionpropertywidget.h"

#include <QComboBox>
#include "core/properties/optionproperty.h"

namespace campvis {
    OptionPropertyWidget::OptionPropertyWidget(AbstractOptionProperty* property, DataContainer* dataContainer /*= nullptr*/, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, false, dataContainer, parent)
        , _comboBox(0)
    {
        _comboBox = new QComboBox(this);
        updateWidgetFromProperty();
        addWidget(_comboBox);

        connect(_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxIndexChanged(int)));
    }

    OptionPropertyWidget::~OptionPropertyWidget() {
        static_cast<IntProperty*>(_property)->s_minMaxChanged.disconnect(this);
    }

    void OptionPropertyWidget::updateWidgetFromProperty() {
        AbstractOptionProperty* prop = static_cast<AbstractOptionProperty*>(_property);
        _comboBox->blockSignals(true);
        _comboBox->clear();

        // build combo box from descriptions
        std::vector< std::pair<std::string, std::string> > options = prop->getOptionsAsPairOfStrings();
        for (std::vector< std::pair<std::string, std::string> >::iterator it = options.begin(); it != options.end(); ++it) {
            _comboBox->addItem(QString::fromStdString(it->second), QString::fromStdString(it->first));
        }

        // set selected options
        _comboBox->setCurrentIndex(prop->getValue());

        _comboBox->blockSignals(false);
    }

    void OptionPropertyWidget::onComboBoxIndexChanged(int value) {
        ++_ignorePropertyUpdates;
        AbstractOptionProperty* prop = static_cast<AbstractOptionProperty*>(_property);
        prop->setValue(value);
        --_ignorePropertyUpdates;
    }

}