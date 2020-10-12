#ifndef UNIVERSE_H
#define UNIVERSE_H

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "../include/glm/gtc/matrix_transform.hpp"

#include "body3d.h"
#include "node3d.h"
#include "shader.h"

#include <vector>
#include <cmath>
#include <ctime>

class Universe {
private:
    unsigned int VAO, VBO;
    
public:
    int num_bodies;
    double size;
    Node3D bh_tree;
    std::vector<Body3D> bodies;

    Universe(int num_bodies, double size) {
        this->num_bodies = num_bodies;
        this->size = size * 9.4e15;
        //this->bh_tree = Node3D(glm::dvec3(0.0f), size);
    }

    void setup() {
        float vertex[] = {0.5f, 0.5f, 0.5f};

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(0);
    }

    void draw(Shader shader) {
        glBindVertexArray(VAO);
        glPointSize(2);
        for(int i = 0; i < bodies.size(); i++) {
            glm::mat4 model = glm::mat4(1.0f);
            glm::dvec3 pos = bodies[i].position / 9.4e15;
            model = glm::translate(model, (glm::vec3)pos);
            shader.setMat4("model", model);

            double mass_n = (bodies[i].mass - 0.08*2e30) / (150*2e30 - 0.08*2e30);
            if(mass_n > 1.0f) {
                glPointSize(4);
            }
            else
                glPointSize(2);

            shader.setFloat("mass", mass_n);

            shader.setVec3("velocity", glm::normalize(bodies[i].velocity));

            glDrawArrays(GL_POINTS, 0, 1);
        }

        glBindVertexArray(0);
    }

    void generate(glm::dvec3 dimensions) {
        double lightyear = 9.4e15;
        double M0 = 2e30;

        srand((unsigned)time(0));

        for(int i = 0; i < num_bodies; i++) {
            glm::dvec3 position = random_point_elipsoid(dimensions * lightyear);
            glm::dvec3 velocity;

            //orbital velocity
            velocity = glm::normalize(glm::cross(glm::dvec3(0.0f, 0.0f, 1.0f), position));
            velocity *= sqrt((6.67e-11 * 1e6*M0) / glm::length(position));

            //random velocity
            // double vx = -1.0f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(1.0f-(-1.0f))));
            // double vy = -1.0f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(1.0f-(-1.0f))));
            // double vz = -1.0f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(1.0f-(-1.0f))));
            // velocity = glm::dvec3(
            //     vx, vy, vz
            // ) * sqrt((6.67e-11 * 1e6*M0) / (glm::length(position)));

            //zero velocity
            //velocity = glm::dvec3(0.0f);

            //random mass
            double m_min = 0.08f * M0;
            double m_max = 150 * M0;
            double mass = m_min + static_cast <float> (rand()) / ( static_cast <float> (RAND_MAX/(m_max - m_min)));

            bodies.emplace_back(Body3D(position, velocity, glm::dvec3(0.0f), mass));
        }

        //black holes
        // for(int i = 0; i < 5; i++) {
        //     glm::dvec3 position = random_point_elipsoid(dimensions * lightyear);

        //     bodies.emplace_back(Body3D(position, glm::dvec3(0.0f), glm::dvec3(0.0f), 1e6*M0));
        // }

        bodies.emplace_back(Body3D(glm::dvec3(0.0f), glm::dvec3(0.0f), glm::dvec3(0.0f), 1e6*M0));
    }
    glm::dvec3 random_point_elipsoid(glm::dvec3 dimensions) {
        double u = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        double v = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

        double theta = u * 2.0f * 3.14f;
        double phi = acos(2.0f * v - 1.0f);

        double sinTheta = sin(theta);
        double cosTheta = cos(theta);

        double sinPhi = sin(phi);
        double cosPhi = cos(phi);

        double sx = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        double sy = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        double sz = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

        return glm::dvec3(
            sx * dimensions.x * sinPhi * cosTheta,
            sy * dimensions.y * sinPhi * sinTheta,
            sz * dimensions.z * cosPhi
        );
    }

    void simulate(double dt) {
        bh_tree = Node3D(glm::dvec3(0.0f), size);

        for(int i = 0; i < bodies.size(); i++) {
            bh_tree.insert(bodies[i]);
        }

        for(int i = 0; i < bodies.size(); i++) {
            bodies[i].reset_force();
            bh_tree.update_force(bodies[i]);
            bodies[i].update(dt*9.4e13);
        }
    }
};

#endif /* UNIVERSE_H */