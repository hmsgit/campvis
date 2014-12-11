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

#ifndef CAMERADATA_H__
#define CAMERADATA_H__

#include "cgt/camera.h"
#include "core/datastructures/abstractdata.h"
#include <vector>

namespace campvis {
    /**
     * Data object storing camera data.
     */
    class CAMPVIS_CORE_API CameraData : public AbstractData {
    public:
        /**
         * Constructor, creating a new CameraData object initialized by \a camera.
         * \param   camera  Camera object used for initialization (optional)
         */
        explicit CameraData(const cgt::Camera& camera = cgt::Camera());

        /**
         * Destructor.
         */
        virtual ~CameraData();


        /// \see AbstractData::clone()
        virtual CameraData* clone() const;

        /// \see AbstractData::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;

        /// \see AbstractData::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;

        /// \see AbstractData::getTypeAsString()
        virtual std::string getTypeAsString() const;


        /**
         * Const getter for the camera settings.
         * \return  _camera
         */
        const cgt::Camera& getCamera() const;

        /**
         * Non-const getter for the camera settings.
         * \return  _camera
         */
        cgt::Camera& getCamera();

        /**
         * Sets the camera settings to \a camera.
         * \param   camera  New camera settings.
         */
        void setCamera(const cgt::Camera& camera);

    protected:
        cgt::Camera _camera;        ///< The cgt::Camera object storing the camera setup

    };

}

#endif // CAMERADATA_H__
