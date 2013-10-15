// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
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
            _btnLoadFile = new QPushButton(tr("Load File"), this);
            addWidget(_btnLoadFile);
            connect(_btnLoadFile, SIGNAL(clicked(bool)), this, SLOT(onBtnLoadFileClicked(bool)));
        }

        connect(_lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
    }

    StringPropertyWidget::~StringPropertyWidget() {

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