#include "Camera.hpp"

namespace gps {

    Camera::Camera(glm::vec3 position, glm::vec3 front, glm::vec3 up)
            : position(position), front(front), worldUp(up), movementSpeed(15.5f) {
        updateCameraVectors();
    }

    void Camera::move(CameraMovement direction, float deltaTime) {
        float velocity = movementSpeed * deltaTime;
        if (direction == MOVE_FORWARD) {
            position += front * velocity;
        }
        if (direction == MOVE_BACKWARD) {
            position -= front * velocity;
        }
        if (direction == MOVE_LEFT) {
            position -= right * velocity;
        }
        if (direction == MOVE_RIGHT) {
            position += right * velocity;
        }
    }

    void Camera::rotate(float pitch, float yaw) {

        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(front);

        updateCameraVectors();
    }

    void Camera::updateCameraVectors() {
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }

    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(position, position + front, up);
    }

    glm::vec3 Camera::getPosition() {
        return position;
    }

    void Camera::setPosition(glm::vec3 position) {
        this->position = position;
    }


} // namespace gps
