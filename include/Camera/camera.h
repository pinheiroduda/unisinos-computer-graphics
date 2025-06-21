#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Definições possíveis para movimento (opcional)
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera {
    public:
        // Atributos principais
        glm::vec3 Position;
        glm::vec3 Front;
        glm::vec3 Up;
        glm::vec3 Right;
        glm::vec3 WorldUp;

        // Ângulos de Euler
        float Yaw;
        float Pitch;

        // Zoom
        float Fov;

        // Construtor
        Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);

        // Métodos principais
        glm::mat4 GetViewMatrix();

        void ProcessKeyboard(Camera_Movement direction, float deltaTime);
        void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
        void ProcessMouseScroll(float yoffset);

    private:
        void updateCameraVectors();
};
