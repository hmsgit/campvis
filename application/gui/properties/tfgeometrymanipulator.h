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

#ifndef TFGEOMETRYMANIPULATOR_H__
#define TFGEOMETRYMANIPULATOR_H__

#include "tgt/vector.h"
#include "tgt/matrix.h"
#include "tgt/event/eventlistener.h"
#include "core/classification/tfgeometry1d.h"
#include "core/classification/tfgeometry2d.h"

namespace campvis {
    class Geometry1DTransferFunction;

    /**
     * Abstract base class for TFGeometry1D manipulators.
     * TFGeometry1D manipulators encapsulate a TF Geometry (or a part of it) and offer methods
     * for manipulating the TFGeometry1D (which is handy e.g. for TF editors...). Therefore, it
     * extends the tgt::EventListener class and should implement its methods as necessary.
     */
    class AbstractTFGeometryManipulator : public tgt::EventListener {
    public:
        /**
         * Creates a new AbstractTFGeometryManipulator for the given Geometry1DTransferFunction.
         * \param   viewportSize    Viewport size (extent of the OpenGL rendering canvas)
         */
        AbstractTFGeometryManipulator(const tgt::ivec2& viewportSize);

        /**
         * Pure virtual Destructor
         */
        virtual ~AbstractTFGeometryManipulator() {};

        /**
         * Sets the new viewport size.
         * \param   viewportSize    The new Viewport size (extent of the OpenGL rendering canvas)
         */
        void setViewportSize(const tgt::ivec2& viewportSize);

        /**
         * Renders the manipulator to the current OpenGL context.
         * \note    Must be called from an active and valid OpenGL context.
         */
        virtual void render() = 0;

    protected:
        /**
         * Transforms TF coordinates to viewport coordinates.
         * \param   pos     Coordinates in TF coordinate system.
         * \return  \a pos converted to viewport coordinates.
         */
        tgt::ivec2 tfToViewport(const tgt::vec2& pos) const;

        /**
         * Transforms viewport coordinates to TF coordinates.
         * \param   pos     Coordnates in viewport coordinate system
         * \return  \a pos converted to TF coordinates
         */
        tgt::vec2 viewportToTF(const tgt::ivec2& pos) const;

        tgt::ivec2 _viewportSize;               ///< Viewport size (extent of the OpenGL rendering canvas)
    };

// ================================================================================================

    /**
     * Class for manipulating the KeyPoints of a TFGeometry1D.
     */
    class KeyPointManipulator : public AbstractTFGeometryManipulator {
    public:
        /**
         * Creates a new KeyPointManipulator
         * \param   viewportSize    Viewport size (extent of the OpenGL rendering canvas)
         * \param   geometry        Parent geometry of the KeyPoint to manipulate
         * \param   keyPoint        Iterator to the KeyPoint to manipulate
         */
        KeyPointManipulator(const tgt::ivec2& viewportSize, TFGeometry1D* geometry, const std::vector<TFGeometry1D::KeyPoint>::iterator& keyPoint);

        /// \see AbstractTFGeometryManipulator::render
        void render();

        /// \see tgt::EventListener::mousePressEvent
        virtual void mousePressEvent(tgt::MouseEvent* e);
        /// \see tgt::EventListener::mouseReleaseEvent
        virtual void mouseReleaseEvent(tgt::MouseEvent* e);
        /// \see tgt::EventListener::mouseMoveEvent
        virtual void mouseMoveEvent(tgt::MouseEvent* e);
        /// \see tgt::EventListener::mouseDoubleClickEvent
        virtual void mouseDoubleClickEvent(tgt::MouseEvent* e);

    protected:
        static const int MANIPULATOR_SIZE;                          ///< Size of the manipulator
        TFGeometry1D* _geometry;                                    ///< Parent geometry of the KeyPoint to manipulate
        std::vector<TFGeometry1D::KeyPoint>::iterator _keyPoint;    ///< Iterator to the KeyPoint to manipulate

        // event handling stuff:
        bool _mousePressed;                     ///< Flag whether the mouse button is currently pressed
        //TFGeometry1D::KeyPoint _valueWhenPressed; ///< KeyPointValue when pressed
    };

// ================================================================================================

    /**
     * Class for manipulating the whole TFGeometry1D at once.
     */
    class WholeTFGeometryManipulator : public AbstractTFGeometryManipulator, public sigslot::has_slots<> {
    public:
        /**
         * Creates a new KeyPointManipulator
         * \param   viewportSize    Viewport size (extent of the OpenGL rendering canvas)
         * \param   geometry        Parent geometry of the KeyPoint to manipulate
         */
        WholeTFGeometryManipulator(const tgt::ivec2& viewportSize, TFGeometry1D* geometry);

        /**
         * Destructor
         */
        virtual ~WholeTFGeometryManipulator();

        /**
         * Returns the Parent geometry of the KeyPoint to manipulate.
         * \return  _geometry
         */
        TFGeometry1D* getGeometry() const;

        /**
         * Returns the vector caching the 2D coordinates of the TF key points.
         * \return  _helperPoints
         */
        const std::vector<tgt::vec2>& getHelperPoints() const;

        /// \see AbstractTFGeometryManipulator::render
        void render();

        /// \see tgt::EventListener::mousePressEvent
        virtual void mousePressEvent(tgt::MouseEvent* e);
        /// \see tgt::EventListener::mouseReleaseEvent
        virtual void mouseReleaseEvent(tgt::MouseEvent* e);
        /// \see tgt::EventListener::mouseMoveEvent
        virtual void mouseMoveEvent(tgt::MouseEvent* e);
        /// \see tgt::EventListener::mouseDoubleClickEvent
        virtual void mouseDoubleClickEvent(tgt::MouseEvent* e);

        /**
         * Slot to be called when the geometry has changed.
         */
        void onGeometryChanged();

        /// Signal to be emitted when the mouse was pressed and has hit this manipulator
        sigslot::signal1<WholeTFGeometryManipulator*> s_selected;

    protected:
        /**
         * Checks whether \a position is within the geometry.
         * \param   position    Position to check in TF coordinate system
         * \return  true if \a position is within the bounds of this geometry.
         */
        bool insideGeometry(const tgt::vec2& position) const;

        /**
         * Updates the helper points.
         */
        void updateHelperPoints();

        TFGeometry1D* _geometry;                                ///< Parent geometry of the KeyPoint to manipulate
        std::vector<tgt::vec2> _helperPoints;                   ///< vector caching the 2D coordinates of the TF key points

        // event handling stuff:
        bool _mousePressed;                                     ///< Flag whether the mouse button is currently pressed
        tgt::vec2 _pressedPosition;                             ///< Position where mousedown occured, in TF coordinates
        std::vector<TFGeometry1D::KeyPoint> _valuesWhenPressed; ///< KeyPoints when pressed
    };

}

#endif // TFGEOMETRYMANIPULATOR_H__
