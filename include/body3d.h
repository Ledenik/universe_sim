#ifndef BODY_H
#define BODY_H

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <vector>
#include <cmath>
#include "glm/glm.hpp"

class Body3D {
public:
    glm::dvec3 position;
    glm::dvec3 velocity;
    glm::dvec3 force;
    double mass;


    Body3D() {}
    Body3D(glm::dvec3 position, glm::dvec3 velocity, glm::dvec3 force, double mass) {
        this->position = position;
        this->velocity = velocity;
        this->force = force;
        this->mass = mass;
    }

    static Body3D* add(Body3D &a, Body3D &b) {
        //return new Body3D(a.position + b.position, a.velocity + b.velocity, a.force + b.force, a.mass + b.mass);
        double m = a.mass + b.mass;
        double x = (a.position.x * a.mass + b.position.x * b.mass) / m;
        double y = (a.position.y * a.mass + b.position.y * b.mass) / m;
        double z = (a.position.z * a.mass + b.position.z * b.mass) / m;

        return new Body3D(glm::dvec3(x, y, z), glm::dvec3(0.0f), glm::dvec3(0.0f), m);
    }

    double distance_to(Body3D &b) {
        glm::dvec3 delta = position - b.position;
        return std::sqrt(delta.x*delta.x + delta.y*delta.y + delta.z*delta.z);
    }

    double distance_to2(Body3D &b) {
        glm::dvec3 delta = position - b.position;
        return (delta.x*delta.x + delta.y*delta.y + delta.z*delta.z);
    }

    void update(double dt) {
        velocity += dt * force / mass;
        position += dt * velocity;
    }

    void reset_force() {
        force = glm::dvec3(0.0f);
    }

    void add_force(Body3D &b) {
        double G = 6.67e-11;
        double eps = 3e4;
        glm::dvec3 delta = b.position - position;
        double distance = std::sqrt(delta.x*delta.x + delta.y*delta.y + delta.z*delta.z);
        double F = (G * mass * b.mass) / (distance*distance + eps*eps);
        force += F * delta / distance;
    }

    bool operator == (const Body3D &b) const {
        return  (position == b.position) &&
                (velocity == b.velocity) &&
                (force == b.force) &&
                (mass == b.mass);
    }

    bool collision(Body3D &b) {
        double eps = 1e-11;
        // if(position == b.position) {
        //     return true;
        // }
        return (abs(position.x - b.position.x) < eps &&
                abs(position.y - b.position.y) < eps &&
                abs(position.z - b.position.z) < eps);
    }
};

#endif /* BODY_H */