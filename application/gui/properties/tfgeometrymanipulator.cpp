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

#include "tfgeometrymanipulator.h"

#include "cgt/assert.h"
#include "cgt/cgt_gl.h"
#include "cgt/event/mouseevent.h"

#include "application/gui/qtcolortools.h"
#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"
#include "core/tools/algorithmicgeometry.h"

#include <QColorDialog>
#include <algorithm>

namespace campvis {

    AbstractTFGeometryManipulator::AbstractTFGeometryManipulator(const cgt::ivec2& viewportSize)
        : _viewportSize(viewportSize)
    {
    }

    void AbstractTFGeometryManipulator::setViewportSize(const cgt::ivec2& viewportSize) {
        _viewportSize = viewportSize;
    }

    cgt::ivec2 AbstractTFGeometryManipulator::tfToViewport(const cgt::vec2& pos) const {
        // TF coordinate system is expected to be [0, 1]^n
        return cgt::ivec2(cgt::iround(pos.x * _viewportSize.x), cgt::iround(pos.y * _viewportSize.y));
    }

    cgt::vec2 AbstractTFGeometryManipulator::viewportToTF(const cgt::ivec2& pos) const {
        // TF coordinate system is expected to be [0, 1]^n
        return cgt::vec2(static_cast<float>(pos.x) / static_cast<float>(_viewportSize.x), static_cast<float>(pos.y) / static_cast<float>(_viewportSize.y));
    }

// ================================================================================================

    KeyPointManipulator::KeyPointManipulator(const cgt::ivec2& viewportSize, TFGeometry1D* geometry, const std::vector<TFGeometry1D::KeyPoint>::iterator& keyPoint)
        : AbstractTFGeometryManipulator(viewportSize)
        , _geometry(geometry)
        , _keyPoint(keyPoint)
        , _mousePressed(false)
    {
        cgtAssert(geometry != 0, "Geometry must not be 0.");
    }

    void KeyPointManipulator::render() {
        cgt::ivec2 pos = tfToViewport(cgt::vec2(_keyPoint->_position, static_cast<float>(_keyPoint->_color.a) / 255.f));
        glColor3ub(0, 0, 0);
        glBegin(GL_LINE_LOOP);
            glVertex2i(pos.x - MANIPULATOR_SIZE, pos.y - MANIPULATOR_SIZE);
            glVertex2i(pos.x + MANIPULATOR_SIZE, pos.y - MANIPULATOR_SIZE);
            glVertex2i(pos.x + MANIPULATOR_SIZE, pos.y + MANIPULATOR_SIZE);
            glVertex2i(pos.x - MANIPULATOR_SIZE, pos.y + MANIPULATOR_SIZE);
        glEnd();
    }

    void KeyPointManipulator::mousePressEvent(cgt::MouseEvent* e) {
        cgt::ivec2 kppos = tfToViewport(cgt::vec2(_keyPoint->_position, static_cast<float>(_keyPoint->_color.a) / 255.f));
        if ((abs(kppos.x - e->coord().x) < MANIPULATOR_SIZE) && (abs(kppos.y - _viewportSize.y + e->coord().y) < MANIPULATOR_SIZE)) {
            //_valueWhenPressed = *_keyPoint;
            _mousePressed = true;
            e->accept();
        }
        else {
            e->ignore();
        }
    }

    void KeyPointManipulator::mouseReleaseEvent(cgt::MouseEvent* e) {
        _mousePressed = false;
        // ignore here, because other listeners probably need this signal as well
        e->ignore();
    }

    void KeyPointManipulator::mouseMoveEvent(cgt::MouseEvent* e) {
        if (_mousePressed) {
            cgt::ivec2 currentPosition = cgt::clamp(cgt::ivec2(e->coord().x, _viewportSize.y - e->coord().y), cgt::ivec2(0, 0), _viewportSize);
            cgt::vec2 tfCoords = viewportToTF(currentPosition);

            _keyPoint->_position = tfCoords.x;
            _keyPoint->_color.a = static_cast<uint8_t>(tfCoords.y * 255.f);
            std::sort(_geometry->getKeyPoints().begin(), _geometry->getKeyPoints().end());
            _geometry->s_changed.emitSignal();
        }
        // ignore here, because other listeners probably need this signal as well
        e->ignore();
    }

    void KeyPointManipulator::mouseDoubleClickEvent(cgt::MouseEvent* e) {
        cgt::ivec2 kppos = tfToViewport(cgt::vec2(_keyPoint->_position, static_cast<float>(_keyPoint->_color.a) / 255.f));
        if ((abs(kppos.x - e->coord().x) < MANIPULATOR_SIZE) && (abs(kppos.y - _viewportSize.y + e->coord().y) < MANIPULATOR_SIZE)) {
            // launch a color picker dialog and set new color on success
            QColor newColor = QColorDialog::getColor(QtColorTools::toQColor(_keyPoint->_color), 0, "Select New Color");
            if(newColor.isValid()) {
                cgt::col4 tmp = QtColorTools::toTgtColor(newColor);
                _keyPoint->_color = cgt::col4(tmp.xyz(), _keyPoint->_color.a);
                _geometry->s_changed.emitSignal();
            }
            e->accept();
        }
        else {
            e->ignore();
        }
    }

    const int KeyPointManipulator::MANIPULATOR_SIZE = 5;

// ================================================================================================

    WholeTFGeometryManipulator::WholeTFGeometryManipulator(const cgt::ivec2& viewportSize, TFGeometry1D* geometry)
        : AbstractTFGeometryManipulator(viewportSize)
        , _geometry(geometry)
        , _mousePressed(false)
    {
        cgtAssert(geometry != 0, "Geometry must not be 0.");
        _geometry->s_changed.connect(this, &WholeTFGeometryManipulator::onGeometryChanged);
        updateHelperPoints();
    }

    WholeTFGeometryManipulator::~WholeTFGeometryManipulator() {
        _geometry->s_changed.disconnect(this);
    }

    void WholeTFGeometryManipulator::render() {

    }

    TFGeometry1D* WholeTFGeometryManipulator::getGeometry() const {
        return _geometry;
    }

    const std::vector<cgt::vec2>& WholeTFGeometryManipulator::getHelperPoints() const {
        return _helperPoints;
    }

    void WholeTFGeometryManipulator::mousePressEvent(cgt::MouseEvent* e) {
        _pressedPosition = viewportToTF(cgt::ivec2(e->coord().x, _viewportSize.y - e->coord().y));
        if (insideGeometry(_pressedPosition)) {
            _mousePressed = true;
            _valuesWhenPressed = _geometry->getKeyPoints();
            s_selected.emitSignal(this);
            e->accept();
        }
        else { 
            e->ignore();
        }
    }

    void WholeTFGeometryManipulator::mouseReleaseEvent(cgt::MouseEvent* e) {
        _mousePressed = false;
        // ignore here, because other listeners probably need this signal as well
        e->ignore();
    }

    void WholeTFGeometryManipulator::mouseMoveEvent(cgt::MouseEvent* e) {
        if (_mousePressed) {
            cgt::vec2 currentPosition = viewportToTF(cgt::clamp(cgt::ivec2(e->coord().x, _viewportSize.y - e->coord().y), cgt::ivec2(0, 0), _viewportSize));
            cgt::vec2 displacement = currentPosition - _pressedPosition;

            for (size_t i = 0; i < _valuesWhenPressed.size(); ++i) {
                _geometry->getKeyPoints()[i]._position = _valuesWhenPressed[i]._position + displacement.x;
            }

            _geometry->s_changed.emitSignal();
        }
        // ignore here, because other listeners probably need this signal as well
        e->ignore();
    }

    void WholeTFGeometryManipulator::mouseDoubleClickEvent(cgt::MouseEvent* e) {
        cgt::vec2 pos = viewportToTF(cgt::ivec2(e->coord().x, _viewportSize.y - e->coord().y));
        if (insideGeometry(pos)) {
            // launch a color picker dialog and set new color on success
            QColor newColor = QColorDialog::getColor(QtColorTools::toQColor(_geometry->getKeyPoints().front()._color), 0, "Select New Color");
            if(newColor.isValid()) {
                cgt::col4 tmp = QtColorTools::toTgtColor(newColor);

                for (std::vector<TFGeometry1D::KeyPoint>::iterator it = _geometry->getKeyPoints().begin(); it != _geometry->getKeyPoints().end(); ++it) {
                    it->_color = cgt::col4(tmp.xyz(), it->_color.a);
                }
                _geometry->s_changed.emitSignal();
            }
            e->accept();
        }
        else {
            e->ignore();
        }
    }

    void WholeTFGeometryManipulator::onGeometryChanged() {
        updateHelperPoints();
    }

    namespace {
        struct XComparer {
            bool operator() (const cgt::vec2& left, const cgt::vec2& right) {
                return left.x < right.x;
            };
        };
    }

    bool WholeTFGeometryManipulator::insideGeometry(const cgt::vec2& position) const {
        if (_helperPoints.size() < 2)
            return false;

        std::vector<cgt::vec2>::const_iterator lb = std::upper_bound(_helperPoints.begin(), _helperPoints.end(), position, XComparer());
        if (lb == _helperPoints.begin() || lb == _helperPoints.end())
            return false;

        return AlgorithmicGeometry::rightTurn2D(*(lb-1), *lb, position);
    }

    void WholeTFGeometryManipulator::updateHelperPoints() {
        _helperPoints.clear();
        const std::vector<TFGeometry1D::KeyPoint>& keyPoints = _geometry->getKeyPoints();

        if (keyPoints.front()._color.w > 0) {
            _helperPoints.push_back(cgt::vec2(keyPoints.front()._position, 0.f));
        }

        for (std::vector<TFGeometry1D::KeyPoint>::const_iterator it = keyPoints.begin(); it != keyPoints.end(); ++it) {
            float y = static_cast<float>(it->_color.a) / 255.f;
            _helperPoints.push_back(cgt::vec2(it->_position, y));
        }

        if (keyPoints.back()._color.w > 0) {
            _helperPoints.push_back(cgt::vec2(keyPoints.back()._position, 0.f));
        }
    }

}