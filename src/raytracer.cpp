#define _USE_MATH_DEFINES
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <vector>
#include <chrono>
#include <string>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include "object.h"
#include "sphere.h"
#include "plane.h"
#include "ray.h"


#include "glm/gtx/string_cast.hpp"


// write PPM image file (input parameters: image width in px, image height in px, image data (vector of uint8 vec3), filename)
bool writeP6PPM(unsigned int dX, unsigned int dY, std::vector<glm::u8vec3> img, std::string filename = "rtimage") {
    // return false if image size does not fit data size
    try {
        if (img.size() != dX * dY) return false;
        // PPM writing code adapted from: https://rosettacode.org/wiki/Bitmap/Write_a_PPM_file#C++
        std::ofstream ofs(filename + ".ppm", std::ios_base::out | std::ios_base::binary);
        if (ofs.fail()) return false;
        ofs << "P6" << std::endl << dX << ' ' << dY << std::endl << "255" << std::endl;
        for (unsigned int j = 0; j < dY; ++j)
            for (unsigned int i = 0; i < dX; ++i)
                ofs << img[j * dX + i].r << img[j * dX + i].g << img[j * dX + i].b;
        ofs.close();
    }
    catch (const std::exception& e) {
        // return false if an exception occurred
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

// generate the correct RGB vector for Phong illumination model
glm::vec3 phongShading(Object* obj, glm::vec3& intersectPos, glm::vec3& normal, Ray& ray,
                       glm::vec3& light, glm::vec3 k_a, glm::vec3 k_d){
    glm::vec3 res(0.0f, 0.0f, 0.0f);
    float I_a = obj->AmbientFactor(); // ambient intensity
//    glm::vec3 k_a = obj->Color()*255.0f; // ambient coefficients

    float I_i = 1.0f; // light intensity
//    glm::vec3 k_d = obj->Color()*255.0f; // diffuse coefficients

    float n = obj->SpecularExponent(); // specular exponent/Phong exponent
    glm::vec3 k_s(1.0f, 1.0f, 1.0f); // specular coefficient
    k_s *= 255.0f;

    glm::vec3 l = glm::normalize(light - intersectPos); // direction to light
    // Ambient Component
    res += I_a*k_a;

    // Diffuse Component
    float ln = glm::clamp(glm::dot(l,normal), .0f, 1.0f); // cosine
    res += I_i*k_d*ln;

    // Specular Reflection
    glm::vec3 r = glm::normalize(2*ln*normal - l); // reflection ray
    glm::vec3 v = glm::normalize(ray.orig - intersectPos); // direction to camera
    float rv = glm::clamp(glm::dot(v, r), 0.0f, 1.0f);
    res += I_i*k_s*float(pow(rv, n));

    return glm::clamp(res, 0.0f, 255.0f);
}

// generate the correct RGB vector for shadow color
glm::vec3 phongShadows(Object* obj, glm::vec3 k_a){
    glm::vec3 res(0.0f, 0.0f, 0.0f);
    float I_a = obj->AmbientFactor(); // ambient intensity
//    glm::vec3 k_a = obj->Color()*255.0f; // ambient coefficients

    // Ambient Component
    res += I_a*k_a;

    return glm::clamp(res, 0.0f, 255.0f);
}
// check whether there is an object intersected by the ray starting from the light to a point
bool isIntersected(std::vector<Object*> objs, Object* obj,
                   glm::vec3& intersectPosOnObj, glm::vec3& light){
    glm::vec3 intersectPos;
    glm::vec3 normal;

    float distObjToLight = glm::distance(light, intersectPosOnObj);
    glm::vec3 lightToObj = glm::normalize( intersectPosOnObj - light);

    // loop through all other objects
    // and check whether there is an object intersected in the middle by checking the distance
    for(Object* currObj : objs){
        if(currObj == obj) continue;

        float dist_ = currObj->intersect(light, lightToObj, intersectPos, normal);

        if(dist_ < 0) continue; // either -1(no interset) or behind the intersectPosOnObj point

        if(dist_ <= distObjToLight){
            return true;
        }
    }

    return false;
}


glm::vec3 reflect(std::vector<Object*>& objs, Object* obj, glm::vec3& intersectPos, glm::vec3& normal, Ray& ray,
                  glm::vec3& light, int level){
    if(level == 0){
        // return the color of the last object
        return glm::vec3(300.0f, 300.0f, 300.0f); // invalid vector
    }

    glm::vec3 reflectedRay = glm::normalize(glm::reflect(ray.direction(), normal));

    float minDistance = FLT_MAX;
    bool setFlag = false;
    Object* closeObj = nullptr;

    glm::vec3 intersectPosCurr;
    glm::vec3 normalCurr;
    glm::vec3 closeIntersectPos;
    glm::vec3 closeNormal;
    for(Object* objCurr : objs){
        if(objCurr == obj) continue;
        float dist_ = objCurr->intersect(intersectPos, reflectedRay, intersectPosCurr, normalCurr);

//        -0.1f
        if(dist_ > -FLT_EPSILON){
            if(!setFlag || minDistance > dist_){
                setFlag = true;
                minDistance = dist_;
                closeObj = objCurr;
                closeIntersectPos = intersectPosCurr;
                closeNormal = normalCurr;
            }
        }
    }
    if(setFlag){
        if(closeObj->Reflect()){
            Ray nextRay(intersectPos, reflectedRay);
            glm::vec3 next = reflect(objs, closeObj, closeIntersectPos, closeNormal, nextRay, light, level-1);
            if(next.x < 255.1f){
                if(isIntersected(objs, closeObj, closeIntersectPos, light)) {
                    return phongShadows(closeObj, next);
                }
                return phongShading(closeObj, closeIntersectPos, closeNormal, ray, light, next, next);
            }
        }
        if(isIntersected(objs, closeObj, closeIntersectPos, light)) {
            return phongShadows(closeObj, closeObj->Color()*255.0f);
        }
        return phongShading(closeObj, closeIntersectPos, closeNormal, ray, light, closeObj->Color()*255.0f, closeObj->Color()*255.0f);

    }else{
        return glm::vec3(300.0f, 300.0f, 300.0f); // invalid vector
    }
}


int main() {
    // image dimensions
    int dimx = 800;
    int dimy = 600;

    // TODO Set up camera, light etc.
    float focal_length = 1.0f;
    float viewport_width = 2.0f * tan(glm::radians(45.0f / 2.0f));
    float viewport_height = viewport_width * 3.0f / 4.0f;

    glm::vec3 e = glm::vec3(0.0, 0.0, 0.0); // camera position
    glm::vec3 v = glm::vec3(0.0,1.0,0.0); // camera up direction
    glm::vec3 w = glm::vec3(0.0,0.0,1.0); // camera view direction
    glm::vec3 u = glm::cross(v,w); // camera right direction

    glm::vec3 horizontal = viewport_width * u;
    glm::vec3 vertical = viewport_height * v;
    glm::vec3 lower_left_corner = e - horizontal/2.0f - vertical/2.0f - w*focal_length;

    // image data
    std::vector<glm::u8vec3> image;
    image.resize(dimx * dimy);
    // start time measurement
    auto start = std::chrono::high_resolution_clock::now();

    // TODO Loop over all pixels
    // TODO    Compute ray-object intersections etc.
    // TODO    Store color per pixel in image vector

    std::vector<Ray> ray;
    ray.resize(dimx * dimy);

    int k = 0;
    for (int j = dimy-1; j >= 0; --j) {
        for (int i = 0; i < dimx; ++i) {
            glm::vec3 direction = glm::normalize(lower_left_corner + float(i)/float(dimx-1)*horizontal
                                                 + float(j)/float(dimy-1)*vertical - e);

//            image[k] = direction*255.0f; // code for non-clamped images

            glm::vec3 directionClamped = glm::clamp(direction, 0.0f, 1.0f);
            image[k] = directionClamped*255.0f; // clamped

            ray[k] = Ray(e, direction);

            k++;
        }
    }

    // write part1 clamped image
    writeP6PPM((unsigned int)dimx, (unsigned int)dimy, image, "part1_clamped");


    glm::vec3 light = e - u*1.9f + v*1.9f;

    std::vector<Object*> objs;
    objs.push_back(new Sphere(glm::vec3(1.0, 0.5, 0.0), 0.75, glm::vec3(0.0, 0.0, -5.0), true));
    objs.push_back(new Sphere(glm::vec3(0.0, 1.0, 0.5), 0.5, glm::vec3(1.0, 0.0, -5.5), false));
    objs.push_back(new Sphere(glm::vec3(0.0, 0.5, 1.0), 0.2, glm::vec3(-1.0, 0.5, -3.0), false));
    objs.push_back(new Sphere(glm::vec3(1.0, 0.5, 0.5), 0.2, glm::vec3(-0.5, -0.5, -2.5), false));

    // part 2 spheres
    k= 0;
    for (int j = dimy-1; j >= 0; --j) {
        for (int i = 0; i < dimx; ++i) {
            float minDistance = FLT_MAX;
            bool setFlag = false;
            Object* closeObj = nullptr;

            glm::vec3 intersectPos;
            glm::vec3 normal;
            glm::vec3 closeIntersectPos;
            glm::vec3 closeNormal;

            for(Object* obj : objs){
                float dist_ = obj->intersect(ray[k].origin(), ray[k].direction(), intersectPos, normal);

                if(dist_ != -1.0f){
                    if(!setFlag || minDistance > dist_){
                        setFlag = true;
                        minDistance = dist_;
                        closeObj = obj;
                        closeIntersectPos = intersectPos;
                        closeNormal = normal;
                    }
                }
            }
            if(setFlag){
                image[k] = closeObj->Color()*255.0f;
            }else{
                image[k] = glm::vec3(0.5, 0.0, 1.0)*255.0f;
            }
            k++;
        }
    }
    // write part2 spheres
    writeP6PPM((unsigned int)dimx, (unsigned int)dimy, image, "part2_spheres");

    // part 3 shading
    k= 0;
    for (int j = dimy-1; j >= 0; --j) {
        for (int i = 0; i < dimx; ++i) {
            float minDistance = FLT_MAX;
            bool setFlag = false;
            Object* closeObj = nullptr;

            glm::vec3 intersectPos;
            glm::vec3 normal;
            glm::vec3 closeIntersectPos;
            glm::vec3 closeNormal;

            for(Object* obj : objs){
                float dist_ = obj->intersect(ray[k].origin(), ray[k].direction(), intersectPos, normal);

                if(dist_ != -1.0f){
                    if(!setFlag || minDistance > dist_){
                        setFlag = true;
                        minDistance = dist_;
                        closeObj = obj;
                        closeIntersectPos = intersectPos;
                        closeNormal = normal;
                    }
                }
            }
            if(setFlag){
                image[k] = phongShading(closeObj, closeIntersectPos, closeNormal, ray[k], light, closeObj->Color()*255.0f, closeObj->Color()*255.0f);
            }else{
                image[k] = glm::vec3(0.5, 0.0, 1.0)*255.0f;
            }
            k++;
        }
    }
    // write part3 shading
    writeP6PPM((unsigned int)dimx, (unsigned int)dimy, image, "part3_shading");

    // part 4 shadows
    k= 0;
    for (int j = dimy-1; j >= 0; --j) {
        for (int i = 0; i < dimx; ++i) {
            float minDistance = FLT_MAX;
            bool setFlag = false;
            Object* closeObj = nullptr;

            glm::vec3 intersectPos;
            glm::vec3 normal;
            glm::vec3 closeIntersectPos;
            glm::vec3 closeNormal;

            for(Object* obj : objs){
                float dist_ = obj->intersect(ray[k].origin(), ray[k].direction(), intersectPos, normal);

                if(dist_ != -1.0f){
                    if(!setFlag || minDistance > dist_){
                        setFlag = true;
                        minDistance = dist_;
                        closeObj = obj;
                        closeIntersectPos = intersectPos;
                        closeNormal = normal;
                    }
                }
            }
            if(setFlag){
                if(isIntersected(objs, closeObj, closeIntersectPos, light)){
                    image[k] = phongShadows(closeObj, closeObj->Color()*255.0f);
                }else{
                    image[k] = phongShading(closeObj, closeIntersectPos, closeNormal, ray[k], light, closeObj->Color()*255.0f, closeObj->Color()*255.0f);
                }
            }else{
                image[k] = glm::vec3(0.5, 0.0, 1.0)*255.0f;
            }
            k++;
        }
    }
    // write part4 shadows
    writeP6PPM((unsigned int)dimx, (unsigned int)dimy, image, "part4_shadows");

    objs.push_back(new Plane(glm::vec3(0.75, 0.75, 0.75), glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, -1.0, 0.0), true));
    objs.push_back(new Plane(glm::vec3(0.75, 0.75, 0.75), glm::vec3(-1.0, 0.0, 0.0), glm::vec3(2.0, 0.0, 0.0), false));
    objs.push_back(new Plane(glm::vec3(0.75, 0.75, 0.75), glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 0.0, -10.0), false));
    objs.push_back(new Plane(glm::vec3(0.75, 0.75, 0.75), glm::vec3(1.0, 0.0, 0.0), glm::vec3(-3.0, 0.0, 0.0), false));
    objs.push_back(new Plane(glm::vec3(0.75, 0.75, 0.75), glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 2.5, 0.0), false));
    objs.push_back(new Plane(glm::vec3(0.75, 0.75, 0.75), glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 0.0, 2.0), false));

    // part 5 planes
    k= 0;
    for (int j = dimy-1; j >= 0; --j) {
        for (int i = 0; i < dimx; ++i) {
            float minDistance = FLT_MAX;
            bool setFlag = false;
            Object* closeObj = nullptr;

            glm::vec3 intersectPos;
            glm::vec3 normal;
            glm::vec3 closeIntersectPos;
            glm::vec3 closeNormal;

            for(Object* obj : objs){
                float dist_ = obj->intersect(ray[k].origin(), ray[k].direction(), intersectPos, normal);

                if(dist_ != -1.0f){
                    if(!setFlag || minDistance > dist_){
                        setFlag = true;
                        minDistance = dist_;
                        closeObj = obj;
                        closeIntersectPos = intersectPos;
                        closeNormal = normal;
                    }
                }
            }
            if(setFlag){
                if(isIntersected(objs, closeObj, closeIntersectPos, light)){
                    image[k] = phongShadows(closeObj, closeObj->Color()*255.0f);
                }else{
                    image[k] = phongShading(closeObj, closeIntersectPos, closeNormal, ray[k], light, closeObj->Color()*255.0f, closeObj->Color()*255.0f);
                }
            }else{
                image[k] = glm::vec3(0.5, 0.0, 1.0)*255.0f;
            }
            k++;
        }
    }
    // write part 5 planes
    writeP6PPM((unsigned int)dimx, (unsigned int)dimy, image, "part5_planes");

    // part 6 reflections
    k = 0;
    for (int j = dimy-1; j >= 0; --j) {
        for (int i = 0; i < dimx; ++i) {
            float minDistance = FLT_MAX;
            bool setFlag = false;
            Object* closeObj = nullptr;

            glm::vec3 intersectPos;
            glm::vec3 normal;
            glm::vec3 closeIntersectPos;
            glm::vec3 closeNormal;

            for(Object* obj : objs){
                float dist_ = obj->intersect(ray[k].origin(), ray[k].direction(), intersectPos, normal);

//                if(dist_ != -1.0){
                if(dist_ != -1.0f){
                    if(!setFlag || minDistance > dist_){
                        setFlag = true;
                        minDistance = dist_;
                        closeObj = obj;
                        closeIntersectPos = intersectPos;
                        closeNormal = normal;
                    }
                }
            }
            if(setFlag){
                if(isIntersected(objs, closeObj, closeIntersectPos, light)){
                    if(closeObj->Reflect()){
                        glm::vec3 reflectRes = reflect(objs, closeObj, closeIntersectPos, closeNormal, ray[k], light, 10);
                        image[k] = phongShadows(closeObj, reflectRes);
                    }else{
                        image[k] = phongShadows(closeObj, closeObj->Color()*255.0f);
                    }
                }else{
                    if(closeObj->Reflect()){
//                        image[k] = reflect(objs, closeObj, closeIntersectPos, closeNormal, ray[k], light, 10);

                        glm::vec3 reflectRes = reflect(objs, closeObj, closeIntersectPos, closeNormal, ray[k], light, 10);
                        image[k] = phongShading(closeObj, closeIntersectPos, closeNormal, ray[k], light, reflectRes, reflectRes);
                    }else{
                        image[k] = phongShading(closeObj, closeIntersectPos, closeNormal, ray[k], light, closeObj->Color()*255.0f, closeObj->Color()*255.0f);
                    }
                }
            }else{
                image[k] = glm::vec3(0.5, 0.0, 1.0)*255.0f;
            }

            k++;
        }
    }


    // stop time
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Total execution time in milliseconds: " << duration.count() << std::endl;

    // write image data to PPM file
    return writeP6PPM((unsigned int)dimx, (unsigned int)dimy, image, "part6_reflections");
}
