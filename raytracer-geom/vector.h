#pragma once

#include <array>
#include <cstddef>
#include <math.h>

class Vector {
public:
    Vector() : data_() {
    }

    Vector(double x, double y, double z) : data_({x, y, z}) {
    }

    double& operator[](size_t ind) {
        return data_[ind];
    }
    double operator[](size_t ind) const {
        return data_[ind];
    }

    Vector operator+(const Vector& other) const {
        double x1 = data_[0];
        double y1 = data_[1];
        double z1 = data_[2];
        double x2 = other.data_[0];
        double y2 = other.data_[1];
        double z2 = other.data_[2];
        return Vector(x1 + x2, y1 + y2, z1 + z2);
    }

    Vector operator-(const Vector& other) const {
        double x1 = data_[0];
        double y1 = data_[1];
        double z1 = data_[2];
        double x2 = other.data_[0];
        double y2 = other.data_[1];
        double z2 = other.data_[2];
        return Vector(x1 - x2, y1 - y2, z1 - z2);
    }

    Vector MultiplyOnScalar(double alpha) const {
        double x1 = data_[0];
        double y1 = data_[1];
        double z1 = data_[2];
        return Vector(x1 * alpha, y1 * alpha, z1 * alpha);
    }

    void Normalize() {
        double x = data_[0];
        double y = data_[1];
        double z = data_[2];
        double norm = std::sqrt(x * x + y * y + z * z);
        data_[0] = x / norm;
        data_[1] = y / norm;
        data_[2] = z / norm;
    }

    double Length() const {
        double x = data_[0];
        double y = data_[1];
        double z = data_[2];
        return std::sqrt(x * x + y * y + z * z);
    }

    double Dot(const Vector& other) const {
        double x = data_[0];
        double y = data_[1];
        double z = data_[2];
        return x * other.data_[0] + y * other.data_[1] + z * other.data_[2];
    }

    Vector CrossProduct(const Vector& other) const {
        double x1 = data_[0];
        double y1 = data_[1];
        double z1 = data_[2];
        double x2 = other.data_[0];
        double y2 = other.data_[1];
        double z2 = other.data_[2];
        double x3 = y1 * z2 - z1 * y2;
        double y3 = z1 * x2 - x1 * z2;
        double z3 = x1 * y2 - y1 * x2;
        return Vector(x3, y3, z3);
    }

private:
    std::array<double, 3> data_;
};

double DotProduct(const Vector& a, const Vector& b) {
    return a.Dot(b);
}

Vector CrossProduct(const Vector& a, const Vector& b) {
    return a.CrossProduct(b);
}

double Length(const Vector& v) {
    return v.Length();
}

double Distance(const Vector& v1, const Vector& v2) {
    Vector v = v1 - v2;
    return v.Length();
}
