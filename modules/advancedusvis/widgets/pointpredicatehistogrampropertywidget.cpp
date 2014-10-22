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


#include "tgt/assert.h"

#include "pointpredicatehistogrampropertywidget.h"
#include "core/tools/stringutils.h"

#include <QPainter>
#include <QPen>
#include <QBrush>

#include <QGridLayout>

namespace campvis {

    PointPredicateHistogramPropertyWidget::PointPredicateHistogramPropertyWidget(PointPredicateHistogramProperty* property, DataContainer* dc, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, true, dc, parent)
        , _pcw(0)
    {
        QWidget* mainWidget = new QWidget(this);
        QGridLayout* layout = new QGridLayout();
        mainWidget->setLayout(layout);

        QStringList predicateList;
        predicateList.push_back(tr("Intensity Range"));
        predicateList.push_back(tr("Gradient Magnitude Range"));
        predicateList.push_back(tr("Gradient Orientation"));
        //predicateList.push_back(tr("Silhouette View"));
        predicateList.push_back(tr("SNR Range"));
        predicateList.push_back(tr("Vesselness Range"));
        //predicateList.push_back(tr("Ultrasound Confidence Range"));
        predicateList.push_back(tr("Segment ID"));

        _lwPredicates = new QListWidget(mainWidget);
        _lwPredicates->addItems(predicateList);
        _lwPredicates->setSelectionBehavior(QAbstractItemView::SelectRows);
        _lwPredicates->setSelectionMode(QAbstractItemView::ExtendedSelection);
        _lwPredicates->setMaximumHeight(128);
        layout->addWidget(_lwPredicates, 0, 0, 1, 3);

        QWidget* narf = new QWidget(this);
        QHBoxLayout *nl = new QHBoxLayout();
        narf->setLayout(nl);
        nl->setMargin(0);

        _btnAdd = new QPushButton("Add Predicate");
        nl->addWidget(_btnAdd);

        _btnReset = new QPushButton("Reset Predicates");
        nl->addWidget(_btnReset);

        layout->addWidget(narf, 1, 0, 1, 3);

        _lwPredicates->setVisible(false);
        narf->setVisible(false);
        
        _slHue = new QSlider(Qt::Vertical, mainWidget);
        _slHue->setMinimum(0);
        _slHue->setMaximum(100);
        layout->addWidget(_slHue, 3, 0);

        QLabel* lblHue = new QLabel("Hue");
        layout->addWidget(lblHue, 4, 0);

        _slSaturation = new QSlider(Qt::Vertical, mainWidget);
        _slSaturation->setMinimum(0);
        _slSaturation->setMaximum(100);
        layout->addWidget(_slSaturation, 3, 1);

        QLabel* lblSaturation = new QLabel("Sat");
        layout->addWidget(lblSaturation, 4, 1);

        _canvas = new PointPredicateRenderArea(property, this);
        layout->addWidget(_canvas, 3, 2, 2, 1);

        _pcw = new PropertyCollectionWidget(this);
        layout->addWidget(_pcw, 5, 0, 1, 3);

        addWidget(mainWidget);

        getProp()->s_headerChanged.connect(this, &PointPredicateHistogramPropertyWidget::onHistogramHeaderChanged);

        connect(_btnAdd, SIGNAL(clicked(bool)), this, SLOT(onBtnAddClicked(bool)));
        connect(_btnReset, SIGNAL(clicked(bool)), this, SLOT(onBtnResetClicked(bool)));
        connect(_canvas, SIGNAL(selectedPredicateChanged(AbstractPointPredicate*)), this, SLOT(onSelectedPredicateChanged(AbstractPointPredicate*)));
        connect(_slHue, SIGNAL(valueChanged(int)), this, SLOT(onHueValueChanged(int)));
        connect(_slSaturation, SIGNAL(valueChanged(int)), this, SLOT(onSaturationValueChanged(int)));
    }

    PointPredicateHistogramPropertyWidget::~PointPredicateHistogramPropertyWidget() {
        getProp()->s_headerChanged.disconnect(this);
    }

    void PointPredicateHistogramPropertyWidget::updateWidgetFromProperty() {
        _canvas->update();
    }

    void PointPredicateHistogramPropertyWidget::onSelectedPredicateChanged(AbstractPointPredicate* predicate) {
        _selectedPredicate = predicate;
        _pcw->updatePropCollection(_selectedPredicate, _dataContainer);

        if (_selectedPredicate == nullptr) {
            _slHue->setEnabled(false);
            _slHue->setValue(0);
            _slSaturation->setEnabled(false);
            _slSaturation->setValue(0);
        }
        else {
            _slHue->setEnabled(true);
            _slSaturation->setEnabled(true);

            _slHue->setValue(_selectedPredicate->p_color.getValue().x * 100);
            _slSaturation->setValue(_selectedPredicate->p_color.getValue().y * 100);
        }
    }

    void PointPredicateHistogramPropertyWidget::onHueValueChanged(int value) {
        if (_selectedPredicate != nullptr) {
            tgt::vec2 c = _selectedPredicate->p_color.getValue();
            c.x = static_cast<float>(value) / 100.f;
            _selectedPredicate->p_color.setValue(c);
        }
    }

    void PointPredicateHistogramPropertyWidget::onSaturationValueChanged(int value) {
        if (_selectedPredicate != nullptr) {
            tgt::vec2 c = _selectedPredicate->p_color.getValue();
            c.y = static_cast<float>(value) / 100.f;
            _selectedPredicate->p_color.setValue(c);
        }
    }

    void PointPredicateHistogramPropertyWidget::onBtnAddClicked(bool) {
        QList<QListWidgetItem*> selectedItems = _lwPredicates->selectedItems();
        if (selectedItems.empty())
            return;

        AbstractPointPredicate* predicateToAdd = nullptr;
        if (selectedItems.count() > 1) {
            std::vector<AbstractPointPredicate*> preds;
            for (int i = 0; i < selectedItems.size(); ++i)
                preds.push_back(createPredicate(selectedItems[i]));

            std::string title = preds.front()->getTitle();
            std::string name = preds.front()->getName();
            for (size_t i = 1; i < preds.size(); ++i) {
                title += " & " + preds[i]->getTitle();
                name += "_" + preds[i]->getName();
            }

            predicateToAdd = new AndCombinedPointPredicate(name, title, preds);
        }
        else {
            predicateToAdd = createPredicate(selectedItems[0]);
        }
        getProp()->getPredicateHistogram()->addPredicate(predicateToAdd);
    }

    AbstractPointPredicate* PointPredicateHistogramPropertyWidget::createPredicate(QListWidgetItem* item) {
        size_t numPreds = getProp()->getPredicateHistogram()->getPredicates().size();
        AbstractPointPredicate* vpToAdd = nullptr;

        if (item->text() == tr("Intensity Range")) {
            vpToAdd = new RangePointPredicate("intensity", "Intensity", "Intensity Range");
        }
        else if (item->text() == tr("Gradient Magnitude Range")) {
            vpToAdd = new RangePointPredicate("gradientMagnitude", "GradMag", "Gradient Magnitude Range");
        }
        else if (item->text() == tr("Gradient Orientation")) {
            vpToAdd = new RangePointPredicate("gradientAngle", "GradAngle", "Gradient Angle");
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setMaxValue(tgt::vec2(180.f, 180.f));
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setValue(tgt::vec2(80.f, 100.f));
        }
        else if (item->text() == tr("Silhouette View")) {
            vpToAdd = new RangePointPredicate("viewAngle", "Silhouette", "Silhouette View");
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setMaxValue(tgt::vec2(180.f, 180.f));
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setValue(tgt::vec2(85.f, 95.f));
        }
        else if (item->text() == tr("SNR Range")) {
            vpToAdd = new RangePointPredicate("snr", "SNR", "SNR Range");
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setMaxValue(tgt::vec2(10.f, 10.f));
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setValue(tgt::vec2(0.f, 10.f));
        }
        else if (item->text() == tr("Vesselness Range")) {
            vpToAdd = new RangePointPredicate("vesselness", "Vesselness", "Vesselness Range");
        }
        else if (item->text() == tr("Ultrasound Confidence Range")) {
            vpToAdd = new RangePointPredicate("confidence", "Confidence", "Confidence Range");
        }
        else if (item->text() == tr("Segment ID")) {
            vpToAdd = new LabelBitPointPredicate("label", "Label" + StringUtils::toString(numPreds), "Label ID " + StringUtils::toString(numPreds));
        }
        else {
            tgtAssert(false, "This should not happen.");
        }

        return vpToAdd;
    }

    void PointPredicateHistogramPropertyWidget::onHistogramHeaderChanged() {
        emit s_propertyChanged(_property);
    }

    void PointPredicateHistogramPropertyWidget::onBtnResetClicked(bool) {
        getProp()->getPredicateHistogram()->resetPredicates();
    }


}

