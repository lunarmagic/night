#pragma once

//#include "utility.h"
#include "core.h"
#include "input/InputKey.h"
#include "event/Event.h"

namespace night
{

	struct IWindow;
	struct KeyPressedEvent;
	struct KeyReleasedEvent;
	struct MouseButtonPressedEvent;
	struct MouseButtonReleasedEvent;
	struct MouseWheelEvent;
	struct MouseMotionEvent;
	struct WindowCloseEvent;
	struct INode;

	struct NIGHT_API Application
	{
		Application();
		~Application();

		s32 run();

		static Application& get();

		IWindow& window() { return *_window; }

		void terminate() { _isPendingTermination = true; }

		struct WindowParams
		{
			string title{ "Window" };
			s32 width{ 640 };
			s32 height{ 480 };
			function<void(Event&)> eventCallback{ [](Event& event) { Application::get().on_event(event); } };
			real fps{ 60 };
		};

		ref<INode> const root() { return _root; }

		// TODO: fix this
		// returns an iterator that is used to unbind the input later.
		umultimap<InputKey, function<void()>>::iterator bind_input(EKey key, EInputType type, function<void()> fn);
		umultimap<InputKey, function<void()>>::iterator bind_input(EMouse mouse, EInputType type, function<void()> fn);
		umultimap<InputKey, function<void()>>::iterator bind_input(EButton button, EInputType type, function<void()> fn);

		// pass in the iterator returned from bind_input.
		void unbind_input(umultimap<InputKey, function<void()>>::iterator input);
		void unbind_all_inputs();

		umultimap<EEventType, function<void(Event&)>>::iterator bind_event(auto fn)
		{
			return bind_event_impl(function(fn));
		}

		void unbind_event(umultimap<EEventType, function<void(Event&)>>::iterator event);
		void unbind_all_events();

	protected:

		virtual pair<IWindow*, WindowParams> create_window();
		virtual INode* create_root();

		virtual void random_set_seed();

		virtual void on_load_resources() { return; }
		virtual void on_initialized() { return; }
		virtual void on_close() { return; }
	
	private:

		void load_resources();
		void init();
		void close();

		void on_event(Event& event);
		u8 on_key_pressed(KeyPressedEvent& event);
		u8 on_key_released(KeyReleasedEvent& event);
		u8 on_mouse_button_pressed(MouseButtonPressedEvent& event);
		u8 on_mouse_button_released(MouseButtonReleasedEvent& event);
		u8 on_mouse_motion(MouseMotionEvent& event);
		u8 on_mouse_wheel(MouseWheelEvent& event);
		u8 on_window_close(WindowCloseEvent& event);

		template<typename T>
		umultimap<EEventType, function<void(Event&)>>::iterator bind_event_impl(function<void(T&)> fn)
		{
			auto l = [fn](Event& event)
			{
				if (event.type() == T::get_static_type())
				{
					fn(static_cast<T&>(event));
				}
			};

			return _eventBindings.insert({ T::get_static_type(), l });
		}

		void callback_bound_inputs(const InputKey& key);
		template<typename T> u8 callback_bound_events(T& event);

		IWindow* _window{ nullptr };
		u8 _isPendingTermination{ false };

		umultimap<InputKey, function<void()>> _inputBindings; // TODO: may want to combine input bindings and event bindings.
		umultimap<EEventType, function<void(Event&)>> _eventBindings;

		sref<INode> _root;

		static Application* _instance;
	};

	Application* create_application();

}