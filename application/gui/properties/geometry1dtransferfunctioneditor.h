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

#ifndef GEOMETRY1DTRANSFERFUNCTIONEDITOR_H__
#define GEOMETRY1DTRANSFERFUNCTIONEDITOR_H__

#include "tbb/mutex.h"
#include "tgt/painter.h"
#include "tgt/event/eventlistener.h"
#include "core/classification/tfgeometry1d.h"
#include "application/gui/properties/abstracttransferfunctioneditor.h"

class QCheckBox;
class QGridLayout;
class QLabel;
class QPushButton;

namespace tgt {
    class QtThreadedCanvas;
    class Shader;
}

namespace campvis {
    class ColorPickerWidget;
    class Geometry1DTransferFunction;
    class AbstractTFGeometryManipulator;
    class WholeTFGeometryManipulator;

    /**
     * Editor widget for a Geometry1DTransferFunction.
     */
    class Geometry1DTransferFunctionEditor : public AbstractTransferFunctionEditor, public tgt::EventListener, public tgt::Painter {
        Q_OBJECT;

    public:
        /**
         * Creates a new editor widget for the for the TransferFunctionProperty \a property.
         * \param   prop        TransferFunctionProperty to generate the editor for.
         * \param   tf          The transfer function the editor shall handle.
         * \param   parent      Parent Qt widget
         */
        Geometry1DTransferFunctionEditor(TransferFunctionProperty* prop, Geometry1DTransferFunction* tf, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~Geometry1DTransferFunctionEditor();

        /**
         * Schedule a repaint job for the editor's render target
         */
        virtual void repaint();

        /// \see tgt::Painter::sizeChanged
        virtual void sizeChanged(const tgt::ivec2&);

        /// \see tgt::EventListener::mousePressEvent
        virtual void mousePressEvent(tgt::MouseEvent* e);

        /**
         * Slot to be called when the geometry vector of the transfer function has changed.
         */
        void onGeometryCollectionChanged();

        /**
         * Slot tp be called when a WholeTFGeometryManipulator was selected.
         * \param   the selected WholeTFGeometryManipulator
         */
        void onWholeTFGeometryManipulatorSelected(WholeTFGeometryManipulator* wtf /* :) */);

    protected slots:
        /**
         * Slot to be called when _btnAddGeometry was clicked.
         */
        void onBtnAddGeometryClicked();

        /**
         * Slot to be called when _btnRemoveGeometry was clicked.
         */
        void onBtnRemoveGeometryClicked();

        /**
         * Slot to be called when _cbLogScale's check state has changed.
         * \param state the checkbox's new Qt::CheckState.
         */
        void onCbLogScaleStateChanged(int state);

    protected:
        /**
         * Performs the painting.
         */
        virtual void paint();

        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();

        /**
         * Updates the GeometryManipulator vector. 
         * Invalidates all of its iterators.
         */
        void updateManipulators();

        /**
         * To be called when the canvas is invalidated, issues new paint job.
         */
        void invalidate();

        /**
         * Sets up the GUI stuff.
         */
        void setupGUI();

        std::vector<AbstractTFGeometryManipulator*> _manipulators;
        tbb::atomic<WholeTFGeometryManipulator*> _selectedGeometry;
        bool _logScale;
        tbb::mutex _localMutex;

        QGridLayout* _layout;

        tgt::QtThreadedCanvas* _canvas;
        QLabel* _lblIntensityLeft;
        QLabel* _lblIntensityRight;
        QPushButton* _btnAddGeometry;
        QPushButton* _btnRemoveGeometry;
        QCheckBox* _cbLogScale;
    };
}

#endif // GEOMETRY1DTRANSFERFUNCTIONEDITOR_H__
