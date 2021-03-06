/**********************************************************************
 *                                                                    *
 * cgt - CAMP Graphics Toolbox, Copyright (C) 2012-2015               *
 *     Chair for Computer Aided Medical Procedures                    *
 *     Technische Universitaet Muenchen, Germany.                     *
 *     <http://campar.in.tum.de/>                                     *
 *                                                                    *
 * forked from tgt - Tiny Graphics Toolbox, Copyright (C) 2006-2011   *
 *     Visualization and Computer Graphics Group, Department of       *
 *     Computer Science, University of Muenster, Germany.             *
 *     <http://viscg.uni-muenster.de>                                 *
 *                                                                    *
 * This file is part of the cgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#include "cgt/textureunit.h"
#include "cgt/gpucapabilities.h"
#include "cgt/assert.h"

namespace cgt {

bool TextureUnit::initialized_ = false;
unsigned short TextureUnit::totalActive_ = 0;
unsigned short TextureUnit::maxTexUnits_ = 0;
unsigned short TextureUnit::numKeptUnits_ = 0;
std::vector<bool> TextureUnit::busyUnits_ = std::vector<bool>();
tbb::spin_mutex TextureUnit::mutex_ = tbb::spin_mutex();

TextureUnit::TextureUnit(bool keep)
    : number_(0)
    , glEnum_(0)
    , assigned_(false)
    , keep_(keep)
{
    if (!initialized_)
        init();
}

TextureUnit::~TextureUnit() {
    if (assigned_ && !keep_) {
        tbb::spin_mutex::scoped_lock lock(mutex_);

        busyUnits_.at(number_) = false;
        totalActive_--;
    }
}

void TextureUnit::activate() const {
    if (!assigned_)
        assignUnit();
    glActiveTexture(getEnum());
}

GLint TextureUnit::getEnum() const {
    if (!assigned_)
        assignUnit();
    return glEnum_;
}

GLint TextureUnit::getUnitNumber() const {
    if (!assigned_)
        assignUnit();
    return number_;
}

void TextureUnit::setZeroUnit() {
    glActiveTexture(GL_TEXTURE0);
}

void TextureUnit::cleanup() {
    tbb::spin_mutex::scoped_lock lock(mutex_);

    for (size_t i = 0; i < maxTexUnits_; i++) {
        if (busyUnits_.at(i))
            busyUnits_.at(i) = false;
    }
    totalActive_ = 0;
    numKeptUnits_ = 0;
    setZeroUnit();
}

bool TextureUnit::unused() {
    return (totalActive_ == 0);
}

unsigned short TextureUnit::numLocalActive() {
    return (totalActive_ - numKeptUnits_);
}

void TextureUnit::assignUnit() const {
    tbb::spin_mutex::scoped_lock lock(mutex_);
    cgtAssert(totalActive_ <= maxTexUnits_, "No more texture units available");

    assigned_ = true;

    for (size_t i = 0; i < maxTexUnits_; i++) {
        if (!busyUnits_.at(i)) {
            number_ = (GLint)i;
            busyUnits_.at(i) = true;
            totalActive_++;
            if(keep_)
                numKeptUnits_++;
            break;
        }
    }

    glEnum_ = GL_TEXTURE0 + number_;
}

void TextureUnit::init() {
    tbb::spin_mutex::scoped_lock lock(mutex_);
    if (initialized_)
        return;

    maxTexUnits_ = GpuCaps.getNumTextureUnits();
    busyUnits_ = std::vector<bool>(maxTexUnits_, false);
    numKeptUnits_ = 0;
    initialized_ = true;
}

} // namespace
