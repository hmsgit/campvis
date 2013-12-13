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

#include "camerapropertywidget.h"

#include <QGridLayout>
#include <QLabel>
#include "core/tools/stringutils.h"

namespace campvis {
    CameraPropertyWidget::CameraPropertyWidget(CameraProperty* property, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, true, parent)
        , _lblCameraPosition(0)
        , _lblLookDirection(0)
        , _lblUpVector(0)
    {
        _widget = new QWidget(this);
        QGridLayout* gridLayout = new QGridLayout(_widget);
        _widget->setLayout(gridLayout);

        _lblCameraPosition = new QLabel("Position: ", _widget);
        gridLayout->addWidget(_lblCameraPosition, 0, 0);
        _lblFocusPosition = new QLabel("Focus: ", _widget);
        gridLayout->addWidget(_lblFocusPosition, 1, 0);
        _lblLookDirection = new QLabel("Look Direction: ", _widget);
        gridLayout->addWidget(_lblLookDirection, 2, 0);
        _lblUpVector = new QLabel("Up Vector: ", _widget);
        gridLayout->addWidget(_lblUpVector, 3, 0);

        addWidget(_widget);
        updateWidgetFromProperty();
    }

    CameraPropertyWidget::~CameraPropertyWidget() {
    }

    void CameraPropertyWidget::updateWidgetFromProperty() {
        CameraProperty* prop = static_cast<CameraProperty*>(_property);
        _lblCameraPosition->setText("Position: " + QString::fromStdString(StringUtils::toString(prop->getValue().getPosition())));
        _lblFocusPosition->setText("Focus: " + QString::fromStdString(StringUtils::toString(prop->getValue().getFocus())));
        _lblLookDirection->setText("Look Direction: " + QString::fromStdString(StringUtils::toString(prop->getValue().getLook())));
        _lblUpVector->setText("Up Vector: " + QString::fromStdString(StringUtils::toString(prop->getValue().getUpVector())));
    }

}