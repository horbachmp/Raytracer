#pragma once

#include <vector.h>
#include <sphere.h>
#include <intersection.h>
#include <triangle.h>
#include <ray.h>
#include <stdexcept>
#include <optional>

const double kEps = 0.00001;

std::optional<Intersection> GetIntersection(const Ray& ray, const Sphere& sphere) {
    Vector center = sphere.GetCenter();
    double radius = sphere.GetRadius();
    Vector d = ray.GetDirection();
    d.Normalize();
    Vector orig = ray.GetOrigin();
    Vector ll = center - orig;
    double tca = DotProduct(ll, d);
    double d2 = DotProduct(ll, ll) - tca * tca;
    if (d2 > radius * radius) {
        return std::optional<Intersection>();
    }
    double thc = std::sqrt(radius * radius - d2);
    double t0 = tca - thc;
    double t1 = tca + thc;
    if (t0 > t1) {
        std::swap(t0, t1);
    }
    if (t0 < 0.0) {
        t0 = t1;
        if (t0 < 0.0) {
            return std::optional<Intersection>();
        }
    }
    double t = t0;
    Vector pos = orig + d.MultiplyOnScalar(t);
    double dist = Distance(pos, ray.GetOrigin());
    Vector normal = center - pos;
    normal.Normalize();
    if (DotProduct(normal, d) >= 0) {
        normal = normal.MultiplyOnScalar(-1);
    }
    return Intersection(pos, normal, dist);
}
std::optional<Intersection> GetIntersection(const Ray& ray, const Triangle& triangle) {
    Vector e1 = triangle[1] - triangle[0];
    Vector e2 = triangle[2] - triangle[0];
    Vector d = ray.GetDirection();
    d.Normalize();
    Vector n = CrossProduct(d, e2);
    if (std::abs(DotProduct(e1, n)) < kEps) {
        return std::optional<Intersection>();
    } else {
        Vector tt = ray.GetOrigin() - triangle[0];
        Vector pp = CrossProduct(d, e2);
        Vector qq = CrossProduct(tt, e1);
        double div = DotProduct(pp, e1);
        double t = DotProduct(qq, e2) / div;
        double u = DotProduct(pp, tt) / div;
        double v = DotProduct(qq, d) / div;
        if (t < 0.0) {
            return std::optional<Intersection>();
        }
        if ((u < 0.0 || u > 1.0) || (v < 0.0 || u + v > 1.0)) {
            return std::optional<Intersection>();
        }
        Vector pos = ray.GetOrigin() + d.MultiplyOnScalar(t);
        Vector normal = CrossProduct(pos - triangle[0], pos - triangle[1]);
        normal.Normalize();
        if (DotProduct(normal, d) >= 0) {
            normal = normal.MultiplyOnScalar(-1);
        }
        double dist = Distance(pos, ray.GetOrigin());
        return Intersection(pos, normal, dist);
    }
}

Vector Reflect(const Vector& ray, const Vector& normal) {
    double cos1 = -DotProduct(normal, ray);
    // if (cos1 < 0) ...
    return ray + normal.MultiplyOnScalar(2 * cos1);
}
std::optional<Vector> Refract(const Vector& ray, const Vector& normal, double eta) {
    double cos1 = -DotProduct(normal, ray);
    // if (cos1 < 1.0) {
    //     return std::optional<Vector>();
    // }
    double sin2 = eta * std::sqrt(1 - cos1 * cos1);
    if (sin2 > 1.0) {
        return std::optional<Vector>();
    }
    double cos2 = std::sqrt(1 - sin2 * sin2);
    return ray.MultiplyOnScalar(eta) + normal.MultiplyOnScalar(eta * cos1 - cos2);
}
Vector GetBarycentricCoords(const Triangle& triangle, const Vector& point) {
    Vector v0 = triangle[1] - triangle[0];
    Vector v1 = triangle[2] - triangle[0];
    Vector v2 = point - triangle[0];
    double d00 = DotProduct(v0, v0);
    double d01 = DotProduct(v0, v1);
    double d11 = DotProduct(v1, v1);
    double d20 = DotProduct(v2, v0);
    double d21 = DotProduct(v2, v1);
    double denom = d00 * d11 - d01 * d01;
    double v = (d11 * d20 - d01 * d21) / denom;
    double w = (d00 * d21 - d01 * d20) / denom;
    double u = 1.0 - v - w;
    return Vector(u, v, w);
}
