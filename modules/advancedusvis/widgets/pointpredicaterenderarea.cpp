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
#include "modules/advancedusvis/datastructures/pointpredicate.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFontMetrics>

namespace campvis {

    static const int MARGIN_LEFT = 15;
    static const int MARGIN_BOTTOM = 48;
    static const int MARGIN_TOP = 10;
    static const int MARGIN_RIGHT = 10;
    static const bool SHOW_ENABLE_ROW = false;
    static const int ENABLE_HEIGHT = (SHOW_ENABLE_ROW ? 16 : 0);

    PointPredicateRenderArea::PointPredicateRenderArea(PointPredicateHistogramProperty* prop, QWidget *parent /*= 0*/)
        : QWidget(parent)
        , _prop(prop)
        , _movingPredicate(-1)
        , _selectedPredicate(-1)
    {
        tgtAssert(prop != 0, "Property must not be 0");

        setBackgroundRole(QPalette::Base);
        setAutoFillBackground(true);

        qApp->installEventFilter(this);
    }

    PointPredicateRenderArea::~PointPredicateRenderArea() {
        qApp->removeEventFilter(this);
    }

    QSize PointPredicateRenderArea::minimumSizeHint() const {
        return QSize(300, 220);
    }

    QSize PointPredicateRenderArea::sizeHint() const {
         return QSize(300, 220);
    }

    void PointPredicateRenderArea::paintEvent(QPaintEvent *event) {

        int hw = width() - MARGIN_LEFT - MARGIN_RIGHT;
        int hh = height() - MARGIN_TOP - MARGIN_BOTTOM;

        QPainter painter(this);
        painter.setPen(QPen(Qt::black, 2.f));
        painter.setBrush(QBrush(Qt::NoBrush));
        painter.setRenderHint(QPainter::Antialiasing, true);

        // axes
        painter.drawLine(MARGIN_LEFT, 10, MARGIN_LEFT, height() - MARGIN_BOTTOM);
        painter.drawLine(MARGIN_LEFT, height() - MARGIN_BOTTOM, width() - 10, height() - MARGIN_BOTTOM);

        painter.drawText(2, MARGIN_TOP + 5, tr("1"));
        painter.drawText(2, MARGIN_TOP + hh + 5, tr("0"));

        // paint bars
        std::vector<AbstractPointPredicate*> preds = _prop->getPredicateHistogram()->getPredicates();
        double w = static_cast<double>(hw) / preds.size();

        for (int i = 0; i < static_cast<int>(preds.size()); ++i) {
            if (i == _selectedPredicate)
                painter.setPen(QPen(Qt::black, 2.f, Qt::DashDotDotLine));
            else
                painter.setPen(QPen(Qt::black, 2.f, Qt::SolidLine));

            QColor hsl = QColor::fromHslF(preds[i]->p_color.getValue().x, preds[i]->p_color.getValue().y, .75f);
            painter.setBrush(QBrush(hsl));
            painter.drawRect(QRectF(MARGIN_LEFT + i*w, height() - MARGIN_BOTTOM, w, static_cast<double>(-hh) * preds[i]->p_importance.getValue()));

            painter.setPen(QPen(Qt::black, 2.f, Qt::SolidLine));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(QRectF(MARGIN_LEFT + i*w, height() - MARGIN_BOTTOM, w, ENABLE_HEIGHT));

            QRectF daRect(QPointF(MARGIN_LEFT + i*w, height() - MARGIN_BOTTOM), QPointF(MARGIN_LEFT + (i+1)*w, height()));
            painter.drawText(daRect, Qt::AlignCenter | Qt::TextWordWrap, QString::fromStdString(preds[i]->getTitle()));            

            if (SHOW_ENABLE_ROW) {
                if (preds[i]->isEnabled()) {
                    const int BOXSIZE = (ENABLE_HEIGHT - 4) / 2;
                    painter.setPen(QPen(Qt::green, 2.f, Qt::SolidLine));
                    painter.drawLine(QPointF(MARGIN_LEFT + i*w + (w/2 - BOXSIZE), height() - MARGIN_BOTTOM + 4 + BOXSIZE), QPointF(MARGIN_LEFT + i*w + (w/2 - BOXSIZE) + 4, height() - MARGIN_BOTTOM + 4 + 2*BOXSIZE));
                    painter.drawLine(QPointF(MARGIN_LEFT + i*w + (w/2 - BOXSIZE) + 4, height() - MARGIN_BOTTOM + 4 + 2*BOXSIZE), QPointF(MARGIN_LEFT + i*w + (w/2 + BOXSIZE), height() - MARGIN_BOTTOM + 4));
                }
                else {
                    const int BOXSIZE = (ENABLE_HEIGHT - 4) / 2;
                    painter.setPen(QPen(Qt::red, 2.f, Qt::SolidLine));
                    painter.drawLine(QPointF(MARGIN_LEFT + i*w + (w/2 - BOXSIZE), height() - MARGIN_BOTTOM + 2), QPointF(MARGIN_LEFT + i*w + (w/2 + BOXSIZE), height() - MARGIN_BOTTOM + 2 + 2*BOXSIZE));
                    painter.drawLine(QPointF(MARGIN_LEFT + i*w + (w/2 - BOXSIZE), height() - MARGIN_BOTTOM + 2 + 2*BOXSIZE), QPointF(MARGIN_LEFT + i*w + (w/2 + BOXSIZE), height() - MARGIN_BOTTOM + 2));
                }
            }
        }

    }

    void PointPredicateRenderArea::mouseMoveEvent(QMouseEvent* e) {
        if (_movingPredicate >= 0) {
            int hh = height() - MARGIN_TOP - MARGIN_BOTTOM;
            double y = height() - e->y() - MARGIN_BOTTOM;
            float newValue = tgt::clamp(y/hh, 0.0, 1.0);
            
            float diff = _originalImportances[_movingPredicate] - newValue;
            float addendum = diff / (_originalImportances.size() - 1);

            std::vector<float> deltas(_originalImportances.size(), addendum);
            deltas[_movingPredicate] = -diff;
            _prop->adjustImportances(deltas, _originalImportances, _movingPredicate);
        }
        else {
            if (getPredicateTopUnderMouse(e) >= 0)
                setCursor(Qt::SizeVerCursor);
            else
                setCursor(Qt::ArrowCursor);
        }
    }

    void PointPredicateRenderArea::mousePressEvent(QMouseEvent* e) {
        _movingPredicate = getPredicateTopUnderMouse(e);
        if (_movingPredicate >= 0) {
            std::vector<AbstractPointPredicate*>& preds = _prop->getPredicateHistogram()->getPredicates();
            _originalImportances.resize(preds.size());

            for (size_t i = 0; i < preds.size(); ++i)
                _originalImportances[i] = preds[i]->p_importance.getValue();
        }

        int sp = getPredicateUnderMouse(e);
        if (e->button() == Qt::LeftButton) {
            if (sp != _selectedPredicate) {
                _selectedPredicate = sp;
                emit selectedPredicateChanged(_selectedPredicate >= 0 ? getPredicate(_selectedPredicate) : nullptr);
                update();
            }
        }
        else if (e->button() == Qt::RightButton) {
            emit selectedPredicateChanged(nullptr);
            if (sp >= 0) {
                std::vector<AbstractPointPredicate*>& preds = _prop->getPredicateHistogram()->getPredicates();
                float toRemove = preds[sp]->p_importance.getValue();

                std::vector<float> deltas(_prop->getPredicateHistogram()->getPredicates().size(), 0.f);
                for (int i = 0; i < static_cast<int>(deltas.size()); ++i) {
                    if (i == sp)
                        deltas[i] = -toRemove;
                    else 
                        deltas[i] = toRemove/(static_cast<int>(deltas.size()) - 1);
                }

                // set predicate importance to 0 before removal so that the sum remains 1.
                _prop->adjustImportances(deltas, _prop->getCurrentHistogramDistribution(), sp);
                _prop->getPredicateHistogram()->removePredicate(sp);
            }
            update();
        }
    }

    void PointPredicateRenderArea::mouseReleaseEvent(QMouseEvent* e) {
        if (_movingPredicate != -1) {
            int ep = getPredicateEnabledUnderMouse(e);
            if (ep >= 0) {
                std::vector<AbstractPointPredicate*>& preds = _prop->getPredicateHistogram()->getPredicates();
                preds[ep]->p_enable.setValue(! preds[ep]->p_enable.getValue());
            }
        }

        _movingPredicate = -1;
    }

    bool PointPredicateRenderArea::eventFilter(QObject* obj, QEvent* e) {
        if (e->type() == QEvent::MouseMove) {
            mouseMoveEvent(static_cast<QMouseEvent*>(e));
        }
        return false;
    }

    QPoint PointPredicateRenderArea::transformToGraphSpace(int x, int y) {
        return QPoint(x - MARGIN_LEFT, height() - y - MARGIN_BOTTOM);
    }

    int PointPredicateRenderArea::getPredicateTopUnderMouse(QMouseEvent* e) {
        int hw = width() - MARGIN_LEFT - MARGIN_RIGHT;
        int hh = height() - MARGIN_TOP - MARGIN_BOTTOM;

        std::vector<AbstractPointPredicate*> preds = _prop->getPredicateHistogram()->getPredicates();
        if (preds.empty())
            return -1;

        double x = e->x() - MARGIN_LEFT;
        double y = height() - e->y() - MARGIN_BOTTOM;

        if (x > 0 && x < hw) {
            int index = (x / hw) * preds.size();

            AbstractPointPredicate* daPredicate = preds[index];
            int py = (hh * daPredicate->p_importance.getValue());

            if (std::abs(py - y) < 5) {
                return index;
            }
        }

        return -1;
    }

    int PointPredicateRenderArea::getPredicateUnderMouse(QMouseEvent* e) {
        int hw = width() - MARGIN_LEFT - MARGIN_RIGHT;
        int hh = height() - MARGIN_TOP - MARGIN_BOTTOM;

        std::vector<AbstractPointPredicate*> preds = _prop->getPredicateHistogram()->getPredicates();
        if (preds.empty())
            return -1;

        double x = e->x() - MARGIN_LEFT;
        double y = height() - e->y() - MARGIN_BOTTOM;

        if (x > 0 && x < hw) {
            int index = (x / hw) * preds.size();

            AbstractPointPredicate* daPredicate = preds[index];
            int py = (hh * daPredicate->p_importance.getValue());

            if (y > 0 && y < py) {
                return index;
            }
        }

        return -1;
    }

    int PointPredicateRenderArea::getPredicateEnabledUnderMouse(QMouseEvent* e) {
        if (! SHOW_ENABLE_ROW)
            return -1;

        int hw = width() - MARGIN_LEFT - MARGIN_RIGHT;
        int hh = height() - MARGIN_TOP - MARGIN_BOTTOM;

        std::vector<AbstractPointPredicate*> preds = _prop->getPredicateHistogram()->getPredicates();
        if (preds.empty())
            return -1;

        double x = e->x() - MARGIN_LEFT;
        double y = e->y() - MARGIN_TOP - hh;

        if (y > 0 && y < ENABLE_HEIGHT) {
            if (x > 0 && x < hw) {
                int index = (x / hw) * preds.size();
                return index;
            }
        }

        return -1;
    }
    AbstractPointPredicate* PointPredicateRenderArea::getPredicate(int i) {
        std::vector<AbstractPointPredicate*> preds = _prop->getPredicateHistogram()->getPredicates();
        tgtAssert(i >= 0 && i < static_cast<int>(preds.size()), "Index out of bounds!");
        
        return preds[i];
    }


}
