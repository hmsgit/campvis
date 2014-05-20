
#ifndef COLORPROPERTY_H__
#define COLORPROPERTY_H__


# include "core/properties/floatingpointproperty.h"

namespace campvis {
    template <typename T>
    class _ColorProperty : public FloatingPointProperty<T> {
    public:
                /**
             * Creates a new FloatingPointProperty.
             * \param name      Property name
             * \param title     Property title (e.g. used for GUI)
             * \param value     Initial value of the property
             * \param minValue  Minimum value for this property
             * \param maxValue  Maximum value for this property
             * \param stepValue Step value for this property
             * \param decimals  Number of significant decimal places
             */
            _ColorProperty(
                const std::string& name,
                const std::string& title,
                const T& value,
                const T& minValue,
                const T& maxValue,
                const T& stepValue = T(0.01f));

            static const std::string loggerCat_;
    };

    template<typename T>
    const std::string campvis::_ColorProperty<T>::loggerCat_ = "CAMPVis.core.properties.ColorProperty";

    typedef _ColorProperty<tgt::vec4> ColorProperty;



    template<typename T>
    _ColorProperty<T>::_ColorProperty(
        const std::string& name,
        const std::string& title,
        const T& value,
        const T& minValue,
        const T& maxValue,
        const T& stepValue = T(0.01f)) 
        : Vec4Property(name, title, value, minValue, maxValue, stepValue)
    {

    }
}


#endif // COLORPROPERTY_H__

