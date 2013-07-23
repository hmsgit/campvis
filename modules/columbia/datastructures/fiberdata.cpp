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

#include "fiberdata.h"

namespace campvis {

    FiberData::FiberData() 
        : AbstractData()
    {
    }

    FiberData::~FiberData() {

    }

    void FiberData::addFiber(const std::deque<tgt::vec3>& vertices) {
        _vertices.insert(_vertices.end(), vertices.begin(), vertices.end());
        _fibers.push_back(Fiber(_vertices.size() - vertices.size(), _vertices.size()));
    }

    void FiberData::addFiber(const std::vector<tgt::vec3>& vertices) {
        _vertices.insert(_vertices.end(), vertices.begin(), vertices.end());
        _fibers.push_back(Fiber(_vertices.size() - vertices.size(), _vertices.size()));
    }

    void FiberData::clear() {
        _fibers.clear();
        _vertices.clear();
    }

    void FiberData::updateLengths() const {
        for (size_t i = 0; i < _fibers.size(); ++i) {
            _fibers[i]._length = 0.0f;

            for (size_t j = _fibers[i]._startIndex + 1; j < _fibers[i]._endIndex; ++j)
                _fibers[i]._length += distance(_vertices[j-1], _vertices[j]);
        }
    }

    size_t FiberData::numFibers() const {
        return _fibers.size();
    }

    size_t FiberData::numSegments() const {
        size_t sum = 0;
        for (std::vector<Fiber>::const_iterator it = _fibers.begin(); it != _fibers.end(); ++it) 
            sum += (it->_endIndex - it->_startIndex);
        return sum;
    }


    bool FiberData::empty() const {
        return _fibers.empty();
    }

    FiberData* FiberData::clone() const {
        FiberData* toReturn = new FiberData(*this);
        return toReturn;
    }

    size_t FiberData::getLocalMemoryFootprint() const {
        size_t sum = _vertices.size() * sizeof(tgt::vec3);
        sum += _fibers.size() * sizeof(Fiber);
        sum += sizeof(*this);
        return sum;
    }

    size_t FiberData::getVideoMemoryFootprint() const {
        return 0;
    }

}