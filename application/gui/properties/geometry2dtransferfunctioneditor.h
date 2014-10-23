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

#ifndef GEOMETRY2DTRANSFERFUNCTIONEDITOR_H__
#define GEOMETRY2DTRANSFERFUNCTIONEDITOR_H__

#include "tbb/mutex.h"
#include "cgt/painter.h"
#include "cgt/event/eventlistener.h"
#include "core/classification/tfgeometry2d.h"
#include "application/gui/properties/abstracttransferfunctioneditor.h"

class QGridLayout;
class QLabel;
class QPushButton;

namespace cgt {
    class QtThreadedCanvas;
    class Shader;
}

namespace campvis {
    class ColorPickerWidget;
    class Geometry2DTransferFunction;
    class AbstractTFGeometryManipulator;
    class WholeTFGeometryManipulator;

    /**
     * Editor widget for a Geometry2DTransferFunction.
     */
    class Geometry2DTransferFunctionEditor : public AbstractTransferFunctionEditor, public cgt::EventListener, public cgt::Painter {
        Q_OBJECT;

    public:
        /**
         * Creates a new editor widget for the for the TransferFunctionProperty \a property.
         * \param   prop        TransferFunctionProperty to generate the editor for.
         * \param   tf          The transfer function the editor shall handle.
         * \param   parent      Parent Qt widget
         */
        Geometry2DTransferFunctionEditor(TransferFunctionProperty* prop, Geometry2DTransferFunction* tf, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~Geometry2DTransferFunctionEditor();

        /**
         * Schedule a repaint job for the editor's render target
         */
        virtual void repaint();

        /// \see cgt::Painter::sizeChanged
        virtual void sizeChanged(const cgt::ivec2&);

        /// \see cgt::EventListener::mousePressEvent
        virtual void mousePressEvent(cgt::MouseEvent* e);

        /**
         * Slot to be called when the geometry vector of the transfer function has changed.
         */
        void onGeometryCollectionChanged();

        /**
         * Slot to be called when the handled TF is about to be deleted.
         */
        void onTfAboutToBeDeleted();

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

    protected:
        /**
         * Disconnects this editor from the handled TF and cleans up everything.
         */
        void disconnectFromTf();

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
        tbb::mutex _localMutex;

        QGridLayout* _layout;

        cgt::QtThreadedCanvas* _canvas;
        QLabel* _lblIntensityLeft;
        QLabel* _lblIntensityRight;
        QPushButton* _btnAddGeometry;
        QPushButton* _btnRemoveGeometry;
    };
}

#endif // GEOMETRY2DTRANSFERFUNCTIONEDITOR_H__
