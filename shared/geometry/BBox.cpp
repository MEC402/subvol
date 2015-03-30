#include "BBox.h"

#include <array>

namespace bd {
    namespace geometry {
        const std::array<float, 32> BBox::vertices{
            -0.5, -0.5, -0.5, 1.0,
            0.5, -0.5, -0.5, 1.0,
            0.5, 0.5, -0.5, 1.0,
            -0.5, 0.5, -0.5, 1.0,
            -0.5, -0.5, 0.5, 1.0,
            0.5, -0.5, 0.5, 1.0,
            0.5, 0.5, 0.5, 1.0,
            -0.5, 0.5, 0.5, 1.0,
        };

        const std::array<unsigned short, 16> BBox::elements{
            0, 1, 2, 3,
            4, 5, 6, 7,
            0, 4, 1, 5, 2, 6, 3, 7
        };

        BBox::BBox()
            : min_x(0.0f),
            max_x(0.0f),
            min_y(0.0f),
            max_y(0.0f),
            min_z(0.0f),
            max_z(0.0f)
        {
            glm::vec3 size = glm::vec3(max_x - min_x, max_y - min_y, max_z - min_z);
            glm::vec3 center = glm::vec3((min_x + max_x) / 2.0f, (min_y + max_y) / 2.0f, (min_z + max_z) / 2.0f);
            glm::mat4 transform = glm::translate(glm::mat4(1.0f), center) * glm::scale(glm::mat4(1.0f), size);
            m_transform = transform;
        }

        BBox::BBox(const std::array<float, 6> &minmax)
            : min_x(minmax[0]),
            max_x(minmax[1]),
            min_y(minmax[2]),
            max_y(minmax[3]),
            min_z(minmax[4]),
            max_z(minmax[5])
        {
            glm::vec3 size = glm::vec3(max_x - min_x, max_y - min_y, max_z - min_z);
            glm::vec3 center = glm::vec3((min_x + max_x) / 2.0f, (min_y + max_y) / 2.0f, (min_z + max_z) / 2.0f);
            glm::mat4 transform = glm::translate(glm::mat4(1.0f), center) * glm::scale(glm::mat4(1.0f), size);
            m_transform = transform;
        }

        BBox::~BBox() {
        }
    }
} // namespace