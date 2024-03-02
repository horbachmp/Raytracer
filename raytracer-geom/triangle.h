#pragma once

#include <vector.h>

class Triangle {
public:
    Triangle(const Vector& a, const Vector& b, const Vector& c) : a_(a), b_(b), c_(c) {
    }

    const Vector& operator[](size_t ind) const {
        if (ind == 0) {
            return a_;
        }
        if (ind == 1) {
            return b_;
        }
        return c_;
    }

    double Area() const {
        double s1 = Distance(a_, b_);
        double s2 = Distance(a_, c_);
        double s3 = Distance(c_, b_);
        double p = (s1 + s2 + s3) / 2;
        double area = std::sqrt(p * (p - s1) * (p - s2) * (p - s3));
        return area;
    }

private:
    Vector a_;
    Vector b_;
    Vector c_;
};
