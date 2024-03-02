#pragma once

#include <vector>

#include <triangle.h>
#include <material.h>
#include <sphere.h>
#include <vector.h>

struct Object {
    const Material* material = nullptr;
    Triangle polygon;
    std::vector<Vector> normals;

    const Vector* GetNormal(size_t index) const {
        return &normals[index];
    }
};

struct SphereObject {
    const Material* material = nullptr;
    Sphere sphere;
};
