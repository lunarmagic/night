#pragma once

#include "IGameMode.h"

namespace night
{

	struct CameraGameMode : public IGameMode
	{
		CameraGameMode();

	protected:

		
		virtual void on_update(real delta) override;

		vec2 _mouse{ 0 };
		vec2 _mouse_prev{ 0 };

		vec3 _cameraPosition{0};
		mat4 _camera = mat4(1.0f); // TODO: add this stuff to a base class.
		u8 _is_rotating_camera = false;
		real _rotation_sensitivity{ 1.0f };
		mat4 _rotation = mat4(1.0f);
		real _zoom = 1.5f;
		real _zoom_sensitivity{ 1.0f };
	};

}