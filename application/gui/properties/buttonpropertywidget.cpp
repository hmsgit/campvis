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

#include "buttonpropertywidget.h"

#include <QPushButton>

namespace campvis {
    ButtonPropertyWidget::ButtonPropertyWidget(ButtonProperty* property, DataContainer* dataContainer, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, false, dataContainer, parent)
        , _button(0)
    {
        setLabelVisibile(false);

        _button = new QPushButton(QString::fromStdString(property->getTitle()), this);
        connect(_button, SIGNAL(clicked(bool)), this, SLOT(onButtonClicked(bool)));
        addWidget(_button);
    }

    ButtonPropertyWidget::~ButtonPropertyWidget() {
    }

    void ButtonPropertyWidget::updateWidgetFromProperty() {
    }

    void ButtonPropertyWidget::onButtonClicked(bool) {
        ButtonProperty* bp = static_cast<ButtonProperty*>(_property);
        bp->click();
    }

}