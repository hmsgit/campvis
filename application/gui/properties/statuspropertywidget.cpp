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

#include "statuspropertywidget.h"

#include <QLabel>

namespace campvis {
    const char * StatusPropertyWidget::baseStyle = "text-align: center; padding: 1px; font-weight: bold;";

    StatusPropertyWidget::StatusPropertyWidget(StatusProperty * property, DataContainer* dataContainer /*= nullptr*/, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, false, dataContainer, parent)
    {
        _statusLabel = new QLabel(this);
        _statusLabel->setText(QString::fromStdString(property->getValue().string));
        _statusLabel->setStyleSheet(getStyleFromStatusType(property->getValue().status));
        _statusLabel->setMinimumWidth(100);
        _statusLabel->setMaximumWidth(100);
        _statusLabel->setAlignment(Qt::AlignCenter);

        addWidget(new QWidget(this), 1); //spacer item to force right-alignment
        addWidget(_statusLabel, 0);        
        //_statusLabel->setStyleSheet();
    }

    StatusPropertyWidget::~StatusPropertyWidget() {

    }

    void StatusPropertyWidget::updateWidgetFromProperty() {
        StatusProperty * prop = static_cast<StatusProperty*>(_property);
        QString qs = QString::fromStdString(prop->getValue().string);
        _statusLabel->setText(qs);
        _statusLabel->setStyleSheet(getStyleFromStatusType(prop->getValue().status));
    }

    QString StatusPropertyWidget::getStyleFromStatusType(Status::StatusType type) const {
        switch (type) {
        case Status::UNKNOWN:
            return QString("color: black; background-color: grey; border: 1px solid black;") + baseStyle;
        case Status::OK:
            return QString("color: black; background-color: green; border: 1px solid darkgreen;") + baseStyle;
        case Status::WARNING:
            return QString("color: black; background-color: orange; border: 1px solid darkorange;") + baseStyle;
        case Status::CRITICAL:
            return QString("color: black; background-color: red; border: 1px solid darkred; text-align: center; padding: 2px;") + baseStyle;
        default:
            return QString("color: black; background-color: white; border: 1px solid grey;") + baseStyle;
        }
    };

}