#pragma once
#include <Arduino.h>

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


struct Quat {
    
    double w;
    double x;
    double y;
    double z;
    
    Quat();
    Quat(double nw, double nx, double ny, double nz);

    Quat getProduct(Quat q);
    Quat getConjugate();
    double getMagnitude();
    void normalize();
    Quat getNormalized();
};

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
    void rotate(Quat *q);
    Vec3 getRotated(Quat *q);
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
double norm(Quat v);

Vec3 rot(Quat q, Vec3 v);

Vec3 operator*(Dcm d,Vec3 v);
