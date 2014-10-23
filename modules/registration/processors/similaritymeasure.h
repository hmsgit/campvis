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

#ifndef SIMILARITYMEASURE_H__
#define SIMILARITYMEASURE_H__

#include <string>

#include "cgt/buffer.h"
#include "cgt/matrix.h"
#include "cgt/vertexarrayobject.h"

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/buttonproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/optionproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/transferfunctionproperty.h"

namespace tgt {
    class Shader;
}

namespace campvis {
    class FaceGeometry;
    class ImageData;
    class ImageRepresentationGL;
    class GlReduction;

    /**
     * Computes a Similarity Measure using OpenGL
     */
    class SimilarityMeasure : public VisualizationProcessor {
    public:
        enum AdditionalInvalidationLevels {
            PERFORM_OPTIMIZATION = FIRST_FREE_TO_USE_INVALIDATION_LEVEL << 0,
            COMPUTE_DIFFERENCE_IMAGE = FIRST_FREE_TO_USE_INVALIDATION_LEVEL << 1
        };

        /**
         * Constructs a new SimilarityMeasure Processor
         **/
        SimilarityMeasure();

        /**
         * Destructor
         **/
        virtual ~SimilarityMeasure();

        /// \see AbstractProcessor::init
        virtual void init();
        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "SimilarityMeasure"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Computes a Similarity Measure using OpenGL."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };
        
        /**
         * Computes the similarity between \a movingImage and \a referenceImage using the given 
         * translation and rotation and currently selected similarity metric.
         * \param   referenceImage  Reference Image
         * \param   movingImage     Moving Image
         * \param   translation     Translation to apply to \a movingImage
         * \param   rotation        Rotation to apply to \a movingImage
         * \return  The similarity
         */
        float computeSimilarity(const ImageRepresentationGL* referenceImage, const ImageRepresentationGL* movingImage, const tgt::vec3& translation, const tgt::vec3& rotation);

        /**
         * Computes the difference image between \a movingImage and \a referenceImage using the given 
         * translation and rotation.
         * \param   dc              DataContainer to store the difference image in
         * \param   referenceImage  Reference Image
         * \param   movingImage     Moving Image
         * \param   translation     Translation to apply to \a movingImage
         * \param   rotation        Rotation to apply to \a movingImage
         */
        void generateDifferenceImage(DataContainer* dc, const ImageRepresentationGL* referenceImage, const ImageRepresentationGL* movingImage, const tgt::vec3& translation, const tgt::vec3& rotation);

        DataNameProperty p_referenceId;                 ///< image ID for reference image
        DataNameProperty p_movingId;                    ///< image ID for moving image

        IVec2Property p_clipX;                          ///< clip coordinates for x axis
        IVec2Property p_clipY;                          ///< clip coordinates for y axis
        IVec2Property p_clipZ;                          ///< clip coordinates for z axis

        BoolProperty p_applyMask;                       ///< Flag whether use reference image as mask
        Vec3Property p_translation;                     ///< Moving image translation
        Vec3Property p_rotation;                        ///< Moving image rotation

        GenericOptionProperty<std::string> p_metric;    ///< Similarity metric
        ButtonProperty p_computeSimilarity;             ///< Compute similarity

        DataNameProperty p_differenceImageId;           ///< Image ID for difference Image
        ButtonProperty p_computeDifferenceImage;        ///< Generate Difference Image

    private:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dc);

        /**
         * Transforms euler angles to a 4x4 rotation matrix.
         * \param   eulerAngles     A vec3 with euler angles
         * \return  The corresponding 4x4 rotation matrix.
         */
        static tgt::mat4 euleranglesToMat4(const tgt::vec3& eulerAngles);

        /**
         * Computes the registration matrix to align \a movingImage to \a referenceImage with the 
         * provided translation and rotation. The resulting registration matrix is from reference
         * image's texture coordinates to moving image's texture coordinates. Rotation is performed
         * around the center of \a movingImage.
         * \param   referenceImage  Reference Image
         * \param   movingImage     Moving Image
         * \param   translation     Translation to apply to \a movingImage
         * \param   rotation        Rotation to apply to \a movingImage
         * \return  The registration matrix to align \a movingImage to \a referenceImage in texture coordinates.
         */
        static tgt::mat4 computeRegistrationMatrix(const ImageRepresentationGL* referenceImage, const ImageRepresentationGL* movingImage, const tgt::vec3& translation, const tgt::vec3& rotation);

        IVec2Property p_viewportSize;

        tgt::Shader* _sadssdCostFunctionShader;         ///< Shader for computing SAD/SSD
        tgt::Shader* _nccsnrCostFunctionShader;         ///< Shader for computing NCC/SNR
        tgt::Shader* _differenceShader;                 ///< Shader for computing the difference image
        GlReduction* _glr;                              ///< Pointer to GlReduction object

        static const std::string loggerCat_;
    };

}

#endif // SIMILARITYMEASURE_H__
