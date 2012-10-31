// ================================================================================================
// 
// This file is part of the CAMPVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef SIMPLETRANSFERFUNCTION_H__
#define SIMPLETRANSFERFUNCTION_H__

#include "core/classification/abstracttransferfunction.h"

namespace campvis {

    /**
     * A very simple ramp transfer function, just for testing purposes...
     */
    class SimpleTransferFunction : public AbstractTransferFunction {
    public:
        /**
         * Creates a new SimpleTransferFunction.
         * \param   size            Size of the transfer function texture
         * \param   intensityDomain Intensity Domain where the transfer function is mapped to during classification
         */
        SimpleTransferFunction(size_t size, const tgt::vec2& intensityDomain = tgt::vec2(0.f, 1.f));

        /**
         * Destructor, make sure to delete the OpenGL texture beforehand by calling deinit() with a valid OpenGL context!
         */
        virtual ~SimpleTransferFunction();

        /**
         * Returns the dimensionality of the transfer function.
         * \return  The dimensionality of the transfer function.
         */
        virtual size_t getDimensionality() const;

        const tgt::col4& getLeftColor() const;
        void setLeftColor(const tgt::col4& color);
        const tgt::col4& getRightColor() const;
        void setRightColor(const tgt::col4& color);

    protected:
        /**
         * Creates the texture and uploads it to OpenGL.
         * Gets called by bind() with the local mutex already acquired.
         */
        virtual void createTexture();

        tgt::col4 _leftColor;
        tgt::col4 _rightColor;

        static const std::string loggerCat_;

    };

}

#endif // SIMPLETRANSFERFUNCTION_H__
