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

#ifndef GEOMETRY2DTRANSFERFUNCTIONEDITOR_H__
#define GEOMETRY2DTRANSFERFUNCTIONEDITOR_H__

#include "tbb/mutex.h"
#include "tgt/painter.h"
#include "tgt/event/eventlistener.h"
#include "core/classification/tfgeometry2d.h"
#include "application/gui/properties/abstracttransferfunctioneditor.h"

class QGridLayout;
class QLabel;
class QPushButton;

namespace tgt {
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
    class Geometry2DTransferFunctionEditor : public AbstractTransferFunctionEditor, public tgt::EventListener, public tgt::Painter {
        Q_OBJECT;

    public:
        /**
         * Creates a new editor widget for the for the TransferFunctionProperty \a property.
         * \param   tf          The transfer function the editor shall handle.
         * \param   parent      Parent Qt widget
         */
        Geometry2DTransferFunctionEditor(Geometry2DTransferFunction* tf, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~Geometry2DTransferFunctionEditor();

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
        tbb::mutex _localMutex;

        QGridLayout* _layout;

        tgt::QtThreadedCanvas* _canvas;
        QLabel* _lblIntensityLeft;
        QLabel* _lblIntensityRight;
        QPushButton* _btnAddGeometry;
        QPushButton* _btnRemoveGeometry;
    };
}

#endif // GEOMETRY2DTRANSFERFUNCTIONEDITOR_H__
