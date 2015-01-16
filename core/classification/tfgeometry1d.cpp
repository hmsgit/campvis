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

#include "tfgeometry1d.h"

#include "cgt/assert.h"
#include "cgt/texture.h"
#include "cgt/cgt_math.h"

#include "core/datastructures/facegeometry.h"

#include <algorithm>

namespace campvis {

    bool operator< (const TFGeometry1D::KeyPoint& left, const TFGeometry1D::KeyPoint& right) {
        return left._position < right._position;
    }
    
    TFGeometry1D::TFGeometry1D(const std::vector<KeyPoint>& keyPoints)
        : _keyPoints(keyPoints)
    {
        std::sort(_keyPoints.begin(), _keyPoints.end());
    }

    TFGeometry1D::~TFGeometry1D() {

    }

    TFGeometry1D* TFGeometry1D::clone() const {
        return new TFGeometry1D(_keyPoints);
    }

    std::vector<TFGeometry1D::KeyPoint>& TFGeometry1D::getKeyPoints() {
        return _keyPoints;
    }

    void TFGeometry1D::renderIntoEditor() const {
        // TODO: get rid of intermediade mode?
        if (_keyPoints.size() < 2)
            return;

        glBegin(GL_QUADS);
        std::vector<KeyPoint>::const_iterator a = _keyPoints.begin();
        std::vector<KeyPoint>::const_iterator b = _keyPoints.begin()+1;
        for (/* already inited */; b != _keyPoints.end(); ++a, ++b) {
            glColor4ub(a->_color.r, a->_color.g, a->_color.b, 144);
            float y = static_cast<float>(a->_color.a) / 255.f;
            glVertex2f(a->_position, 0.f);
            glVertex2f(a->_position, y);

            glColor4ub(b->_color.r, b->_color.g, b->_color.b, 144);
            y = static_cast<float>(b->_color.a) / 255.f;
            glVertex2f(b->_position, y);
            glVertex2f(b->_position, 0.f);
        }
        glEnd();
    }

    void TFGeometry1D::render() const {
        if (_keyPoints.size() < 2)
            return;

        // TODO: regenerating these buffers each time is slow as hell
        std::vector<cgt::vec3> vertices;
        std::vector<cgt::vec4> colors;

        for (std::vector<KeyPoint>::const_iterator a = _keyPoints.begin(); a != _keyPoints.end(); ++a) {
            vertices.push_back(cgt::vec3(a->_position, 0.f, 0.f));
            vertices.push_back(cgt::vec3(a->_position, 1.f, 0.f));

            colors.push_back(cgt::vec4(a->_color) / 255.f);
            colors.push_back(cgt::vec4(a->_color) / 255.f);
        }

        FaceGeometry fg(vertices, std::vector<cgt::vec3>(), colors);
        fg.render(GL_TRIANGLE_STRIP);
    }

    TFGeometry1D* TFGeometry1D::createQuad(const cgt::vec2& interval, const cgt::col4& leftColor, const cgt::col4& rightColor) {
        cgtAssert(interval.x >= 0.f && interval.y <= 1.f, "Interval out of bounds");

        std::vector<KeyPoint> keyPoints;
        keyPoints.push_back(KeyPoint(interval.x, leftColor));
        keyPoints.push_back(KeyPoint(interval.y, rightColor));
        return new TFGeometry1D(keyPoints);
    }

    TFGeometry1D* TFGeometry1D::crateRamp(const cgt::vec2& interval, const cgt::col4& color) {
        return createQuad(interval, cgt::col4(color.xyz(), 0), cgt::col4(color.xyz(), 255));
    }

    TFGeometry1D* TFGeometry1D::createDivergingColorMap(const cgt::vec2& interval, const cgt::col4& leftColor, const cgt::col4& rightColor, float bias /*= 0.5f*/) {
        cgtAssert(interval.x >= 0.f && interval.y <= 1.f, "Interval out of bounds, must be in [0, 1].");
        cgtAssert(bias > 0.f && bias < 1.f, "Bias out of bounds, must be in (0, 1).");

        std::vector<KeyPoint> keyPoints;
        keyPoints.push_back(KeyPoint(interval.x, leftColor));
        keyPoints.push_back(KeyPoint(interval.x + (interval.y - interval.x) * bias, cgt::col4(255, 255, 255, 255)));
        keyPoints.push_back(KeyPoint(interval.y, rightColor));
        return new TFGeometry1D(keyPoints);
    }

    TFGeometry1D* TFGeometry1D::createColdHotColorMap(const cgt::vec2& interval /*= cgt::vec2(0.f, 1.f)*/) {
        return createDivergingColorMap(interval, cgt::col4(0, 0, 255, 255), cgt::col4(255, 0, 0, 255), 0.5f);
    }

    TFGeometry1D* TFGeometry1D::createHeatedBodyColorMap(const cgt::vec2& interval /*= cgt::vec2(0.f, 1.f)*/) {
        cgtAssert(interval.x >= 0.f && interval.y <= 1.f, "Interval out of bounds, must be in [0, 1].");

        std::vector<KeyPoint> keyPoints;
        keyPoints.push_back(KeyPoint(interval.x, cgt::col4(0, 0, 0, 255)));
        keyPoints.push_back(KeyPoint(interval.x + (interval.y - interval.x) * 0.35f, cgt::col4(224, 0, 0, 255)));
        keyPoints.push_back(KeyPoint(interval.x + (interval.y - interval.x) * 0.85f, cgt::col4(255, 255, 0, 255)));
        keyPoints.push_back(KeyPoint(interval.y, cgt::col4(255, 255, 255, 255)));
        return new TFGeometry1D(keyPoints);
    }

    void TFGeometry1D::addKeyPoint( float position, float alpha) {
        TFGeometry1D::KeyPoint kp(position, cgt::col4(255));
        cgt::col4 color(255);
        std::vector<TFGeometry1D::KeyPoint>::iterator lb = std::upper_bound(_keyPoints.begin(), _keyPoints.end(), kp);
        if (lb != _keyPoints.end()) {
            color = lb->_color;
        }
        else {
            color = _keyPoints.back()._color;
        }
        color.a = static_cast<uint8_t>(alpha * 255.f);
        addKeyPoint(position, color);
    }

    void TFGeometry1D::addKeyPoint( float position, const cgt::col4& color ) {
        TFGeometry1D::KeyPoint kp(position, color);
        std::vector<TFGeometry1D::KeyPoint>::iterator lb = std::upper_bound(_keyPoints.begin(), _keyPoints.end(), kp);
        _keyPoints.insert(lb, kp);
    }

    cgt::vec2 TFGeometry1D::getIntensityDomain() const {
        return cgt::vec2(_keyPoints.front()._position, _keyPoints.back()._position);
    }

}