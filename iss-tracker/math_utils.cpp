/*
  math_utils.cpp - Utilities to handle vector math
 */
#include "math_utils.h"

// DCM transposition
Dcm Dcm::transpose(Dcm d) {
    return Dcm{x00,x01,x02,x10,x11,x12,x20,x21,x22};
}

// DCM multiplication
Dcm Dcm::operator*(Dcm D) {
    return Dcm {
        x00*D.x00 + x01*D.x10 + x02*D.x20,
        x10*D.x00 + x11*D.x10 + x12*D.x20,
        x20*D.x00 + x21*D.x10 + x22*D.x20,
        x00*D.x01 + x01*D.x11 + x02*D.x21,
        x10*D.x01 + x11*D.x11 + x12*D.x21,
        x20*D.x01 + x21*D.x11 + x22*D.x21,
        x00*D.x02 + x01*D.x12 + x02*D.x22,
        x10*D.x02 + x11*D.x12 + x12*D.x22,
        x20*D.x02 + x21*D.x12 + x22*D.x22,
    };
}

// Vector3 constructors
Vec3::Vec3() {x = 0;y = 0;z = 0;}
Vec3::Vec3(double nx, double ny, double nz) {x = nx; y = ny; z = nz;}

// Vector3 getter operator
double Vec3::operator[](uint8_t i) {
    if (i == 0) return x;
    if (i == 1) return y;
    else return z;
}

// Vector3 math operators
Vec3 operator*(Vec3 a, Vec3 b) { return Vec3{a.x*b.x,a.y*b.y,a.z*b.z};}
Vec3 operator/(Vec3 a, Vec3 b) { return Vec3{a.x/b.x,a.y/b.y,a.z/b.z};}
Vec3 operator+(Vec3 a, Vec3 b) { return Vec3{a.x+b.x,a.y+b.y,a.z+b.z};}
Vec3 operator-(Vec3 a, Vec3 b) { return Vec3{a.x-b.x,a.y-b.y,a.z-b.z};}

Vec3 operator*(Vec3 a, double b) { return Vec3{a.x*b,a.y*b,a.z*b};}
Vec3 operator/(Vec3 a, double b) { return Vec3{a.x/b,a.y/b,a.z/b};}
Vec3 operator+(Vec3 a, double b) { return Vec3{a.x+b,a.y+b,a.z+b};}
Vec3 operator-(Vec3 a, double b) { return Vec3{a.x-b,a.y-b,a.z-b};}

// Vector3 dot product
double dot(Vec3 a, Vec3 b) {return a.x*b.x + a.y*b.y + a.z*b.z;}
// Vector3 normalization
double norm(Vec3 v) { return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);}

// DCM & Vector3 multiplication
Vec3 operator*(Dcm d,Vec3 v) {
    return Vec3 {
        v.x*d.x00 + v.y*d.x01 + v.z*d.x02,
        v.x*d.x10 + v.y*d.x11 + v.z*d.x12,
        v.x*d.x20 + v.y*d.x21 + v.z*d.x22,
    };
}
