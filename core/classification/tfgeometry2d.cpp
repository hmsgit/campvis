// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#include "tfgeometry2d.h"

#include "cgt/assert.h"
#include "cgt/texture.h"
#include "cgt/cgt_math.h"

#include <algorithm>

namespace {
    cgt::col4 toCol(const cgt::vec4& c) {
        return cgt::col4(
            static_cast<uint8_t>(255 * c.r), 
            static_cast<uint8_t>(255 * c.g), 
            static_cast<uint8_t>(255 * c.b), 
            static_cast<uint8_t>(255 * c.a));
    }

    cgt::vec4 toVec(const cgt::col4& c) {
        return cgt::vec4(
            static_cast<float>(c.r) / 255.f, 
            static_cast<float>(c.g) / 255.f, 
            static_cast<float>(c.b) / 255.f, 
            static_cast<float>(c.a) / 255.f);
    }
}

namespace campvis {
    
    TFGeometry2D::TFGeometry2D(const std::vector<KeyPoint>& keyPoints)
        : _keyPoints(keyPoints)
        , _center(cgt::vec2::zero, cgt::col4(255))
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

    TFGeometry2D* TFGeometry2D::createQuad(const cgt::vec2& ll, const cgt::vec2& ur, const cgt::col4& color) {
        cgtAssert(cgt::hand(cgt::greaterThanEqual(ll, cgt::vec2::zero)) && cgt::hand(cgt::lessThanEqual(ur, cgt::vec2(1.f))), "Interval out of bounds");
        cgtAssert(cgt::hand(cgt::lessThan(ll, ur)), "Lower left corner coordinates must be smaller than the upper right ones!");

        std::vector<KeyPoint> keyPoints;
        keyPoints.push_back(KeyPoint(ll, color));
        keyPoints.push_back(KeyPoint(cgt::vec2(ur.x, ll.y), color));
        keyPoints.push_back(KeyPoint(ur, color));
        keyPoints.push_back(KeyPoint(cgt::vec2(ll.x, ur.y), color));
        return new TFGeometry2D(keyPoints);
    }

    namespace {
        struct KeyPointSorter {
            explicit KeyPointSorter(const cgt::vec2& center)
                : _center(center)
            {};

            bool operator() (const TFGeometry2D::KeyPoint& left, const TFGeometry2D::KeyPoint& right) {
                float angleLeft = atan2(left._position.y - _center.y, left._position.x - _center.x);
                float angleRight = atan2(right._position.y - _center.y, right._position.x - _center.x);
                return angleLeft < angleRight;
            };

            cgt::vec2 _center;
        };
    }

    void TFGeometry2D::computeCenterAndSortKeyPoints() {
        if (_keyPoints.empty())
            return;

        cgt::vec2 cPos(0.f);
        cgt::vec4 cCol(0.f);
        for (std::vector<KeyPoint>::const_iterator it = _keyPoints.begin(); it != _keyPoints.end(); ++it) {
            cPos += it->_position;
            cCol += toVec(it->_color);
        }
        cPos /= static_cast<float>(_keyPoints.size());
        cCol /= static_cast<float>(_keyPoints.size());
        _center = KeyPoint(cPos, toCol(cCol));

        std::sort(_keyPoints.begin(), _keyPoints.end(), KeyPointSorter(_center._position));
    }

    cgt::vec2 TFGeometry2D::getIntensityDomain() const {
        return cgt::vec2(_keyPoints.front()._position.x, _keyPoints.back()._position.x);
    }



}