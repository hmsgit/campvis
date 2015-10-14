%module cgt
%include "stdint.i"
%{
#include "ext/cgt/camera.h"
#include "ext/cgt/navigation/navigation.h"
#include "ext/cgt/vector.h"
#include "ext/cgt/event/eventlistener.h"
%}


namespace cgt {

    /* Vector2 */

    template<class T>
    struct Vector2 {
        Vector2(T t);
        Vector2(T t1, T t2);

        static Vector2<T> zero;

        T x;
        T y;
    };

    %template(ivec2) Vector2<int>;
    typedef Vector2<int> ivec2;
    %template(svec2) Vector2<size_t>;
    typedef Vector2<size_t> svec2;
    %template(vec2) Vector2<float>;
    typedef Vector2<float> vec2;

    /* Vector3 */

    template<class T>
    struct Vector3 {
        Vector3();
        explicit Vector3(T v);
        Vector3(T t1, T t2, T t3);
        Vector3(const Vector2<T>& vec, T z_);
        Vector3(T _x, const Vector2<T>& v);

        static Vector3<T> zero;
        
        T x;
        T y;
        T z;
    };

    %template(ivec3) Vector3<int>;
    typedef Vector3<int> ivec3;
    %template(svec3) Vector3<size_t>;
    typedef Vector3<size_t> svec3;
    %template(vec3) Vector3<float>;
    typedef Vector3<float> vec3;

    /* Vector4 */

    template<class T>
    struct Vector4 {
        Vector4() {}
        explicit Vector4(T init);
        Vector4(T t1, T t2, T t3, T t4);
        Vector4(const Vector2<T>& v1, const Vector2<T>& v2);
        Vector4(const Vector2<T>& vec, T z_, T w_);
        Vector4(T _x, T _y, const Vector2<T>& v);

        T x;
        T y;
        T z;
        T w;
    };

    %template(ivec4) Vector4<int>;
    typedef Vector4<int> ivec4;
    %template(svec4) Vector4<size_t>;
    typedef Vector4<size_t> svec4;
    %template(vec4) Vector4<float>;
    typedef Vector4<float> vec4;

    %template(col4) Vector4<uint8_t>;
    typedef Vector4<uint8_t> col4;



    /* Matrix */

    template<class T>
    struct Matrix4 {
        T t00, t01, t02, t03;
        T t10, t11, t12, t13;
        T t20, t21, t22, t23;
        T t30, t31, t32, t33;

        Matrix4();
        explicit Matrix4(T t);
        template<class U> Matrix4(const Matrix4<U>& m);
        explicit Matrix4(const T* t);
        Matrix4(T _t00, T _t01, T _t02, T _t03,
                T _t10, T _t11, T _t12, T _t13,
                T _t20, T _t21, T _t22, T _t23,
                T _t30, T _t31, T _t32, T _t33);
        Matrix4(const Vector4<T>& v1, const Vector4<T>& v2,
                const Vector4<T>& v3, const Vector4<T>& v4);



        static const Matrix4<T> zero;
        static const Matrix4<T> identity;

        static Matrix4<T> createTranslation(const Vector3<T>& v);
        static Matrix4<T> createScale(const Vector3<T>& v);

        static Matrix4<T> createRotationX(T angle);
        static Matrix4<T> createRotationY(T angle);
        static Matrix4<T> createRotationZ(T angle);
        static Matrix4<T> createRotation(T angle, Vector3<T> axis);

        static Matrix4<T> createLookAt(const Vector3<T>& eye, const Vector3<T>& focus, const Vector3<T>& up);
        static Matrix4<T> createFrustum(T left, T right, T top, T bottom, T pnear, T pfar);
        static Matrix4<T> createPerspective(T fov, T aspect, T pnear, T pfar);
        static Matrix4<T> createOrtho(T left, T right, T top, T bottom, T pnear, T pfar);

        Vector4<T> operator [] (size_t i) const;
        Vector4<T> operator [] (size_t i);

        bool invert(Matrix4<T>& result) const;
    };



    /* EventListener */

    class EventListener {
    public:
        EventListener();
        virtual ~EventListener();
    };

    /* Camera */

    class Camera {
    public:
        enum ProjectionMode {
            ORTHOGRAPHIC,
            PERSPECTIVE,
            FRUSTUM
        };

        Camera(const vec3& position =  vec3(0.f, 0.f,  0.f),
               const vec3& focus    =  vec3(0.f, 0.f, -1.f),
               const vec3& up       =  vec3(0.f, 1.f,  0.f),
               float fovy           =  45.f,
               float ratio          =  static_cast<float>(GLCanvas::DEFAULT_WINDOW_WIDTH) /
                                       GLCanvas::DEFAULT_WINDOW_HEIGHT,
               float distn          =  0.1f,
               float distf          =  50.f,
               ProjectionMode pm    =  PERSPECTIVE);

        virtual ~Camera();
    };

    class AbstractCameraProxy {
    public:
        virtual ~AbstractCameraProxy();

    virtual Camera* getCamera() = 0;
    virtual void update() = 0;
    };
}
