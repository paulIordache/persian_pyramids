//
// Created by Paul on 1/6/2025.
//

#ifndef LAB_11_SURVEILLANCECAMERA_H
#define LAB_11_SURVEILLANCECAMERA_H

#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"
class SurveillanceCamera {
public:
    glm::vec3 position;
    glm::vec3 up;
    glm::vec3 direction;
    glm::mat4 getViewMatrix() {
        return glm::lookAt(position, position + direction, up);
    }
};

#endif //LAB_11_SURVEILLANCECAMERA_H
