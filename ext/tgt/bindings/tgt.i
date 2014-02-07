%module tgt
%include "stdint.i"
%{
#include "ext/tgt/vector.h"
%}


namespace tgt {

    /* Vector2 */

    template<class T>
    struct Vector2 {
        Vector2(T t);
        Vector2(T t1, T t2);

        static Vector2<T> zero;
    };

    %template(ivec2) Vector2<int>;
    typedef Vector2<int> ivec2;

    %template(vec2) Vector2<float>;
    typedef Vector2<float> vec2;

    /* Vector4 */

    template<class T>
    struct Vector4 {
        Vector4() {}
        explicit Vector4(T init);
        explicit Vector4(const T* init);
        Vector4(T t1, T t2, T t3, T t4);
        Vector4(const Vector2<T>& v1, const Vector2<T>& v2);
        Vector4(const Vector2<T>& vec, T z_, T w_);
        Vector4(T _x, T _y, const Vector2<T>& v);
    };

    %template(vec4) Vector4<float>;
    typedef Vector4<float> vec4;

    %template(col4) Vector4<uint8_t>;
    typedef Vector4<uint8_t> col4;
}
