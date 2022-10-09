//
// Created by Hao Wu on 10/6/22.
//

#ifndef RAYTRACER_RAY_H
#define RAYTRACER_RAY_H

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

class Ray {
public:
    Ray() {}
    Ray(const glm::vec3& origin, const glm::vec3& direction)
            : orig(origin), dir(direction)
    {}

    glm::vec3 origin() const  { return orig; }
    glm::vec3 direction() const { return dir; }

    glm::vec3 at(float t) const {
        return orig + t*dir;
    }

public:
    glm::vec3 orig;
    glm::vec3 dir;
};


#endif //RAYTRACER_RAY_H
