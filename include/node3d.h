#ifndef NODE3D_H
#define NODE3D_H

#include <vector>
#include <cmath>

#include "glm/glm.hpp"
#include "body3d.h"

enum Quadrant {
            //x     y       z
    RTF,    //right top     front
    LTF,    //left  top     front
    RBF,    //right bottom  front
    LBF,    //left  bottom  front
    RTB,    //right top     back
    LTB,    //left  top     back
    RBB,     //right bottom  back
    LBB,    //left  bottom  back
    NONE
};

class Node3D {
public:
    glm::dvec3 center;
    double length;
    Body3D *body;

    Node3D *quads[8] = {};

    Node3D() {}
    Node3D(glm::dvec3 center, double length) {
        this->center = center;
        this->length = length;
        this->body = NULL;
    }

    void subdivide() {
        double l = length/2;
        quads[RTF] = new Node3D(glm::dvec3(center.x + l, center.y + l, center.z + l), l);
        quads[LTF] = new Node3D(glm::dvec3(center.x - l, center.y + l, center.z + l), l);
        quads[RBF] = new Node3D(glm::dvec3(center.x + l, center.y - l, center.z + l), l);
        quads[LBF] = new Node3D(glm::dvec3(center.x - l, center.y - l, center.z + l), l);
        quads[RTB] = new Node3D(glm::dvec3(center.x + l, center.y + l, center.z - l), l);
        quads[LTB] = new Node3D(glm::dvec3(center.x - l, center.y + l, center.z - l), l);
        quads[RBB] = new Node3D(glm::dvec3(center.x + l, center.y - l, center.z - l), l);
        quads[LBB] = new Node3D(glm::dvec3(center.x - l, center.y - l, center.z - l), l);
    }

    bool contains(Body3D &b) {
        return  (b.position.x > center.x - length && b.position.x < center.x + length) &&
                (b.position.y > center.y - length && b.position.y < center.y + length) &&
                (b.position.z > center.z - length && b.position.z < center.z + length);
    }

    Quadrant get_quadrant(glm::dvec3 p) {
        if(p.x >= center.x && p.y >= center.y && p.z >= center.z)
            return RTF;
        if(p.x < center.x && p.y >= center.y && p.z >= center.z)
            return LTF;
        if(p.x >= center.x && p.y < center.y && p.z >= center.z)
            return RBF;
        if(p.x < center.x && p.y < center.y && p.z >= center.z)
            return LBF;
        if(p.x >= center.x && p.y >= center.y && p.z < center.z)
            return RTB;
        if(p.x < center.x && p.y >= center.y && p.z < center.z)
            return LTB;
        if(p.x >= center.x && p.y < center.y && p.z < center.z)
            return RBB;
        if(p.x < center.x && p.y < center.y && p.z < center.z)
            return LBB;

        return NONE;
    }

    Node3D* create_node(Quadrant q) {
        if(q == RTF)
            return new Node3D(glm::dvec3(center.x + length/2, center.y + length/2, center.z + length/2), length/2);
        if(q == LTF)
            return new Node3D(glm::dvec3(center.x - length/2, center.y + length/2, center.z + length/2), length/2);
        if(q == RBF)
            return new Node3D(glm::dvec3(center.x + length/2, center.y - length/2, center.z + length/2), length/2);
        if(q == LBF)
            return new Node3D(glm::dvec3(center.x - length/2, center.y - length/2, center.z + length/2), length/2);
        if(q == RTB)
            return new Node3D(glm::dvec3(center.x + length/2, center.y + length/2, center.z - length/2), length/2);
        if(q == LTB)
            return new Node3D(glm::dvec3(center.x - length/2, center.y + length/2, center.z - length/2), length/2);
        if(q == RBB)
            return new Node3D(glm::dvec3(center.x + length/2, center.y - length/2, center.z - length/2), length/2);
        if(q == LBB)
            return new Node3D(glm::dvec3(center.x - length/2, center.y - length/2, center.z - length/2), length/2);

        return NULL;
    }

    bool isExternal() {
        return (quads[0] == NULL &&
                quads[1] == NULL &&
                quads[2] == NULL &&
                quads[3] == NULL &&
                quads[4] == NULL &&
                quads[5] == NULL &&
                quads[6] == NULL &&
                quads[7] == NULL);
    }

    void insert(Body3D &b) {
        if(!contains(b))
            return;

        if(body == NULL) {
            body = &b;
            return;
        }

        //get current node body quadrant
        Quadrant body_q = get_quadrant(body->position);
        //get new body quadrant
        Quadrant new_q = get_quadrant(b.position);

        if(quads[body_q] == NULL) {
            //create new node for current node body
            quads[body_q] = create_node(body_q);
            quads[body_q]->insert(*body);
        }
        //quads[body_q]->insert(*body);
        
        if(quads[new_q] == NULL) {
            //create new node for new body
            quads[new_q] = create_node(new_q);
        }
        quads[new_q]->insert(b);

        body = Body3D::add(*body, b);
    }

    void update_force(Body3D &b) {
        if(body == NULL || b == *body)
            return;

        if(isExternal()) {
            // b.add_force(*body);

            if(body->collision(b)) {
                Body3D *tmp = Body3D::add(*body, b);
                b = *tmp;
                body = NULL;
            }
            else {
                b.add_force(*body);
            }
        }
        else {
            double s = length;
            // double d = body->distance_to2(b);
            // if(length*length < THETA*THETA*d) {
            //     b.add_force(*body);
            // }
            double d = body->distance_to(b);
            if((s/d) < THETA) {
                b.add_force(*body);
            }
            else{
                for(int i = 0; i < 8; i++) {
                    if(quads[i] != NULL)
                        quads[i]->update_force(b);
                }
            }

        }
    }

    
private:
    double THETA = 0.5;
};

#endif /* NODE3D_H */