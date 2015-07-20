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

#include "datanamepropertywidget.h"
#include "core/datastructures/datacontainer.h"
#include "core/properties/datanameproperty.h"

#include <QComboBox>
#include <QLineEdit>
#include <QSortFilterProxyModel>

namespace campvis {
    DataNamePropertyWidget::DataNamePropertyWidget(DataNameProperty* property, DataContainer* dc, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, false, dc, parent)
        , _lineEdit(0)
        , _combobox(0)
    {
        if (property->getAccessInfo() == DataNameProperty::READ) {
            _combobox = new QComboBox(this);
            _combobox->setEditable(true);

            QSortFilterProxyModel* proxy = new QSortFilterProxyModel(_combobox);
            proxy->setSourceModel(_combobox->model());
            _combobox->model()->setParent(proxy); // combo's current model must be reparented, otherwise QComboBox::setModel() will delete it
            _combobox->setModel(proxy);

            if (dc != 0) {
                std::vector< std::pair<std::string, DataHandle> > tmp = dc->getDataHandlesCopy();
                QStringList sl;
                for (size_t i = 0; i < tmp.size(); ++i)
                    sl.append(QString::fromStdString(tmp[i].first));
                _combobox->addItems(sl);
                dc->s_dataAdded.connect(this, &DataNamePropertyWidget::onDataAdded);
                connect(this, SIGNAL(s_dataAddedQt(const QString&, QtDataHandle)), this, SLOT(onDataAddedQt(const QString&, QtDataHandle)));

                _combobox->model()->sort(Qt::AscendingOrder);
                setCurrentComboBoxText(QString::fromStdString(property->getValue()));
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
        if (_dataContainer != 0 && static_cast<DataNameProperty*>(_property)->getAccessInfo() == DataNameProperty::READ)
            _dataContainer->s_dataAdded.disconnect(this);
    }

    void DataNamePropertyWidget::updateWidgetFromProperty() {
        DataNameProperty* prop = static_cast<DataNameProperty*>(_property);
        QString qs = QString::fromStdString(prop->getValue());

        if (prop->getAccessInfo() == DataNameProperty::READ) {
            _combobox->blockSignals(true);
            setCurrentComboBoxText(qs);
            _combobox->blockSignals(false);
        }
        else if (_lineEdit->text() != qs) {
            _lineEdit->blockSignals(true);
            _lineEdit->setText(qs);
            _lineEdit->blockSignals(false);
        }
    }

    void DataNamePropertyWidget::setCurrentComboBoxText(const QString& text) {
        int idx = _combobox->findText(text);

        if (idx != -1)
            _combobox->setCurrentIndex(idx);
        else
            _combobox->setEditText(text);
    }

    void DataNamePropertyWidget::onTextChanged(const QString& text) {
        DataNameProperty* prop = static_cast<DataNameProperty*>(_property);
        ++_ignorePropertyUpdates;
        prop->setValue(text.toStdString());
        --_ignorePropertyUpdates;
    }

    void DataNamePropertyWidget::onDataAdded(std::string key, DataHandle dh) {
        emit s_dataAddedQt(QString::fromStdString(key), dh);
    }

    void DataNamePropertyWidget::onDataAddedQt(const QString& key, QtDataHandle dh) {
        if (_combobox->findText(key) == -1) {
            _combobox->addItem(key);
            _combobox->model()->sort(Qt::AscendingOrder);
        }
    }


}
