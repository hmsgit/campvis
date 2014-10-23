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

#ifndef LIGHTSOURCEDATA_H__
#define LIGHTSOURCEDATA_H__

#include "cgt/vector.h"

#include "core/datastructures/abstractdata.h"

namespace cgt {
    class Shader;
}

namespace campvis {

    /**
     * Abstract base class for data handled by a DataHandle and stored in a DataContainer.
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
            const cgt::vec3& lightPosition,
            const cgt::vec3& ambientColor = cgt::vec3(.4f),
            const cgt::vec3& diffuseColor = cgt::vec3(.75f),
            const cgt::vec3& specularColor = cgt::vec3(.6f),
            float shininess = 24.f,
            const cgt::vec3& attenuation = cgt::vec3(0.f));

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
         * Binds this light source to the given shader by setting the corresponding uniforms.
         * \param   shader      Shader to bind to.
         * \param   uniformName Uniform name to use.
         */
        void bind(cgt::Shader* shader, const std::string& uniformName) const;

        /**
        * Gets the light positoin.
        * \return _lightPosition
        **/
        cgt::vec3 getLightPosition() const { return _lightPosition; }
        /**
        * Sets the light positoin.
        * \param _lightPosition New value for the light positoin.
        **/
        void setLightPosition(cgt::vec3 val) { _lightPosition = val; }

        /**
        * Gets the ambient color.
        * \return _ambientColor
        **/
        cgt::vec3 getAmbientColor() const { return _ambientColor; }
        /**
        * Sets the ambient color.
        * \param _ambientColor New value for the ambient color.
        **/
        void setAmbientColor(cgt::vec3 val) { _ambientColor = val; }

        /**
        * Gets the diffuse light color.
        * \return _diffuseColor
        **/
        cgt::vec3 getDiffuseColor() const { return _diffuseColor; }
        /**
        * Sets the diffuse light color.
        * \param _diffuseColor New value for the diffuse light color.
        **/
        void setDiffuseColor(cgt::vec3 val) { _diffuseColor = val; }

        /**
        * Gets the specular color.
        * \return _specularColor
        **/
        cgt::vec3 getSpecularColor() const { return _specularColor; }
        /**
        * Sets the specular color.
        * \param _specularColor New value for the specular color.
        **/
        void setSpecularColor(cgt::vec3 val) { _specularColor = val; }

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
        cgt::vec3 getAttenuation() const { return _attenuation; }
        /**
        * Sets the attenuation factors.
        * \param _attenuation New value for the attenuation factors.
        **/
        void setAttenuation(cgt::vec3 val) { _attenuation = val; }
                

    protected:
        cgt::vec3 _lightPosition;           ///< Light position
        cgt::vec3 _ambientColor;            ///< Ambient light color
        cgt::vec3 _diffuseColor;            ///< Diffuse light color
        cgt::vec3 _specularColor;           ///< Specular light color
        float _shininess;                   ///< Specular shininess
        cgt::vec3 _attenuation;             ///< Attenuation factors
        
    };

}

#endif // LIGHTSOURCEDATA_H__