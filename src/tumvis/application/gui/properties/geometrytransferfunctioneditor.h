// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef GEOMETRYTRANSFERFUNCTIONEDITOR_H__
#define GEOMETRYTRANSFERFUNCTIONEDITOR_H__

#include "tbb/include/tbb/mutex.h"
#include "tgt/painter.h"
#include "tgt/event/eventlistener.h"
#include "core/classification/tfgeometry.h"
#include "application/gui/properties/abstracttransferfunctioneditor.h"

class QGridLayout;
class QLabel;
class QPushButton;

namespace tgt {
    class QtThreadedCanvas;
    class Shader;
}

namespace TUMVis {
    class ColorPickerWidget;
    class GeometryTransferFunction;
    class AbstractTFGeometryManipulator;
    class WholeTFGeometryManipulator;

    /**
     * Editor widget for a GeometryTransferFunction.
     */
    class GeometryTransferFunctionEditor : public AbstractTransferFunctionEditor, public tgt::EventListener, public tgt::Painter {
        Q_OBJECT;

    public:
        /**
         * Creates a new editor widget for the for the TransferFunctionProperty \a property.
         * \param   tf          The transfer function the editor shall handle.
         * \param   parent      Parent Qt widget
         */
        GeometryTransferFunctionEditor(GeometryTransferFunction* tf, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~GeometryTransferFunctionEditor();

        
        /**
         * Performs the painting.
         */
        void paint();

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

    protected:
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

        tgt::QtThreadedCanvas* _canvas;
        QLabel* _lblIntensityLeft;
        QLabel* _lblIntensityRight;
        QPushButton* _btnAddGeometry;
        QPushButton* _btnRemoveGeometry;
    };
}

#endif // GEOMETRYTRANSFERFUNCTIONEDITOR_H__
