
#include "nightpch.h"
#include "IWindow.h"
#include "renderer/IRenderer.h"

namespace night
{

	s32 IWindow::init(const Application::WindowParams& params)
	{
		_width = params.width;
		_height = params.height;
		_eventCallback = params.eventCallback;
		_fps = params.fps;
		_timeElapsed = 0.0f;
		_deltaTime = _fps / 1000.0f;

		auto [renderer, rparams] = create_renderer();
		_renderer = renderer;
		return _renderer->init(rparams);
	}

	void IWindow::close()
	{
		if (_renderer)
		{
			_renderer->close();
			delete _renderer;
			_renderer = nullptr;
		}
		
		TRACE("Destroying Window");
	}

	ref<ISurface> IWindow::find_surface(const string& id) const
	{
		auto i = _surfaces.find(id);

		if (i != _surfaces.end())
		{
			return (*i).second;
		}

		return nullptr;
	}

	void IWindow::destroy_surface(const string& id)
	{
		auto i = _surfaces.find(id);
		if (i != _surfaces.end())
		{
			TRACE("Window destroyed surface, id: ", id);
			_surfaces.erase(i);
		}
	}

	ref<ITexture> IWindow::find_texture(const string& id) const
	{
		auto i = _textures.find(id);

		if (i != _textures.end())
		{
			return (*i).second;
		}

		return nullptr;
	}

	void IWindow::destroy_texture(const string& id)
	{
		auto i = _textures.find(id);
		if (i != _textures.end())
		{
			TRACE("Window destroyed texture, id: ", id);
			_textures.erase(i);
		}
	}

	real IWindow::aspect_ratio() const
	{
		real a = (real)_height / (real)_width;
		real b = (real)_width / (real)_height;
		return a < b ? a : b;
	}

	u8 IWindow::on_resize(WindowResizeEvent& event)
	{
		_width = event.width();
		_height = event.height();

		return true;
	}

}