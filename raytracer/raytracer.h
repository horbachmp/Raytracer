#pragma once

#include <iostream>
#include <math.h>

#include <scene.h>
#include <ray.h>
#include <intersection.h>
#include <geometry.h>
#include <object.h>
#include <image.h>
#include <options/camera_options.h>
#include <options/render_options.h>

#include <filesystem>

std::vector<std::vector<double>> LookAt(Vector from, Vector to) {
    std::vector<std::vector<double>> matr(4, std::vector<double>(4));
    for (int i = 0; i < 4; ++i) {
        matr[i][i] = 1;
    }
    Vector forward = from - to;
    forward.Normalize();
    Vector tmp = Vector(0, 1, 0);
    Vector right = CrossProduct(tmp, forward);
    if (Length(right) < 1e-9) {
        if (DotProduct(tmp, forward) < 0) {
            tmp = Vector(0, 0, 1);
        } else {
            tmp = Vector(0, 0, -1);
        }
    }
    if ((to - from)[2] < 0) {
        tmp = Vector(tmp[0], tmp[1], -tmp[2]);
    }
    right = CrossProduct(tmp, forward);
    right.Normalize();
    Vector up = CrossProduct(forward, right);
    up.Normalize();
    for (int i = 0; i < 3; ++i) {
        matr[0][i] = right[i];
        matr[1][i] = up[i];
        matr[2][i] = forward[i];
        matr[3][i] = from[i];
    }
    return matr;
}

Vector MultPointMatrix(Vector point, const std::vector<std::vector<double>> m) {
    Vector ans;
    ans[0] = point[0] * m[0][0] + point[1] * m[1][0] + point[2] * m[2][0] + m[3][0];
    ans[1] = point[0] * m[0][1] + point[1] * m[1][1] + point[2] * m[2][1] + m[3][1];
    ans[2] = point[0] * m[0][2] + point[1] * m[1][2] + point[2] * m[2][2] + m[3][2];
    double w = point[0] * m[0][3] + point[1] * m[1][3] + point[2] * m[2][3] + m[3][3];
    if (w != 1 && w != 0) {
        ans[0] /= w;
        ans[1] /= w;
        ans[2] /= w;
    }
    return ans;
}

Vector MultDirMatrix(Vector point, const std::vector<std::vector<double>> m) {
    Vector ans;
    ans[0] = point[0] * m[0][0] + point[1] * m[1][0] + point[2] * m[2][0];
    ans[1] = point[0] * m[0][1] + point[1] * m[1][1] + point[2] * m[2][1];
    ans[2] = point[0] * m[0][2] + point[1] * m[1][2] + point[2] * m[2][2];
    return ans;
}

bool IsLightVis(const Light& light, Vector pos, const std::vector<Object>& objects,
                const std::vector<SphereObject>& sph_objects, Vector normal) {
    Vector dir = light.position - pos;
    double dist = Distance(light.position, pos);
    dir.Normalize();
    Ray ray = Ray(pos + normal.MultiplyOnScalar(0.0000000001), dir);
    for (auto& obj : objects) {
        std::optional<Intersection> intr = GetIntersection(ray, obj.polygon);
        if (intr.has_value()) {
            if (intr->GetDistance() <= dist) {
                return false;
            }
        }
    }
    for (auto& obj : sph_objects) {
        std::optional<Intersection> intr = GetIntersection(ray, obj.sphere);
        if (intr.has_value()) {
            if (intr->GetDistance() <= dist) {
                return false;
            }
        }
    }
    return true;
}

Vector MultiplyComp(Vector a, Vector b) {
    return Vector(a[0] * b[0], a[1] * b[1], a[2] * b[2]);
}

Vector GetPointColorBase(const std::vector<Object>& objects,
                         const std::vector<SphereObject>& sph_objects,
                         const std::vector<Light>& lights, Vector normal, const Material mat,
                         Vector pos, Vector ray) {
    Vector color = mat.ambient_color + mat.intensity;
    Vector rr;
    Vector light;
    Vector vv = ray.MultiplyOnScalar(-1);
    vv.Normalize();
    normal.Normalize();
    for (size_t i = 0; i < lights.size(); ++i) {
        if (IsLightVis(lights[i], pos, objects, sph_objects, normal)) {
            light = lights[i].position - pos;
            light.Normalize();
            rr = normal.MultiplyOnScalar(2 * DotProduct(light, normal)) - light;
            rr.Normalize();
            double d2 = std::pow(std::max(0.0, DotProduct(rr, vv)), mat.specular_exponent);
            Vector spec = MultiplyComp(mat.specular_color, lights[i].intensity);
            Vector specular = spec.MultiplyOnScalar(d2);
            double d1 = std::max(0.0, DotProduct(light, normal));
            Vector diffuse =
                MultiplyComp(mat.diffuse_color.MultiplyOnScalar(d1), lights[i].intensity);
            Vector c = diffuse + specular;
            color = color + c.MultiplyOnScalar(mat.albedo[0]);
        }
    }
    return color;
}

Vector GetPixelColor(const std::vector<Object>& objects,
                     const std::vector<SphereObject>& sph_objects, const std::vector<Light>& lights,
                     Ray ray, int rec_depth, int is_inside) {
    if (rec_depth == -1) {
        return {0, 0, 0};
    }
    double min_d = INFINITY;
    bool is_sphere = 0;
    int min_obj = -1;
    std::optional<Intersection> intr_min;
    Vector pixel_c;
    int counter = 0;
    for (auto& obj : objects) {
        std::optional<Intersection> intr = GetIntersection(ray, obj.polygon);
        if (intr.has_value()) {
            double dist = intr->GetDistance();
            if (dist < min_d) {
                min_d = dist;
                min_obj = counter;
                intr_min = intr;
            }
        }
        ++counter;
    }
    counter = 0;
    for (auto& obj : sph_objects) {
        std::optional<Intersection> intr = GetIntersection(ray, obj.sphere);
        if (intr.has_value()) {
            double dist = intr->GetDistance();
            if (dist < min_d) {
                min_d = dist;
                min_obj = counter;
                intr_min = intr;
                is_sphere = true;
            }
        }
        ++counter;
    }
    Vector normal = {0, 0, 0};
    Material mat;
    if (min_obj == -1) {
        return {0, 0, 0};
    }
    if (!is_sphere) {
        Object obj = objects[min_obj];
        normal = intr_min->GetNormal();
        if (!obj.normals.empty()) {
            Vector bars = GetBarycentricCoords(obj.polygon, intr_min->GetPosition());
            normal = obj.normals[0].MultiplyOnScalar(bars[0]) +
                     obj.normals[1].MultiplyOnScalar(bars[1]) +
                     obj.normals[2].MultiplyOnScalar(bars[2]);
        }
        mat = *obj.material;
        Vector c = GetPointColorBase(objects, sph_objects, lights, normal, mat,
                                     intr_min->GetPosition(), ray.GetDirection());
        pixel_c = c;
    } else {
        SphereObject obj = sph_objects[min_obj];
        normal = intr_min->GetNormal();
        mat = *obj.material;
        Vector c = GetPointColorBase(objects, sph_objects, lights, normal, mat,
                                     intr_min->GetPosition(), ray.GetDirection());
        pixel_c = c;
    }
    Vector pos = intr_min->GetPosition();
    Vector ray_dir = ray.GetDirection();
    ray_dir.Normalize();
    if (is_inside == 0) {
        Vector refl_ray_dir = Reflect(ray_dir, normal);
        refl_ray_dir.Normalize();
        Ray refl_ray = {pos + normal.MultiplyOnScalar(0.000000001), refl_ray_dir};
        Vector i_refl = GetPixelColor(objects, sph_objects, lights, refl_ray, rec_depth - 1, 0);
        pixel_c = pixel_c + i_refl.MultiplyOnScalar(mat.albedo[1]);
    }
    double eta = 1.0 / mat.refraction_index;
    double tr_coef = mat.albedo[2];
    if (is_inside == 1) {
        tr_coef = 1.0;
        eta = mat.refraction_index;
    }
    std::optional<Vector> retr_ray_dir_opt = Refract(ray_dir, normal, eta);
    if (retr_ray_dir_opt.has_value()) {
        Vector retr_ray_dir = *retr_ray_dir_opt;
        retr_ray_dir.Normalize();
        Ray retr_ray = {pos - normal.MultiplyOnScalar(0.000000002), retr_ray_dir};
        Vector i_retr = GetPixelColor(objects, sph_objects, lights, retr_ray, rec_depth - 1,
                                      is_sphere && (1 - is_inside));
        pixel_c = pixel_c + i_retr.MultiplyOnScalar(tr_coef);
    }
    return pixel_c;
}

Image Render(const std::filesystem::path& path, const CameraOptions& camera_options,
             const RenderOptions& render_options) {
    // std::cout<<"hi\n";
    Image img = Image(camera_options.screen_width, camera_options.screen_height);
    Scene scene = ReadScene(path);
    std::vector<Object> objects = scene.GetObjects();
    std::vector<SphereObject> sph_objects = scene.GetSphereObjects();
    std::vector<Light> lights = scene.GetLights();
    std::unordered_map<std::string, Material> materials = scene.GetMaterials();
    double height = 2 * std::tan(camera_options.fov / 2);
    double pixel_size = height / camera_options.screen_height;
    double width = height * camera_options.screen_width / camera_options.screen_height;
    double curr_x = -width / 2 + pixel_size / 2;
    double curr_y = height / 2 - pixel_size / 2;
    std::vector<std::vector<double>> m = LookAt(camera_options.look_from, camera_options.look_to);
    if (render_options.mode == RenderMode::kDepth) {
        std::vector<std::vector<double>> pixel_d(
            camera_options.screen_height, std::vector<double>(camera_options.screen_width, -1.0));
        double max_d = 0;
        for (int i = 0; i < camera_options.screen_height; ++i) {
            curr_x = -width / 2 + pixel_size / 2;
            for (int j = 0; j < camera_options.screen_width; ++j) {
                Vector ray_dir = Vector(curr_x, curr_y, -1);
                ray_dir.Normalize();
                Vector dir_world = MultDirMatrix(ray_dir, m);
                dir_world.Normalize();
                Ray ray = Ray(camera_options.look_from, dir_world);
                double min_d = INFINITY;
                for (auto& obj : objects) {
                    std::optional<Intersection> intr = GetIntersection(ray, obj.polygon);
                    if (intr.has_value()) {
                        double dist = intr->GetDistance();
                        if (dist < min_d) {
                            min_d = dist;
                        }
                    }
                }
                for (auto& obj : sph_objects) {
                    std::optional<Intersection> intr = GetIntersection(ray, obj.sphere);
                    if (intr.has_value()) {
                        double dist = intr->GetDistance();
                        if (dist < min_d) {
                            min_d = dist;
                        }
                    }
                }
                if (min_d < INFINITY) {
                    pixel_d[i][j] = min_d;
                    if (min_d > max_d) {
                        max_d = min_d;
                    }
                }
                curr_x += pixel_size;
            }
            curr_y -= pixel_size;
        }
        for (int i = 0; i < camera_options.screen_height; ++i) {
            for (int j = 0; j < camera_options.screen_width; ++j) {
                if (pixel_d[i][j] == -1.0) {
                    img.SetPixel({255, 255, 255}, i, j);
                } else {
                    int c = std::round(pixel_d[i][j] / max_d * 255);
                    img.SetPixel({c, c, c}, i, j);
                }
            }
        }
    } else if (render_options.mode == RenderMode::kFull) {
        std::vector<std::vector<Vector>> pixel_d(
            camera_options.screen_height,
            std::vector<Vector>(camera_options.screen_width, {0, 0, 0}));
        for (int i = 0; i < camera_options.screen_height; ++i) {
            curr_x = -width / 2 + pixel_size / 2;
            for (int j = 0; j < camera_options.screen_width; ++j) {
                Vector ray_dir = Vector(curr_x, curr_y, -1);
                ray_dir.Normalize();
                Vector dir_world = MultDirMatrix(ray_dir, m);
                dir_world.Normalize();
                Ray ray = Ray(camera_options.look_from, dir_world);
                pixel_d[i][j] =
                    GetPixelColor(objects, sph_objects, lights, ray, render_options.depth, 0);
                curr_x += pixel_size;
            }
            curr_y -= pixel_size;
        }
        double max_c = 0;
        for (int i = 0; i < camera_options.screen_height; ++i) {
            for (int j = 0; j < camera_options.screen_width; ++j) {
                max_c = std::max(max_c, pixel_d[i][j][0]);
                max_c = std::max(max_c, pixel_d[i][j][1]);
                max_c = std::max(max_c, pixel_d[i][j][2]);
            }
        }
        for (int i = 0; i < camera_options.screen_height; ++i) {
            for (int j = 0; j < camera_options.screen_width; ++j) {
                Vector v = pixel_d[i][j];
                Vector c_new;
                for (int h = 0; h < 3; ++h) {
                    double tmp = v[h] * (1 + v[h] / (max_c * max_c)) / (1 + v[h]);
                    c_new[h] = std::pow(tmp, 1 / 2.2);
                }
                if (!(c_new[0] == c_new[0])) {
                    c_new[0] = 0;
                }
                if (!(c_new[1] == c_new[1])) {
                    c_new[1] = 0;
                }
                if (!(c_new[2] == c_new[2])) {
                    c_new[2] = 0;
                }
                int c1 = std::round(c_new[0] * 255);
                int c2 = std::round(c_new[1] * 255);
                int c3 = std::round(c_new[2] * 255);
                img.SetPixel({c1, c2, c3}, i, j);
            }
        }
    } else if (render_options.mode == RenderMode::kNormal) {
        std::vector<std::vector<double>> pixel_d(
            camera_options.screen_height, std::vector<double>(camera_options.screen_width, 0.0));
        for (int i = 0; i < camera_options.screen_height; ++i) {
            curr_x = -width / 2 + pixel_size / 2;
            for (int j = 0; j < camera_options.screen_width; ++j) {
                Vector ray_dir = Vector(curr_x, curr_y, -1);
                ray_dir.Normalize();
                Vector dir_world = MultDirMatrix(ray_dir, m);
                dir_world.Normalize();
                Ray ray = Ray(camera_options.look_from, dir_world);
                double min_d = 1000000000;
                for (auto& obj : objects) {
                    std::optional<Intersection> intr = GetIntersection(ray, obj.polygon);
                    if (intr.has_value()) {
                        double dist = intr->GetDistance();
                        if (dist < min_d) {
                            min_d = dist;
                            Vector normal = intr->GetNormal();
                            if (!obj.normals.empty()) {
                                Vector bars =
                                    GetBarycentricCoords(obj.polygon, intr->GetPosition());
                                normal = obj.normals[0].MultiplyOnScalar(bars[0]) +
                                         obj.normals[1].MultiplyOnScalar(bars[1]) +
                                         obj.normals[2].MultiplyOnScalar(bars[2]);
                            }
                            normal = normal.MultiplyOnScalar(0.5) + Vector(0.5, 0.5, 0.5);
                            int c1 = std::round(normal[0] * 255);
                            int c2 = std::round(normal[1] * 255);
                            int c3 = std::round(normal[2] * 255);
                            img.SetPixel({c1, c2, c3}, i, j);
                        }
                    }
                }
                for (auto& obj : sph_objects) {
                    std::optional<Intersection> intr = GetIntersection(ray, obj.sphere);
                    if (intr.has_value()) {
                        double dist = intr->GetDistance();
                        if (dist < min_d) {
                            min_d = dist;
                            Vector normal = intr->GetNormal();
                            normal = normal.MultiplyOnScalar(0.5) + Vector(0.5, 0.5, 0.5);
                            int c1 = std::round(normal[0] * 255);
                            int c2 = std::round(normal[1] * 255);
                            int c3 = std::round(normal[2] * 255);
                            img.SetPixel({c1, c2, c3}, i, j);
                        }
                    }
                }
                curr_x += pixel_size;
            }
            curr_y -= pixel_size;
        }
    }
    return img;
}
