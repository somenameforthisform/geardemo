#pragma once
#include <cmath> 

class Vec3 {
    public: 
        float x, y, z;

    Vec3(): x(0.0f), y(0.0f), z(0.0f){};
    Vec3(float x): Vec3(x, 0.0f, 0.0f){};
    Vec3(float x, float y): Vec3(x, y, 0.0f){};
    Vec3(float x, float y, float z): x(x), y(y), z(z){};
    Vec3(const Vec3& v): x(v.x), y(v.y), z(v.z){};

    explicit operator bool() const { return true; }
    
    Vec3 operator+(const Vec3& v) const {
        return Vec3(x + v.x, y + v.y, z + v.z);
    }

    Vec3 operator-(const Vec3& v) const {
        return Vec3(x - v.x, y - v.y, z - v.z);
    }

    Vec3 operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    Vec3 operator/(float scalar) const {
        return Vec3(x / scalar, y / scalar, z / scalar);
    }

    float operator*(Vec3 v) const {
        return (x * v.x) + (y * v.y) + (z * v.z);
    }

    Vec3& operator+=(const Vec3& v) { 
        x += v.x; y += v.y; z += v.z; 
        return *this;
    }

    Vec3& operator+=(float scalar) { 
        x += scalar; y += scalar; z += scalar; 
        return *this;
    }

    Vec3& operator-=(const Vec3& v) { 
        x -= v.x; y -= v.y; z -= v.z; 
        return *this;
    }

    Vec3& operator-=(float scalar) { 
        x -= scalar; y -= scalar; z -= scalar; 
        return *this;
    }


    Vec3& operator*=(const Vec3& v) { 
        x *= v.x; y *= v.y; z *= v.z; 
        return *this;
    }

    Vec3& operator*=(float scalar) { 
        x *= scalar; y *= scalar; z *= scalar; 
        return *this;
    }

    Vec3& operator/=(const Vec3& v) { 
        x /= v.x; y /= v.y; z /= v.z; 

        return *this;
    }

    Vec3& operator/=(float scalar) { 
        x /= scalar; y /= scalar; z /= scalar; 

        return *this;
    }

    float length(){
        return std::sqrt(x * x + y * y + z * z);
    }

    Vec3& normalize(){ 
        float length = this->length();
        x /= length; y /= length; z /=length;
        return *this;
    }
};