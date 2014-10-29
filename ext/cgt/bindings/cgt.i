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
    };

    %template(ivec2) Vector2<int>;
    typedef Vector2<int> ivec2;

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
    };

    %template(vec3) Vector3<float>;
    typedef Vector3<float> vec3;

    %template(svec3) Vector3<size_t>;
    typedef Vector3<size_t> svec3;
	
    %template(ivec3) Vector3<int>;
    typedef Vector3<int> ivec3;

    /* Vector4 */

    template<class T>
    struct Vector4 {
        Vector4() {}
        explicit Vector4(T init);
        Vector4(T t1, T t2, T t3, T t4);
        Vector4(const Vector2<T>& v1, const Vector2<T>& v2);
        Vector4(const Vector2<T>& vec, T z_, T w_);
        Vector4(T _x, T _y, const Vector2<T>& v);
    };

    %template(vec4) Vector4<float>;
    typedef Vector4<float> vec4;

    %template(col4) Vector4<uint8_t>;
    typedef Vector4<uint8_t> col4;

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

    class IHasCamera {
    public:
        virtual ~IHasCamera();

    virtual Camera* getCamera() = 0;
    virtual void update() = 0;
    };
}
