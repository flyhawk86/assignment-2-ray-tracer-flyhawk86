#ifndef SPHERE_H_
#define SPHERE_H_

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include "object.h"
#include "math.h"

class Sphere : public Object {
public:
    Sphere(glm::vec3 col, float rad, glm::vec3 cent, bool reflecting = false, float ambientFactor = 0.2f,
           float specExponent = 50.0f) : Object(col, reflecting, ambientFactor, specExponent){
        radius = rad;
        center = cent;
    };

//    Sphere(glm::vec3 col, bool reflecting = false, float ambientFactor = 0.2f,
//           float specExponent = 50.0f) : Object(col, reflecting, ambientFactor, specExponent){};

	// intersection function: returns the closest intersection point with the given ray (or a negative value, if none exists)
	// output parameters: location of the intersection, object normal
	// PURE VIRTUAL FUNCTION: has to be implemented in all child classes.
    float intersect(const glm::vec3& rayOrigin, const glm::vec3& rayDir, glm::vec3& intersectPos, glm::vec3& normal){
//        intersectPos = glm::vec3(0.0f, 0.0f, 0.0f);
//        normal = glm::vec3(0.0f, 0.0f, 0.0f);

        glm::vec3 oc = rayOrigin - center;
        auto a = glm::dot(rayDir, rayDir);
        auto b = 2.0f * glm::dot(oc, rayDir);
        auto c = glm::dot(oc, oc) - radius*radius;
        auto discriminant = b*b - 4*a*c;
        if(discriminant < 0){
            return -1.0f;
        }

        auto dist_ = (-b - sqrt(discriminant))/(2*a);

        intersectPos = rayOrigin + rayDir*dist_;
        normal = glm::normalize(intersectPos - center);

        return dist_;
    };

private:
    float radius;
    glm::vec3 center;


//    const glm::vec3& Color() { return color; };
//    float AmbientFactor() { return ambient; };
//    float SpecularExponent() { return specularEx; };
//    bool Reflect() { return reflect; };
//
//private:
//    // object color
//    glm::vec3 color;
//    // basic material parameters
//    float ambient;
//    float specularEx;
//    // is this object reflecting?
//    bool reflect;

};

#endif
