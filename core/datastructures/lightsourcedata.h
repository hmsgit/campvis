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

#ifndef LIGHTSOURCEDATA_H__
#define LIGHTSOURCEDATA_H__

#include "tgt/vector.h"

#include "core/datastructures/abstractdata.h"

namespace campvis {

    /**
     * Abstract base class for data handled by a DataHandle and stored in a DataContainer.
     * 
     * \todo 
     */
    class CAMPVIS_CORE_API LightSourceData : public AbstractData {
    public:
        /**
         * Constructor, Creates a new light source.
         * \param   lightPosition   The light position
         * \param   ambientColor    The ambient color
         * \param   diffuseColor    The diffuse color
         * \param   specularColor   The specular color
         * \param   shininess       The shininess
         * \param   attenuation     The attenuation factors
         */
        explicit LightSourceData(
            const tgt::vec3& lightPosition,
            const tgt::vec3& ambientColor = tgt::vec3(.4f),
            const tgt::vec3& diffuseColor = tgt::vec3(.75f),
            const tgt::vec3& specularColor = tgt::vec3(.6f),
            float shininess = 24.f,
            const tgt::vec3& attenuation = tgt::vec3(0.f));

        /**
         * Virtual destructor
         */
        virtual ~LightSourceData();

        /// \see AbstractData::clone()
        virtual LightSourceData* clone() const;

        /// \see AbstractData::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;
        /// \see AbstractData::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;

        /**
        * Gets the light positoin.
        * \return _lightPosition
        **/
        tgt::vec3 getLightPosition() const { return _lightPosition; }
        /**
        * Sets the light positoin.
        * \param _lightPosition New value for the light positoin.
        **/
        void setLightPosition(tgt::vec3 val) { _lightPosition = val; }

        /**
        * Gets the ambient color.
        * \return _ambientColor
        **/
        tgt::vec3 getAmbientColor() const { return _ambientColor; }
        /**
        * Sets the ambient color.
        * \param _ambientColor New value for the ambient color.
        **/
        void setAmbientColor(tgt::vec3 val) { _ambientColor = val; }

        /**
        * Gets the diffuse light color.
        * \return _diffuseColor
        **/
        tgt::vec3 getDiffuseColor() const { return _diffuseColor; }
        /**
        * Sets the diffuse light color.
        * \param _diffuseColor New value for the diffuse light color.
        **/
        void setDiffuseColor(tgt::vec3 val) { _diffuseColor = val; }

        /**
        * Gets the specular color.
        * \return _specularColor
        **/
        tgt::vec3 getSpecularColor() const { return _specularColor; }
        /**
        * Sets the specular color.
        * \param _specularColor New value for the specular color.
        **/
        void setSpecularColor(tgt::vec3 val) { _specularColor = val; }

        /**
        * Gets the specular shininess.
        * \return _shininess
        **/
        float getShininess() const { return _shininess; }
        /**
        * Sets the specular shininess.
        * \param _shininess New value for the specular shininess.
        **/
        void setShininess(float val) { _shininess = val; }

        /**
        * Gets the attenuation factors.
        * \return _attenuation
        **/
        tgt::vec3 getAttenuation() const { return _attenuation; }
        /**
        * Sets the attenuation factors.
        * \param _attenuation New value for the attenuation factors.
        **/
        void setAttenuation(tgt::vec3 val) { _attenuation = val; }
                

    protected:
        tgt::vec3 _lightPosition;           ///< Light position
        tgt::vec3 _ambientColor;            ///< Ambient light color
        tgt::vec3 _diffuseColor;            ///< Diffuse light color
        tgt::vec3 _specularColor;           ///< Specular light color
        float _shininess;                   ///< Specular shininess
        tgt::vec3 _attenuation;             ///< Attenuation factors
        
    };

}

#endif // LIGHTSOURCEDATA_H__