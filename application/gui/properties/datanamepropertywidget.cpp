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

#include "datanamepropertywidget.h"
#include "core/datastructures/datacontainer.h"
#include "core/properties/datanameproperty.h"

#include <QComboBox>
#include <QLineEdit>

namespace campvis {
    DataNamePropertyWidget::DataNamePropertyWidget(DataNameProperty* property, DataContainer* dc, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, parent)
        , _lineEdit(0)
        , _combobox(0)
        , _dc(dc)
    {
        if (property->getAccessInfo() == DataNameProperty::READ) {
            _combobox = new QComboBox(this);
            _combobox->setEditable(true);

            if (dc != 0) {
                std::vector< std::pair<std::string, DataHandle> > tmp = dc->getDataHandlesCopy();
                QStringList sl;
                for (size_t i = 0; i < tmp.size(); ++i)
                    sl.append(QString::fromStdString(tmp[i].first));
                _combobox->addItems(sl);
                dc->s_dataAdded.connect(this, &DataNamePropertyWidget::onDataAdded);

                int idx = _combobox->findText(QString::fromStdString(property->getValue()));
                if (idx != -1)
                    _combobox->setCurrentIndex(idx);
                else
                    _combobox->setEditText(QString::fromStdString(property->getValue()));
            }


            addWidget(_combobox);
            connect(_combobox, SIGNAL(editTextChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
        }
        else {
            _lineEdit = new QLineEdit(this);
            _lineEdit->setText(QString::fromStdString(property->getValue()));
            addWidget(_lineEdit);
            connect(_lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
        }
    }

    DataNamePropertyWidget::~DataNamePropertyWidget() {
        if (_dc != 0 && static_cast<DataNameProperty*>(_property)->getAccessInfo() == DataNameProperty::READ)
            _dc->s_dataAdded.disconnect(this);
    }

    void DataNamePropertyWidget::updateWidgetFromProperty() {
        DataNameProperty* prop = static_cast<DataNameProperty*>(_property);
        QString qs = QString::fromStdString(prop->getValue());
        if (_lineEdit->text() != qs) {
            _lineEdit->blockSignals(true);
            _lineEdit->setText(qs);
            _lineEdit->blockSignals(false);
        }
    }

    void DataNamePropertyWidget::onTextChanged(const QString& text) {
        DataNameProperty* prop = static_cast<DataNameProperty*>(_property);
        ++_ignorePropertyUpdates;
        prop->setValue(text.toStdString());
        --_ignorePropertyUpdates;
    }

    void DataNamePropertyWidget::onDataAdded(const std::string& key, const DataHandle& /*dh*/) {
        if (_combobox->findText(QString::fromStdString(key)) == -1) {
            _combobox->addItem(QString::fromStdString(key));
        }
    }


}