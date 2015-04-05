#ifndef bbox_h__
#define bbox_h__

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

namespace bd {


class Box {
public:
    static const std::array<float, 32> vertices;
    static const std::array<unsigned short, 16> elements;
    static const unsigned int vert_element_size = 4;

public:
    Box();

    /** \brief create a bbox with the given mins/maxes.
             *
             *  \param minmax minX, maxX, minY, maxY, minZ, maxZ
             */
    Box(const std::array<float, 6> &minmax);
    ~Box();

public:
    glm::mat4 transform() const
    {
        return m_transform;
    }

private:
    float min_x, max_x,
          min_y, max_y,
          min_z, max_z;

    glm::mat4 m_transform;
};

} // namspace bd

#endif