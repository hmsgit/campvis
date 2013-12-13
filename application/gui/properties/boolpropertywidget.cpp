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

#include "boolpropertywidget.h"

#include <QCheckBox>

namespace campvis {
    BoolPropertyWidget::BoolPropertyWidget(BoolProperty* property, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, false, parent)
        , _spinBox(0)
    {
        _spinBox = new QCheckBox(this);
        _spinBox->setChecked(property->getValue());
        
        addWidget(_spinBox);

        connect(_spinBox, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    }

    BoolPropertyWidget::~BoolPropertyWidget() {
    }

    void BoolPropertyWidget::updateWidgetFromProperty() {
        BoolProperty* prop = static_cast<BoolProperty*>(_property);
        _spinBox->blockSignals(true);
        _spinBox->setChecked(prop->getValue());
        _spinBox->blockSignals(false);
    }

    void BoolPropertyWidget::onStateChanged(int  value) {
        ++_ignorePropertyUpdates;
        BoolProperty* prop = static_cast<BoolProperty*>(_property);
        prop->setValue(_spinBox->isChecked());
        --_ignorePropertyUpdates;
    }

}