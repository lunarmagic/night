
#include "nightpch.h"
#include "common.h"
#include "Sandbox.h"
#include "Renderer3D/Renderer3D.h"
#include "gamemode/FreeformBoxesMode.h"
#include "gamemode/FormIntersectionsMode.h"
#include "gamemode/FormIntersectionsParallelMode.h"
#include "log/log.h"

#include <format>
#include <core.h>
#include <iostream>
#include <string>
#include <string_view>

#undef DELETE

namespace night
{
	pair<IWindow*, Application::WindowParams> Sandbox::create_window()
	{
		return { new WindowSDL(), {.title = "my funy window", .width = 1000, .height = 1000, .fps = 144.0f } };
	}

	INode* Sandbox::create_root()
	{
		//return new FreeformBoxesMode();

		//return new FormIntersectionsMode(FormIntersectionsModeParams
		//	{
		//	.which_forms = EForm::Box
		//	}
		//); // TODO: add main menu node

		return new FormIntersectionsParallelMode(FormIntersectionsParallelModeParams
			{
			.which_forms = EFormType::Box
			}
		); // TODO: add main menu node
	}

	void Sandbox::on_load_resources()
	{
		auto f10x = window().create_surface("Font10x", {.path = "resources/textures/Font10x.png" });
		window().create_texture("Font10x", { .surface = f10x });

		auto f16x = window().create_surface("Font16x", { .path = "resources/textures/Font16x.png" });
		window().create_texture("Font16x", { .surface = f16x });
	}

}