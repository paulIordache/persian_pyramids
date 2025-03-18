#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace gps {

    enum CameraMovement {
        MOVE_FORWARD,
        MOVE_BACKWARD,
        MOVE_LEFT,
        MOVE_RIGHT
    };

    class Camera {
    public:
        Camera(glm::vec3 position, glm::vec3 front, glm::vec3 up);
        void move(CameraMovement direction, float deltaTime);
        void rotate(float pitch, float yaw);
        glm::mat4 getViewMatrix();
        glm::vec3 getPosition();
        void setPosition(glm::vec3 pos);

    private:
        glm::vec3 position;
        glm::vec3 front;
        glm::vec3 up{};
        glm::vec3 right{};
        glm::vec3 worldUp;
        float movementSpeed;

        void updateCameraVectors();
    };

} // namespace gps

#endif // CAMERA_HPP
