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

#ifndef POINTPREDICATERENDERAREA_H__
#define POINTPREDICATERENDERAREA_H__

#include "modules/advancedusvis/properties/pointpredicatehistogramproperty.h"

#include <QWidget>
#include <vector>

namespace campvis {

    /**
     * Widget that takes care of rendering the point predicate histogram and interaction metaphors.
     */
    class PointPredicateRenderArea : public QWidget {
        Q_OBJECT

    public:
        /**
         * Creates a new PointPredicateRenderArea for the given histogram.
         * \param   prop    Predicate histogram to render.
         * \param   parent  Parent widget (optional, defaults to 0).
         */
        PointPredicateRenderArea(PointPredicateHistogramProperty* prop, QWidget* parent = nullptr);

        /// Destructor
        virtual ~PointPredicateRenderArea();

        /// \see QWidget::minimumSizeHint()
        QSize minimumSizeHint() const;
        /// \see QWidget::sizeHint()
        QSize sizeHint() const;

    signals:
        /**
         * Signal emitted when the selected predicate has changed.
         * \param   predicate   Newly selected predicate.
         */
        void selectedPredicateChanged(AbstractPointPredicate* predicate);

    protected:
        /// \see QWidget::paintEvent()
        void paintEvent(QPaintEvent *event);

        /// \see QWidget::mouseMoveEvent()
        void mouseMoveEvent(QMouseEvent* e);
        /// \see QWidget::mousePressEvent()
        void mousePressEvent(QMouseEvent* e);
        /// \see QWidget::mouseReleaseEvent()
        void mouseReleaseEvent(QMouseEvent* e);

        /// \see QWidget::eventFilter()
        bool eventFilter(QObject* obj, QEvent* e);


        /**
         * Transforms viewport given corrdinates (x, y) to predicate histogram space (viewport without the margins).
         * \param   x   Widget viewport x coordinate.
         * \param   y   Widget viewport y coordinate.
         * \return  The given coordinate in histogram space.
         */
        QPoint transformToGraphSpace(int x, int y);

        /**
         * Returns the index of predicate under the given mouse position, -1 if there is none.
         * \param   e   Mouse event with coordinates in viewport space.
         * \return  The index of the point predicate under current mouse position, -1 if there is none.
         */
        int getPredicateUnderMouse(QMouseEvent* e);

        /**
         * Returns the index of predicate, whose top anchor (to drag) is under the given mouse position, -1 if there is none.
         * \param   e   Mouse event with coordinates in viewport space.
         * \return  The index of the point predicate, whose top anchor (to drag) is under current mouse position, -1 if there is none.
         */
        int getPredicateTopUnderMouse(QMouseEvent* e);

        /**
         * Returns the index of predicate, whose enabled button is under the given mouse position, -1 if there is none.
         * \param   e   Mouse event with coordinates in viewport space.
         * \return  The index of the point predicate, whose enabled button is under current mouse position, -1 if there is none.
         */
        int getPredicateEnabledUnderMouse(QMouseEvent* e);

        /**
         * Returns the point predicate with the given index.
         * \param   i   Index of the predicate to return, must be within bounds.
         * \return  Predicate with index \a i.
         */
        AbstractPointPredicate* getPredicate(int i);

    private:
        PointPredicateHistogramProperty* _prop;     ///< PointPredicateHistogramProperty to visualize

        int _movingPredicate;                       ///< index of currently dragged predicate (-1 if none)
        int _selectedPredicate;                     ///< index of currently selected predicate (-1 if none)
        std::vector<float> _originalImportances;    ///< cached predicate importances when predicate dragging started
    };

}


#endif // POINTPREDICATERENDERAREA_H__
