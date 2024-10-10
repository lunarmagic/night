
#include "nightpch.h"
#include "Node3D.h"

namespace night
{
	Node3D::Node3D()
		: _transform(1)
	{
		depth(-1.0f);
	}

	void Node3D::translate(const vec3& translation)
	{
		_transform = glm::translate(_transform, translation);
	}

	void Node3D::scale(const vec3& scale)
	{
		_transform = glm::scale(_transform, scale);
	}

	void Node3D::rotate(const vec3& axis, real radians)
	{
		_transform = glm::rotate(_transform, radians, axis);
	}
}