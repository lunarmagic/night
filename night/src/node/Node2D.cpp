
#include "nightpch.h"
#include "Node2D.h"
#include "math/math.h"
#include "log/log.h"

#define GLM_ENABLE_EXPERIMENTAL // TODO: remove this
#include "glm/gtx/rotate_vector.hpp"

namespace night
{
    const vec2& night::Node2D::position() const
    {
        return _position;
    }

    const vec2& night::Node2D::scale() const
    {
        return _scale;
    }

    const vec2& night::Node2D::rotation() const
    {
        return _rotation;
    }

    void night::Node2D::position(const vec2& position)
    {
        _position = position;
        update();
    }

    void night::Node2D::scale(const vec2& scale)
    {
        _scale = scale;
        update();
    }

    void night::Node2D::rotation(const vec2& rotation)
    {
        _rotation = rotation;
        update();
    }

    void night::Node2D::translate(const vec2& translation)
    {
        _position += translation;
        update();
    }

    void Node2D::rotate(real theta)
    {
        _rotation = glm::rotate(_rotation, theta);
        update();
    }

    void Node2D::update()
    {
        mat4 x(1);
        x = glm::translate(x, vec3(_position, 0.0f));
        x = glm::rotate(x, night::angle(_rotation), vec3(0, 0, -1));
        x = glm::scale(x, vec3(_scale, 1.0f));
        
        transform(x);
    }

}
