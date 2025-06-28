#include "bezier.h"

std::vector<glm::vec3> generateCubicBezierCurve(const std::vector<glm::vec3>& controlPoints, int pointsPerSegment) {
    std::vector<glm::vec3> curvePoints;

    // Matriz de Bézier padrão
    glm::mat4 bezierMatrix(
        -1.0f,  3.0f, -3.0f, 1.0f,
         3.0f, -6.0f,  3.0f, 0.0f,
        -3.0f,  3.0f,  0.0f, 0.0f,
         1.0f,  0.0f,  0.0f, 0.0f
    );

    // Percorre os pontos de controle em grupos de 4
    for (size_t i = 0; i + 3 < controlPoints.size(); i += 3) {
        glm::vec3 p0 = controlPoints[i];
        glm::vec3 p1 = controlPoints[i + 1];
        glm::vec3 p2 = controlPoints[i + 2];
        glm::vec3 p3 = controlPoints[i + 3];

        // Matriz com os pontos de controle
        glm::mat4x3 G(
            p0.x, p0.y, p0.z,
            p1.x, p1.y, p1.z,
            p2.x, p2.y, p2.z,
            p3.x, p3.y, p3.z
        );

        // Gera pontos intermediários ao longo do segmento
        for (int j = 0; j <= pointsPerSegment; ++j) {
            float t = static_cast<float>(j) / pointsPerSegment;
            glm::vec4 T(t * t * t, t * t, t, 1.0f);

            glm::vec3 point = G * (bezierMatrix * T);
            curvePoints.push_back(point);
        }
    }

    return curvePoints;
}
