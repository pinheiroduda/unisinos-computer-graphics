#ifndef BEZIER_H
#define BEZIER_H

#include <vector>
#include <glm/glm.hpp>

// Função que gera uma curva de Bézier cúbica segmentada a partir dos pontos de controle
std::vector<glm::vec3> generateCubicBezierCurve(const std::vector<glm::vec3>& controlPoints, int pointsPerSegment);

#endif
