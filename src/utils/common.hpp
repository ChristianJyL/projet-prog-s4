#pragma once

#include <GLFW/glfw3.h>
#include "glm.hpp"

namespace glBurnout {

struct ShapeVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

}
