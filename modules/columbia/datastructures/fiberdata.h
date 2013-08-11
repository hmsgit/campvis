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

#ifndef FIBERDATA_H__
#define FIBERDATA_H__

#include "tgt/bounds.h"
#include "tgt/tgt_gl.h"
#include "tgt/vector.h"

#include "core/datastructures/abstractdata.h"

#include <deque>
#include <vector>

namespace tgt {
    class BufferObject;
}

namespace campvis {

    /**
     * Data object storing fiber data.
     */
    class FiberData : public AbstractData {
    public:
        /**
         * Struct storing meta information about a single fiber.
         */
        struct Fiber {
            size_t _startIndex;     ///< Start index of the fiber
            size_t _endIndex;       ///< End index of the fiber (as in STL iterators: points to the element _behind_ the last vertex)

            mutable float _length;  ///< Length of the fiber (cached)
            int _segmentId;         ///< Label of the fiber
            bool _visible;          ///< Visibility flag of the fiber
            bool _selected;         ///< Selected flag of the fiber

            Fiber(size_t startIndex, size_t endIndex) 
                : _startIndex(startIndex), _endIndex(endIndex), _length(0.f), _segmentId(0), _visible(true), _selected(false) 
            {};
        };


        /**
         * Constructor.
         */
        FiberData();

        /**
         * Copy Constructor.
         * \param   rhs source
         */
        FiberData(const FiberData& rhs);

        /**
         * Destructor.
         */
        virtual ~FiberData();


        /**
         * Assignment Operator
         * \param   rhs     Source
         * \return  *this
         */
        FiberData& operator=(const FiberData& rhs);

        /**
         * Generates a new fiber from the given vertices and adds it to this data structure.
         * \param   vertices    Coordinates of the fiber points.
         */
        void addFiber(const std::deque<tgt::vec3>& vertices);

        /**
         * Generates a new fiber from the given vertices and adds it to this data structure.
         * \param   vertices    Coordinates of the fiber points.
         */
        void addFiber(const std::vector<tgt::vec3>& vertices);

        /**
         * Clears this data structure.
         */
        void clear();

        /**
         * Computes the lengths of each fiber in this data structure and stores it in the 
         * corresponding field.
         * \note    Since there is currently not automatism to do this, you're responsible to do 
         *          this yourself when needed.
         */
        void updateLengths() const;

        /**
         * Returns the number of fibers in this data structure.
         * \return  _fibers.size()
         */
        size_t numFibers() const;

        /**
         * Returns the number of fiber segments in this data structure.
         * \return  (i.e. the number of vertices - number of fibers)
         */
        size_t numSegments() const;

        /**
         * Returns whether this data structure is empty (i.e. has no fibers).
         */
        bool empty() const;

        /**
         * Returns the fiber data extent in world coordinates.
         * \note    Caution: Calling this method is expensive as the bounds are computed each time.
         * \return  The fiber data extent in world coordinates.
         */
        tgt::Bounds getWorldBounds() const;

        /**
         * Renders the Fiber geometry of this data set in the current OpenGL context.
         * \note    Must be called from a valid openGL context!
         * \param   mode    OpenGL render mode (defaults to GL_LINE_STRIP).
         */
        void render(GLenum mode = GL_LINE_STRIP) const;

        
        /// \see AbstractData::clone()
        virtual FiberData* clone() const;

        /// \see AbstractData::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;

        /// \see AbstractData::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;

    protected:
        /**
         * Creates the OpenGL buffers with vertex and tangent data.
         */
        void createGlBuffers() const;

        std::vector<tgt::vec3> _vertices;   ///< The fiber vertex (coordinates) data
        std::vector<Fiber> _fibers;         ///< The fiber meta data

        mutable tgt::BufferObject* _vertexBuffer;   ///< Pointer to OpenGL buffer with vertex data (lazy-instantiated)
        mutable tgt::BufferObject* _tangentBuffer;  ///< Pointer to OpenGL buffer with tangent data (lazy-instantiated)
        mutable bool _buffersInitialized;           ///< flag whether all OpenGL buffers were successfully initialized

        mutable GLint* _vboFiberStartIndices;       ///< VBO start indices for each fiber
        mutable GLsizei* _vboFiberCounts;           ///< number of indices for each fiber
        mutable GLsizei _vboFiberArraySize;         ///< number of elements in the above two lists
    };

}

#endif // FIBERDATA_H__
