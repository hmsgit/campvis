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

#include "tfgeometry2d.h"

#include "tgt/assert.h"
#include "tgt/texture.h"
#include "tgt/tgt_math.h"

#include <algorithm>

namespace {
    tgt::col4 toCol(const tgt::vec4& c) {
        return tgt::col4(
            static_cast<uint8_t>(255 * c.r), 
            static_cast<uint8_t>(255 * c.g), 
            static_cast<uint8_t>(255 * c.b), 
            static_cast<uint8_t>(255 * c.a));
    }

    tgt::vec4 toVec(const tgt::col4& c) {
        return tgt::vec4(
            static_cast<float>(c.r) / 255.f, 
            static_cast<float>(c.g) / 255.f, 
            static_cast<float>(c.b) / 255.f, 
            static_cast<float>(c.a) / 255.f);
    }
}

namespace campvis {
    
    TFGeometry2D::TFGeometry2D(const std::vector<KeyPoint>& keyPoints)
        : _keyPoints(keyPoints)
        , _center(tgt::vec2::zero, tgt::col4(255))
    {
        computeCenterAndSortKeyPoints();
    }

    TFGeometry2D::~TFGeometry2D() {

    }

    TFGeometry2D* TFGeometry2D::clone() const {
        return new TFGeometry2D(_keyPoints);
    }

    std::vector<TFGeometry2D::KeyPoint>& TFGeometry2D::getKeyPoints() {
        return _keyPoints;
    }

    void TFGeometry2D::render() const {
        // TODO: get rid of intermediade mode?
        if (_keyPoints.size() < 2)
            return;

        glBegin(GL_TRIANGLE_FAN);
        glColor4ubv(_center._color.elem);
        glVertex2fv(_center._position.elem);
        for (std::vector<KeyPoint>::const_iterator it = _keyPoints.begin(); it != _keyPoints.end(); ++it) {
            glColor4ubv(it->_color.elem);
            glVertex2fv(it->_position.elem);
        }
        glEnd();
    }

    TFGeometry2D* TFGeometry2D::createQuad(const tgt::vec2& ll, const tgt::vec2& ur, const tgt::col4& color) {
        tgtAssert(tgt::hand(tgt::greaterThanEqual(ll, tgt::vec2::zero)) && tgt::hand(tgt::lessThanEqual(ur, tgt::vec2(1.f))), "Interval out of bounds");
        tgtAssert(tgt::hand(tgt::lessThan(ll, ur)), "Lower left corner coordinates must be smaller than the upper right ones!");

        std::vector<KeyPoint> keyPoints;
        keyPoints.push_back(KeyPoint(ll, color));
        keyPoints.push_back(KeyPoint(tgt::vec2(ur.x, ll.y), color));
        keyPoints.push_back(KeyPoint(ur, color));
        keyPoints.push_back(KeyPoint(tgt::vec2(ll.x, ur.y), color));
        return new TFGeometry2D(keyPoints);
    }

    namespace {
        struct KeyPointSorter {
            KeyPointSorter(const tgt::vec2& center)
                : _center(center)
            {};

            bool operator() (const TFGeometry2D::KeyPoint& left, const TFGeometry2D::KeyPoint& right) {
                float angleLeft = atan2(left._position.y - _center.y, left._position.x - _center.x);
                float angleRight = atan2(right._position.y - _center.y, right._position.x - _center.x);
                return angleLeft < angleRight;
            };

            tgt::vec2 _center;
        };
    }

    void TFGeometry2D::computeCenterAndSortKeyPoints() {
        if (_keyPoints.empty())
            return;

        tgt::vec2 cPos(0.f);
        tgt::vec4 cCol(0.f);
        for (std::vector<KeyPoint>::const_iterator it = _keyPoints.begin(); it != _keyPoints.end(); ++it) {
            cPos += it->_position;
            cCol += toVec(it->_color);
        }
        cPos /= static_cast<float>(_keyPoints.size());
        cCol /= static_cast<float>(_keyPoints.size());
        _center = KeyPoint(cPos, toCol(cCol));

        std::sort(_keyPoints.begin(), _keyPoints.end(), KeyPointSorter(_center._position));
    }



}