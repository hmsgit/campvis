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

#include "stringpropertywidget.h"
#include "core/properties/datanameproperty.h"

#include <QLineEdit>
#include <QPushButton>
#include <QDir>
#include <QFileDialog>

namespace campvis {
    StringPropertyWidget::StringPropertyWidget(StringProperty* property, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, false, parent)
        , _lineEdit(0)
        , _btnLoadFile(0)
    {
        _lineEdit = new QLineEdit(this);
        _lineEdit->setText(QString::fromStdString(property->getValue()));
        addWidget(_lineEdit);

        if (! dynamic_cast<DataNameProperty*>(property)) {
            _btnLoadFile = new QPushButton(tr("Browse"), this);
            addWidget(_btnLoadFile);
            connect(_btnLoadFile, SIGNAL(clicked(bool)), this, SLOT(onBtnLoadFileClicked(bool)));
        }

        connect(_lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
    }

    StringPropertyWidget::~StringPropertyWidget() {

    }
    void StringPropertyWidget::setButtonLabel(const std::string& btnLabel) {
        this->_btnLoadFile->setText(QString(btnLabel.c_str()));
    }

    void StringPropertyWidget::updateWidgetFromProperty() {
        StringProperty* prop = static_cast<StringProperty*>(_property);
        QString qs = QString::fromStdString(prop->getValue());
        if (_lineEdit->text() != qs) {
            _lineEdit->blockSignals(true);
            _lineEdit->setText(qs);
            _lineEdit->blockSignals(false);
        }
    }

    void StringPropertyWidget::onTextChanged(const QString& text) {
        StringProperty* prop = static_cast<StringProperty*>(_property);
        ++_ignorePropertyUpdates;
        prop->setValue(text.toStdString());
        --_ignorePropertyUpdates;
    }

    void StringPropertyWidget::onBtnLoadFileClicked(bool flag) {
        StringProperty* prop = static_cast<StringProperty*>(_property);

        QString dialogCaption = QString::fromStdString(prop->getTitle());
        QString directory;
        // use directory of current property value if any, default directory otherwise
        if (! prop->getValue().empty())
            directory = QString::fromStdString(prop->getValue());
        else
            directory = tr("");

        const QString fileFilter = /*QString::fromStdString(property_->getFileFilter()) + ";;" + */tr("All files (*)");

        QString filename;
//        if (property_->getFileMode() == FileDialogProperty::OPEN_FILE) {
            filename = QFileDialog::getOpenFileName(QWidget::parentWidget(), dialogCaption, directory, fileFilter);
/*        }
        else if (property_->getFileMode() == FileDialogProperty::SAVE_FILE) {
            filename = QFileDialog::getSaveFileName(QWidget::parentWidget(), dialogCaption, directory, fileFilter);
        }
        else if (property_->getFileMode() == FileDialogProperty::DIRECTORY) {
            filename = QFileDialog::getExistingDirectory(QWidget::parentWidget(), dialogCaption, QString::fromStdString(property_->get()));
        }*/

        if (! filename.isEmpty()) {
            prop->setValue(filename.toStdString());
            //emit modified();
        }
    }

}