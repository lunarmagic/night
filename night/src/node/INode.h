#pragma once

#include "core.h"
#include "log/log.h"

#define BIND_INPUT(key, type, callback) ::night::Application::get().bind_input(key, type, callback) 

namespace night
{

	enum class ENodeVisibility
	{
		Invisible_Tree = 0,
		Invisible,
		Visible
	};

	struct INode;

	struct RenderGraph
	{
		struct Node
		{
			ref<INode> node;
			mat4 relative_transform;
		};

		multimap<real, Node> graph;
	};

	struct NIGHT_API INode : public std::enable_shared_from_this<INode>
	{
		template<typename T, typename... Args> ref<T> create(const string& name, const Args&... args);
		void remove(const string& name);
		void remove_all();
		ref<INode> find(const string& name);

		void destroy();

		ref<INode> parent() const { return _parent; }
		const vector<sref<INode>>& children() const { return _children; }

		const string& name() const { return _name; }
		const u64& unique_id() const { return _unique_id; }

		real timestamp() const { return _timestamp; }

		real lifespan() const { return _lifespan; }
		void lifespan(real lifespan) { _lifespan = lifespan; }

		ENodeVisibility visibility() const { return _visibility; }
		void visibility(ENodeVisibility visibility) { _visibility = visibility; }

		real depth() const { return _depth; }
		void depth(real depth) { _depth = depth; }

		const mat4& transform() const { return _transform; }
		void transform(const mat4& x) { _transform = x; }

		u8 is_pending_destruction() const { return _isPendingDestruction; }

		INode();
		virtual ~INode();
		
	protected:

		static u64 __uid;
		static string __name;
		static ref<INode> __parent;

		//virtual void on_initialize() { return; }
		virtual void on_update(real delta) { return; }
		virtual void on_render() { return; }
		//virtual void on_clean() { return; }

	private:

		friend struct Application; // TODO: remove friend struct
		void update(real delta);
		void render_tree();
		void render(RenderGraph& out_graph);

		friend struct Application;
		u64 _unique_id{ 0 };
		string _name{ "Unnamed" };
		ref<INode> _parent{ nullptr };
		vector<sref<INode>> _created;
		vector<sref<INode>> _children;
		real _timestamp{ -1.0f };
		real _lifespan{ -1.0f };
		u8 _isPendingDestruction{false};
		ENodeVisibility _visibility{ ENodeVisibility::Visible };
		real _depth{ 0 };
		mat4 _transform{mat4(1)};
		// TODO: add _static_type
	};

	template<typename T, typename... Args>
	inline ref<T> INode::create(const string& name, const Args&... args)
	{
		__name = name;
		__parent = weak_from_this();

		auto child = sref<T>(new T(args...));

		if (child)
		{
			_created.push_back(child);
			TRACE("Created child node: ", name);
			__uid++;
		}
		else
		{
			ERROR("Child is nullptr.");
		}

		__parent = nullptr;
		__name = "";

		return child;
	}

	namespace debug
	{
		template<> inline string _print_format<INode>(INode& v)
		{
			sstream stream;
			stream << "\n";
			stream << "Name: " << v.name() << "\n";
			stream << "Unique id: " << v.unique_id() << "\n";
			//stream << "Parent: name: " << v.parent()->name() << ", unique_id: " << v.parent()->unique_id() << "\n";
			stream << "Num children: " << v.children().size() << "\n";
			stream << "Timestamp: " << v.timestamp() << "\n";
			stream << "Visibility: " << 
				(v.visibility() == ENodeVisibility::Visible ? "Visible"
				: v.visibility() == ENodeVisibility::Invisible ? "Invisible"
				: "Invisible_Tree")
				<< "\n";

			stream << "Depth: " << v.depth() << "\n";
			stream << "Is pending destruction?: " << (v.is_pending_destruction() ? "True" : "False") << "\n";

			return stream.str();
		}
	}

}