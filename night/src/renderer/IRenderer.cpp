
#include "nightpch.h"
#include "IRenderer.h"
#include "window/IWindow.h"

namespace night
{

	//IRenderer::IRenderer(const RendererParams& params)
	//	: _window(params.window)
	//{
	//}

	s32 IRenderer::init(const RendererParams& params)
	{
		_window = params.window;
		return 1;
	}

	void IRenderer::close()
	{
	}

}