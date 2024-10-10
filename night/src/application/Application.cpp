
#include "nightpch.h"
#include <SDL.h>
#include "log/log.h"
#include "Application.h"
#include "core.h"
#include "window/IWindow.h"
#include "window/backends/sdl/WindowSDL.h"
#include "event/MouseEvent.h"
#include "event/ApplicationEvent.h"
#include "event/KeyEvent.h"
#include "input/InputKey.h"
#include "node/INode.h"
//#include <sstream>

namespace night
{

	#define BIND_EVENT(x) bind(&Application::x, this, placeholder_1)

	Application* Application::_instance = nullptr;

	Application::Application()
	{
		ASSERT(_instance == nullptr); // Application is a singleton, don't create more than 1
		_instance = this;
	}

	Application::~Application()
	{
	}

	void Application::init()
	{
		random_set_seed();

		auto [window, params] = create_window();
		_window = window;
		_window->init(params);

		load_resources();

		{ // TODO: impl this
			INode::__name = "Root";
			INode::__parent = nullptr;

			_root = sref<INode>(create_root());

			INode::__uid++;
			INode::__name = "";
		}

		on_initialized();
	}

	void Application::load_resources()
	{
		on_load_resources();
	}

	s32 Application::run()
	{
		init();

		while (!_isPendingTermination)
		{
			//on_update(0.0f);

			if (_root)
			{
				_root->update(_window->delta_time());
				_root->render_tree();
			}

			_window->update();
			//_window->render();
		}

		close();

		return 0;
	}

	void Application::close()
	{
		on_close();

		_root.reset();

		delete _window;
	}

	Application& Application::get()
	{
		return *_instance;
	}

	pair<IWindow*, Application::WindowParams> Application::create_window()
	{
		// TODO: add default macro
		//return new WindowSDL({ .title = "Window", .width = 640, .height = 480, .eventCallback = [](Event& event) { Application::get().on_event(event); } });
		return { new WindowSDL, { .title = "Window", .width = 640, .height = 480, .eventCallback = [](Event& event) { Application::get().on_event(event); } } };
	}

	INode* Application::create_root()
	{
		return new INode();
	}

	void Application::random_set_seed()
	{
		::night::random_set_seed((u32)time(nullptr));
	}

	//void Application::on_update(real delta)
	//{
	//	return;
	//}

	void Application::on_event(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.dispatch<KeyPressedEvent>(BIND_EVENT(on_key_pressed));
		dispatcher.dispatch<KeyReleasedEvent>(BIND_EVENT(on_key_released));
		dispatcher.dispatch<MouseButtonPressedEvent> (BIND_EVENT(on_mouse_button_pressed));
		dispatcher.dispatch<MouseButtonReleasedEvent> (BIND_EVENT(on_mouse_button_released));
		dispatcher.dispatch<MouseWheelEvent>(BIND_EVENT(on_mouse_wheel));
		dispatcher.dispatch<MouseMotionEvent>(BIND_EVENT(on_mouse_motion));
		dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT(on_window_close));

		//LOG(event.to_string());
	}

	void Application::callback_bound_inputs(const InputKey& key)
	{
		for (auto [i, end] = _inputBindings.equal_range(key); i != end; i++)
		{
			(*i).second();
		}
	}

	template<typename T>
	inline u8 Application::callback_bound_events(T& event)
	{
		auto r = _eventBindings.equal_range(T::get_static_type());

		for (auto i = r.first; i != r.second; i++)
		{
			(*i).second(event);
		}

		return true;
	}

	u8 Application::on_key_pressed(KeyPressedEvent& event)
	{
		callback_bound_inputs({ .key = (EKey)event.keycode(), .type = event.isRepeat() ? EInputType::REPEAT : EInputType::PRESSED });

		return callback_bound_events(event);
	}

	u8 Application::on_key_released(KeyReleasedEvent& event)
	{
		callback_bound_inputs({ .key = (EKey)event.keycode(), .type = EInputType::RELEASED });

		return callback_bound_events(event);
	}

	u8 Application::on_mouse_button_pressed(MouseButtonPressedEvent& event)
	{
		callback_bound_inputs({ .mouse = (EMouse)event.button(), .type = EInputType::PRESSED });

		return callback_bound_events(event);
	}

	u8 Application::on_mouse_button_released(MouseButtonReleasedEvent& event)
	{
		callback_bound_inputs({ .mouse = (EMouse)event.button(), .type = EInputType::RELEASED });

		return callback_bound_events(event);
	}

	u8 Application::on_mouse_wheel(MouseWheelEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 Application::on_mouse_motion(MouseMotionEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 Application::on_window_close(WindowCloseEvent& event)
	{
		u8 result = callback_bound_events(event);

		terminate();

		return result;
	}

	umultimap<InputKey, function<void()>>::iterator Application::bind_input(EKey key, EInputType type, function<void()> fn)
	{
		InputKey input_key = { .key = key, .type = type };
		return _inputBindings.insert({ input_key, fn });
	}

	umultimap<InputKey, function<void()>>::iterator Application::bind_input(EMouse mouse, EInputType type, function<void()> fn)
	{
		InputKey input_key = { .mouse = mouse, .type = type };
		return _inputBindings.insert({ input_key, fn });
	}

	umultimap<InputKey, function<void()>>::iterator Application::bind_input(EButton button, EInputType type, function<void()> fn)
	{
		InputKey input_key = { .button = button, .type = type };
		return _inputBindings.insert({ input_key, fn });
	}

	void Application::unbind_input(umultimap<InputKey, function<void()>>::iterator input)
	{
		_inputBindings.erase(input);
	}

	void Application::unbind_all_inputs()
	{
		_inputBindings.clear();
	}

	void Application::unbind_event(umultimap<EEventType, function<void(Event&)>>::iterator event)
	{
		_eventBindings.erase(event);
	}

	void Application::unbind_all_events()
	{
		_eventBindings.clear();
	}

}