#ifndef SIMPLETRANSFERFUNCTION_H__
#define SIMPLETRANSFERFUNCTION_H__

#include "core/classification/abstracttransferfunction.h"

namespace TUMVis {

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

        void setLeftColor(const tgt::col4& color);
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
