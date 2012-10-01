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

#include "tfgeometrymanipulator.h"

#include "tgt/assert.h"
#include "tgt/tgt_gl.h"
#include "tgt/event/mouseevent.h"

#include "application/gui/qtcolortools.h"
#include "core/classification/geometrytransferfunction.h"
#include "core/classification/tfgeometry.h"
#include "core/tools/algorithmicgeometry.h"

#include <QColorDialog>
#include <algorithm>

namespace TUMVis {

    AbstractTFGeometryManipulator::AbstractTFGeometryManipulator(const tgt::ivec2& viewportSize, GeometryTransferFunction* tf)
        : _viewportSize(viewportSize)
        , _tf(tf)
    {
        tgtAssert(tf != 0, "Transfer Function must not be 0.");
    }

    void AbstractTFGeometryManipulator::setViewportSize(const tgt::ivec2& viewportSize) {
        _viewportSize = viewportSize;
    }

    tgt::ivec2 AbstractTFGeometryManipulator::tfToViewport(const tgt::vec2& pos) const {
        // TF coordinate system is expected to be [0, 1]^n
        return tgt::ivec2(tgt::iround(pos.x * _viewportSize.x), tgt::iround(pos.y * _viewportSize.y));
    }

    tgt::vec2 AbstractTFGeometryManipulator::viewportToTF(const tgt::ivec2& pos) const {
        // TF coordinate system is expected to be [0, 1]^n
        return tgt::vec2(static_cast<float>(pos.x) / static_cast<float>(_viewportSize.x), static_cast<float>(pos.y) / static_cast<float>(_viewportSize.y));
    }

// ================================================================================================

    KeyPointManipulator::KeyPointManipulator(const tgt::ivec2& viewportSize, GeometryTransferFunction* tf, TFGeometry* geometry, const std::vector<TFGeometry::KeyPoint>::iterator& keyPoint)
        : AbstractTFGeometryManipulator(viewportSize, tf)
        , _geometry(geometry)
        , _keyPoint(keyPoint)
        , _mousePressed(false)
        //, _pressedPosition(0, 0)
    {
        tgtAssert(geometry != 0, "Geometry must not be 0.");
    }

    void KeyPointManipulator::render() {
        tgt::ivec2 pos = tfToViewport(tgt::vec2(_keyPoint->_position, static_cast<float>(_keyPoint->_color.a) / 255.f));
        glColor3ub(0, 0, 0);
        glBegin(GL_LINE_LOOP);
            glVertex2i(pos.x - MANIPULATOR_SIZE, pos.y - MANIPULATOR_SIZE);
            glVertex2i(pos.x + MANIPULATOR_SIZE, pos.y - MANIPULATOR_SIZE);
            glVertex2i(pos.x + MANIPULATOR_SIZE, pos.y + MANIPULATOR_SIZE);
            glVertex2i(pos.x - MANIPULATOR_SIZE, pos.y + MANIPULATOR_SIZE);
        glEnd();
    }

    void KeyPointManipulator::mousePressEvent(tgt::MouseEvent* e) {
        tgt::ivec2 kppos = tfToViewport(tgt::vec2(_keyPoint->_position, static_cast<float>(_keyPoint->_color.a) / 255.f));
        if ((abs(kppos.x - e->coord().x) < MANIPULATOR_SIZE) && (abs(kppos.y - _viewportSize.y + e->coord().y) < MANIPULATOR_SIZE)) {
            //_valueWhenPressed = *_keyPoint;
            _mousePressed = true;
            e->accept();
        }
        else {
            e->ignore();
        }
    }

    void KeyPointManipulator::mouseReleaseEvent(tgt::MouseEvent* e) {
        _mousePressed = false;
        // ignore here, because other listeners probably need this signal as well
        e->ignore();
    }

    void KeyPointManipulator::mouseMoveEvent(tgt::MouseEvent* e) {
        if (_mousePressed) {
            tgt::ivec2 currentPosition = tgt::clamp(tgt::ivec2(e->coord().x, _viewportSize.y - e->coord().y), tgt::ivec2(0, 0), _viewportSize);
            tgt::vec2 tfCoords = viewportToTF(currentPosition);

            _keyPoint->_position = tfCoords.x;
            _keyPoint->_color.a = static_cast<uint8_t>(tfCoords.y * 255.f);
            std::sort(_geometry->getKeyPoints().begin(), _geometry->getKeyPoints().end());
            _geometry->s_changed();
        }
        // ignore here, because other listeners probably need this signal as well
        e->ignore();
    }

    void KeyPointManipulator::mouseDoubleClickEvent(tgt::MouseEvent* e) {
        tgt::ivec2 kppos = tfToViewport(tgt::vec2(_keyPoint->_position, static_cast<float>(_keyPoint->_color.a) / 255.f));
        if ((abs(kppos.x - e->coord().x) < MANIPULATOR_SIZE) && (abs(kppos.y - _viewportSize.y + e->coord().y) < MANIPULATOR_SIZE)) {
            // launch a color picker dialog and set new color on success
            QColor newColor = QColorDialog::getColor(QtColorTools::toQColor(_keyPoint->_color), 0, "Select New Color");
            if(newColor.isValid()) {
                tgt::col4 tmp = QtColorTools::toTgtColor(newColor);
                _keyPoint->_color = tgt::col4(tmp.xyz(), _keyPoint->_color.a);
                _geometry->s_changed();
            }
            e->accept();
        }
        else {
            e->ignore();
        }
    }

    const int KeyPointManipulator::MANIPULATOR_SIZE = 5;

// ================================================================================================

    WholeTFGeometryManipulator::WholeTFGeometryManipulator(const tgt::ivec2& viewportSize, GeometryTransferFunction* tf, TFGeometry* geometry)
        : AbstractTFGeometryManipulator(viewportSize, tf)
        , _geometry(geometry)
        , _mousePressed(false)
    {
        tgtAssert(geometry != 0, "Geometry must not be 0.");
        _geometry->s_changed.connect(this, &WholeTFGeometryManipulator::onGeometryChanged);
        updateHelperPoints();
    }

    WholeTFGeometryManipulator::~WholeTFGeometryManipulator() {
        _geometry->s_changed.disconnect(this);
    }

    void WholeTFGeometryManipulator::render() {

    }

    TFGeometry* WholeTFGeometryManipulator::getGeometry() const {
        return _geometry;
    }

    const std::vector<tgt::vec2>& WholeTFGeometryManipulator::getHelperPoints() const {
        return _helperPoints;
    }

    void WholeTFGeometryManipulator::mousePressEvent(tgt::MouseEvent* e) {
        _pressedPosition = viewportToTF(tgt::ivec2(e->coord().x, _viewportSize.y - e->coord().y));
        if (insideGeometry(_pressedPosition)) {
            _mousePressed = true;
            _valuesWhenPressed = _geometry->getKeyPoints();
            s_selected(this);
            e->accept();
        }
        else { 
            e->ignore();
        }
    }

    void WholeTFGeometryManipulator::mouseReleaseEvent(tgt::MouseEvent* e) {
        _mousePressed = false;
        // ignore here, because other listeners probably need this signal as well
        e->ignore();
    }

    void WholeTFGeometryManipulator::mouseMoveEvent(tgt::MouseEvent* e) {
        if (_mousePressed) {
            tgt::vec2 currentPosition = viewportToTF(tgt::clamp(tgt::ivec2(e->coord().x, _viewportSize.y - e->coord().y), tgt::ivec2(0, 0), _viewportSize));
            tgt::vec2 displacement = currentPosition - _pressedPosition;

            for (size_t i = 0; i < _valuesWhenPressed.size(); ++i) {
                _geometry->getKeyPoints()[i]._position = _valuesWhenPressed[i]._position + displacement.x;
            }

            _geometry->s_changed();
        }
        // ignore here, because other listeners probably need this signal as well
        e->ignore();
    }

    void WholeTFGeometryManipulator::mouseDoubleClickEvent(tgt::MouseEvent* e) {
        tgt::vec2 pos = viewportToTF(tgt::ivec2(e->coord().x, _viewportSize.y - e->coord().y));
        if (insideGeometry(pos)) {
            // launch a color picker dialog and set new color on success
            QColor newColor = QColorDialog::getColor(QtColorTools::toQColor(_geometry->getKeyPoints().front()._color), 0, "Select New Color");
            if(newColor.isValid()) {
                tgt::col4 tmp = QtColorTools::toTgtColor(newColor);

                for (std::vector<TFGeometry::KeyPoint>::iterator it = _geometry->getKeyPoints().begin(); it != _geometry->getKeyPoints().end(); ++it) {
                    it->_color = tgt::col4(tmp.xyz(), it->_color.a);
                }
                _geometry->s_changed();
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
            bool operator() (const tgt::vec2& left, const tgt::vec2& right) {
                return left.x < right.x;
            };
        };
    }

    bool WholeTFGeometryManipulator::insideGeometry(const tgt::vec2& position) const {
        if (_helperPoints.size() < 2)
            return false;

        std::vector<tgt::vec2>::const_iterator lb = std::upper_bound(_helperPoints.begin(), _helperPoints.end(), position, XComparer());
        if (lb == _helperPoints.begin() || lb == _helperPoints.end())
            return false;

        return AlgorithmicGeometry::rightTurn2D(*(lb-1), *lb, position);
    }

    void WholeTFGeometryManipulator::updateHelperPoints() {
        _helperPoints.clear();
        const std::vector<TFGeometry::KeyPoint>& keyPoints = _geometry->getKeyPoints();

        if (keyPoints.front()._color.w > 0) {
            _helperPoints.push_back(tgt::vec2(keyPoints.front()._position, 0.f));
        }

        for (std::vector<TFGeometry::KeyPoint>::const_iterator it = keyPoints.begin(); it != keyPoints.end(); ++it) {
            float y = static_cast<float>(it->_color.a) / 255.f;
            _helperPoints.push_back(tgt::vec2(it->_position, y));
        }

        if (keyPoints.back()._color.w > 0) {
            _helperPoints.push_back(tgt::vec2(keyPoints.back()._position, 0.f));
        }
    }

}