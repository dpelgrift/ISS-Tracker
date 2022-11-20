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

    Dcm transpose(Dcm d) {
        return Dcm{x00,x01,x02,x10,x11,x12,x20,x21,x22};
    }

    Dcm operator*(Dcm D) {
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
};


struct Quat {
    
    double w;
    double x;
    double y;
    double z;
    
    Quat() {
        w = 1.0;
        x = 0.0;
        y = 0.0;
        z = 0.0;
    }
    
    Quat(double nw, double nx, double ny, double nz) {
        w = nw;
        x = nx;
        y = ny;
        z = nz;
    }

    Quat getProduct(Quat q) {
        // Quaternion multiplication is defined by:
        //     (Q1 * Q2).w = (w1w2 - x1x2 - y1y2 - z1z2)
        //     (Q1 * Q2).x = (w1x2 + x1w2 + y1z2 - z1y2)
        //     (Q1 * Q2).y = (w1y2 - x1z2 + y1w2 + z1x2)
        //     (Q1 * Q2).z = (w1z2 + x1y2 - y1x2 + z1w2
        return Quat(
            w*q.w - x*q.x - y*q.y - z*q.z,  // new w
            w*q.x + x*q.w + y*q.z - z*q.y,  // new x
            w*q.y - x*q.z + y*q.w + z*q.x,  // new y
            w*q.z + x*q.y - y*q.x + z*q.w); // new z
    }

    Quat getConjugate() {
        return Quat(w, -x, -y, -z);
    }
    
    double getMagnitude() {
        return sqrt(w*w + x*x + y*y + z*z);
    }
    
    void normalize() {
        double m = getMagnitude();
        w /= m;
        x /= m;
        y /= m;
        z /= m;
    }
    
    Quat getNormalized() {
        Quat r(w, x, y, z);
        r.normalize();
        return r;
    }
};

struct Vec3 {
    
    double x;
    double y;
    double z;

    Vec3() {
        x = 0;
        y = 0;
        z = 0;
    }
    
    Vec3(double nx, double ny, double nz) {
        x = nx;
        y = ny;
        z = nz;
    }

    double operator[](uint8_t i) {
        if (i == 0) return x;
        if (i == 1) return y;
        else return z;
    }

    double getMagnitude() {
        return sqrt(x*x + y*y + z*z);
    }

    void normalize() {
        double m = getMagnitude();
        x /= m;
        y /= m;
        z /= m;
    }
    
    Vec3 getNormalized() {
        Vec3 r(x, y, z);
        r.normalize();
        return r;
    }
    
    void rotate(Quat *q) {
        Quat p(0, x, y, z);

        // quaternion multiplication: q * p, stored back in p
        p = q -> getProduct(p);

        // quaternion multiplication: p * conj(q), stored back in p
        p = p.getProduct(q -> getConjugate());

        // p quaternion is now [0, x', y', z']
        x = p.x;
        y = p.y;
        z = p.z;
    }

    Vec3 getRotated(Quat *q) {
        Vec3 r(x, y, z);
        r.rotate(q);
        return r;
    }
};


Vec3 operator*(Vec3 a, Vec3 b) { return Vec3{a.x*b.x,a.y*b.y,a.z*b.z};}
Vec3 operator/(Vec3 a, Vec3 b) { return Vec3{a.x/b.x,a.y/b.y,a.z/b.z};}
Vec3 operator+(Vec3 a, Vec3 b) { return Vec3{a.x+b.x,a.y+b.y,a.z+b.z};}
Vec3 operator-(Vec3 a, Vec3 b) { return Vec3{a.x-b.x,a.y-b.y,a.z-b.z};}

Vec3 operator*(Vec3 a, double b) { return Vec3{a.x*b,a.y*b,a.z*b};}
Vec3 operator/(Vec3 a, double b) { return Vec3{a.x/b,a.y/b,a.z/b};}
Vec3 operator+(Vec3 a, double b) { return Vec3{a.x+b,a.y+b,a.z+b};}
Vec3 operator-(Vec3 a, double b) { return Vec3{a.x-b,a.y-b,a.z-b};}


double dot(Vec3 a, Vec3 b) {return a.x*b.x + a.y*b.y + a.z*b.z;}

double norm(Vec3 v) { return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);}
double norm(Quat v) { return sqrt(v.w*v.w + v.x*v.x + v.y*v.y + v.z*v.z);}

Vec3 rot(Quat q, Vec3 v) {
    Quat p(0, v.x, v.y, v.z);

    // quaternion multiplication: q * p, stored back in p
    p = q.getProduct(p);

    // quaternion multiplication: p * conj(q), stored back in p
    p = p.getProduct(q.getConjugate());

    // p quaternion is now [0, x', y', z']
    return Vec3{p.x,p.y,p.z};
}

Vec3 operator*(Dcm d,Vec3 v) {
    return Vec3 {
        v.x*d.x00 + v.y*d.x01 + v.z*d.x02,
        v.x*d.x10 + v.y*d.x11 + v.z*d.x12,
        v.x*d.x20 + v.y*d.x21 + v.z*d.x22,
    };
}
