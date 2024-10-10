
#include "nightpch.h"
#include "CameraGameMode.h"
#include "application/Application.h"
#include "window/IWindow.h"
#include "utility.h"
#include "Renderer3D/Renderer3D.h"
#include "math/math.h"

namespace night
{
	CameraGameMode::CameraGameMode()
	{
		BIND_INPUT(EMouse::RIGHT, EInputType::PRESSED, [&]() { _is_rotating_camera = true; });
		BIND_INPUT(EMouse::RIGHT, EInputType::RELEASED, [&]() { _is_rotating_camera = false; });

		BIND_INPUT(EKey::PAGEUP, EInputType::PRESSED, [&]() { _zoom += _zoom_sensitivity; });
		BIND_INPUT(EKey::PAGEDOWN, EInputType::PRESSED, [&]() { _zoom -= _zoom_sensitivity; });

		on_update(0.0f);
	}

	void CameraGameMode::on_update(real delta)
	{
		// TODO: make camera struct.
		_mouse_prev = _mouse;
		_mouse = utility::window().mouse();
		vec2 delta_mouse = _mouse - _mouse_prev;

		_cameraPosition = vec3(0.0f, 0.0f, -7.0f + _zoom);
		vec3 up = vec3(0.0f, 1.0f, 0.0f);

		if (_is_rotating_camera)
		{
			vec3 axis(0.0f, 1.0f, 0.0f);
		
			_rotation = rotate(_rotation, R_PI * delta_mouse.x * _rotation_sensitivity, axis);
		}
		
		_cameraPosition = vec4(_cameraPosition.x, _cameraPosition.y, _cameraPosition.z, 1.0f) * _rotation;
		
		if (_is_rotating_camera)
		{
			vec3 right = normalize(cross(_cameraPosition, up));
			_rotation = rotate(_rotation, R_PI * -delta_mouse.y * _rotation_sensitivity, right);
		}

		vec3 cameraTarget = vec3(0.0f, 0.0f, 0.0f);
		_camera = lookAt(_cameraPosition, cameraTarget, up);

		Renderer3D::camera(_camera);
	}

}