/*
  math_utils.h - Direction-Cosine-Matrix (DCM) and 3-Element Vector (Vec3) struct definitions
 */
#pragma once
#include <Arduino.h>

// Direction-Cosine-Matrix Definition
struct Dcm {
    
    double x00;
    double x10;
    double x20;
    double x01;
    double x11;
    double x21;
    double x02;
    double x12;
    double x22;

    Dcm transpose(Dcm d);
    Dcm operator*(Dcm D);
};

// 3-Element Vector Definition
struct Vec3 {
    
    double x;
    double y;
    double z;

    Vec3();
    Vec3(double nx, double ny, double nz);

    double operator[](uint8_t i);
    double getMagnitude();
    void normalize();
    Vec3 getNormalized();
};


Vec3 operator*(Vec3 a, Vec3 b);
Vec3 operator/(Vec3 a, Vec3 b);
Vec3 operator+(Vec3 a, Vec3 b);
Vec3 operator-(Vec3 a, Vec3 b);

Vec3 operator*(Vec3 a, double b);
Vec3 operator/(Vec3 a, double b);
Vec3 operator+(Vec3 a, double b);
Vec3 operator-(Vec3 a, double b);


double dot(Vec3 a, Vec3 b);
double norm(Vec3 v);

Vec3 operator*(Dcm d,Vec3 v);
