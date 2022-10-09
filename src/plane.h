#ifndef PLANE_H_
#define PLANE_H_

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include "object.h"
#include "math.h"

class Plane : public Object {
public:

    Plane(glm::vec3 col, glm::vec3 norm, glm::vec3 poi, bool reflecting = false, float ambientFactor = 0.2f,
           float specExponent = 50.0f) : Object(col, reflecting, ambientFactor, specExponent){
        planeNormal = norm;
        planePoint = poi;
    };

	// intersection function: returns the closest intersection point with the given ray (or a negative value, if none exists)
	// output parameters: location of the intersection, object normal
	// PURE VIRTUAL FUNCTION: has to be implemented in all child classes.
    float intersect(const glm::vec3& rayOrigin, const glm::vec3& rayDir, glm::vec3& intersectPos, glm::vec3& normal){
//        intersectPos = glm::vec3(0.0f, 0.0f, 0.0f);
//        normal = glm::vec3(0.0f, 0.0f, 0.0f);

        float dn = glm::dot(rayDir, planeNormal);
        if(dn == 0) return -1.0f;

        float dist_ = (glm::dot(planePoint - rayOrigin, planeNormal))/dn;
        if(dist_ < 0) return -1.0f;

        intersectPos = rayOrigin + rayDir*dist_;
        normal = planeNormal;

        return dist_;
    };

private:
    glm::vec3 planeNormal;
    glm::vec3 planePoint;


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
