#pragma once

#include <material.h>
#include <vector.h>
#include <object.h>
#include <light.h>

#include <vector>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>

class Scene {
public:
    Scene(std::vector<Object> objects, std::vector<SphereObject> sph_objects,
          std::vector<Light> lights, std::unordered_map<std::string, Material>&& mats)
        : objects_(objects),
          sph_objects_(sph_objects),
          lights_(lights),
          materials_(std::move(mats)) {
    }

    const std::vector<Object>& GetObjects() const {
        return objects_;
    }
    const std::vector<SphereObject>& GetSphereObjects() const {
        return sph_objects_;
    }
    const std::vector<Light>& GetLights() const {
        return lights_;
    }
    const std::unordered_map<std::string, Material>& GetMaterials() const {
        return materials_;
    }

private:
    std::vector<Object> objects_;
    std::vector<SphereObject> sph_objects_;
    std::vector<Light> lights_;
    std::unordered_map<std::string, Material> materials_;
};

std::unordered_map<std::string, Material> ReadMaterials(const std::filesystem::path& path) {
    std::unordered_map<std::string, Material> ans;
    std::ifstream file(path);
    std::string line;
    file >> line;
    while (!file.eof()) {
        if (line == "#") {
            std::getline(file, line);
        } else if (line == "newmtl") {
            Material mat;
            mat.albedo = {1, 0, 0};
            mat.refraction_index = 1.0;
            mat.ambient_color = {0, 0, 0};
            mat.diffuse_color = {0, 0, 0};
            mat.specular_color = {0, 0, 0};
            mat.intensity = {0, 0, 0};
            mat.specular_exponent = 1;
            file >> mat.name;
            file >> line;
            while (!file.eof() && line != "newmtl") {
                if (line == "#") {
                    std::getline(file, line);
                } else if (line == "Ka") {
                    file >> mat.ambient_color[0] >> mat.ambient_color[1] >> mat.ambient_color[2];
                } else if (line == "Kd") {
                    file >> mat.diffuse_color[0] >> mat.diffuse_color[1] >> mat.diffuse_color[2];
                } else if (line == "Ks") {
                    file >> mat.specular_color[0] >> mat.specular_color[1] >> mat.specular_color[2];
                } else if (line == "Ke") {
                    file >> mat.intensity[0] >> mat.intensity[1] >> mat.intensity[2];
                } else if (line == "Ns") {
                    file >> mat.specular_exponent;
                } else if (line == "Ni") {
                    file >> mat.refraction_index;
                } else if (line == "al") {
                    file >> mat.albedo[0] >> mat.albedo[1] >> mat.albedo[2];
                }
                file >> line;
            }
            ans[mat.name] = mat;
        } else {
            file >> line;
        }
    }
    return ans;
}

std::string StripString(std::string s) {
    size_t i = 0;
    while (std::isspace(s[i])) {
        ++i;
    }
    size_t j = s.size() - 1;
    while (std::isspace(s[j])) {
        --j;
    }
    std::string ans = s.substr(i, j - i + 1);
    return ans;
}

std::vector<std::string> SplitString(std::string s, std::string del) {
    std::vector<std::string> res;
    size_t pos = 0;
    pos = s.find(del);
    while (pos != std::string::npos) {
        res.push_back(s.substr(0, pos));
        s.erase(0, pos + del.size());
        pos = s.find(del);
    }
    res.push_back(s);
    return res;
}

std::vector<int> GetVertex(std::string s) {
    size_t found = s.find("//");
    std::vector<int> ans;
    if (found != std::string::npos) {
        std::vector<std::string> result = SplitString(StripString(s), "//");
        for (auto x : result) {
            if (!x.empty()) {
                ans.push_back(std::stoi(x));
            }
        }
    } else {
        std::vector<std::string> result = SplitString(StripString(s), "/");
        for (auto x : result) {
            if (!x.empty()) {
                ans.push_back(std::stoi(x));
            }
        }
    }
    if (ans.size() <= 2) {
        return ans;
    }
    std::swap(ans[1], ans[2]);
    ans.pop_back();
    return ans;
}

Scene ReadScene(const std::filesystem::path& path) {
    std::filesystem::path directory = path.parent_path();
    std::vector<Vector> points;
    std::vector<Vector> normals;
    std::ifstream file(path);
    std::string line;
    std::unordered_map<std::string, Material> materials;
    std::vector<Object> objects;
    std::vector<SphereObject> sph_objects;
    std::vector<Light> lights;
    std::string curr_material;
    while (file >> line) {
        if (line == "mtllib") {
            std::string mtl_path;
            file >> mtl_path;
            materials = ReadMaterials(directory / mtl_path);
        } else if (line == "v") {
            double x, y, z;
            file >> x >> y >> z;
            points.emplace_back(x, y, z);
        } else if (line == "vn") {
            double x, y, z;
            file >> x >> y >> z;
            normals.emplace_back(x, y, z);
        } else if (line == "usemtl") {
            file >> curr_material;
        } else if (line == "S") {
            double x, y, z, r;
            file >> x >> y >> z >> r;
            Sphere s(Vector(x, y, z), r);
            SphereObject sph(&materials[curr_material], s);
            sph_objects.push_back(sph);
        } else if (line == "P") {
            double x, y, z, r, g, b;
            file >> x >> y >> z >> r >> g >> b;
            Light ll(Vector(x, y, z), Vector(r, g, b));
            lights.push_back(ll);
        } else if (line == "f") {
            std::string s;
            std::getline(file, s);
            std::vector<std::string> vertices = SplitString(StripString(s), " ");
            std::string v0 = vertices[0];
            std::string v1 = vertices[1];
            std::string v2 = vertices[2];
            std::vector<int> num_v0 = GetVertex(v0);
            int ind = 0;
            if (num_v0.size() == 2) {
                ind = 1;
            }
            int v0_ind = num_v0[0];
            if (v0_ind < 0) {
                v0_ind = num_v0[0] + points.size();
            } else {
                --v0_ind;
            }
            std::vector<int> num_v1 = GetVertex(v1);
            int v1_ind = num_v1[0];
            if (v1_ind < 0) {
                v1_ind = num_v1[0] + points.size();
            } else {
                --v1_ind;
            }
            std::vector<int> num_v2 = GetVertex(v2);
            int v2_ind = num_v2[0];
            if (v2_ind < 0) {
                v2_ind = num_v2[0] + points.size();
            } else {
                --v2_ind;
            }
            int v0_n, v1_n, v2_n;
            if (ind) {
                v0_n = num_v0[1];
                if (v0_n < 0) {
                    v0_n = num_v0[1] + normals.size();
                } else {
                    --v0_n;
                }
                v1_n = num_v1[1];
                if (v1_n < 0) {
                    v1_n = num_v1[1] + normals.size();
                } else {
                    --v1_n;
                }
                v2_n = num_v2[1];
                if (v2_n < 0) {
                    v2_n = num_v2[1] + normals.size();
                } else {
                    --v2_n;
                }
                std::vector<Vector> nrmls = {normals[v0_n], normals[v1_n], normals[v2_n]};
                objects.push_back(Object(&materials[curr_material],
                                         Triangle(points[v0_ind], points[v1_ind], points[v2_ind]),
                                         nrmls));
            } else {
                objects.push_back(Object(&materials[curr_material],
                                         Triangle(points[v0_ind], points[v1_ind], points[v2_ind]),
                                         {}));
            }
            for (size_t i = 3; i < vertices.size(); ++i) {
                v1 = v2;
                v1_ind = v2_ind;
                v2 = vertices[i];
                num_v2 = GetVertex(v2);
                v2_ind = num_v2[0];
                if (v2_ind < 0) {
                    v2_ind = num_v2[0] + points.size();
                } else {
                    --v2_ind;
                }
                if (ind) {
                    v1_n = v2_n;
                    v2_n = num_v2[1];
                    if (v2_n < 0) {
                        v2_n = num_v2[1] + normals.size();
                    } else {
                        --v2_n;
                    }
                    std::vector<Vector> nrmls = {normals[v0_n], normals[v1_n], normals[v2_n]};
                    objects.push_back(
                        Object(&materials[curr_material],
                               Triangle(points[v0_ind], points[v1_ind], points[v2_ind]), nrmls));
                } else {
                    objects.push_back(
                        Object(&materials[curr_material],
                               Triangle(points[v0_ind], points[v1_ind], points[v2_ind]), {}));
                }
            }
        }
    }
    return Scene(objects, sph_objects, lights, std::move(materials));
}
