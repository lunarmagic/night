
#include "nightpch.h"
#include "INode.h"
#include "Node3D.h"
#include "log/log.h"
#include "window/IWindow.h"
#include "utility.h"

namespace night
{

	u64 INode::__uid = 0;
	string INode::__name = "";
	ref<INode> INode::__parent = nullptr;

	INode::INode()
	{
		_unique_id = __uid;
		_name = __name;
		_parent = __parent;
		_visibility = ENodeVisibility::Visible;
		_timestamp = -1.0f;
		_lifespan = -1.0f;
		_isPendingDestruction = false;
	}

	INode::~INode()
	{
		TRACE("Destroying ", _name);

		remove_all();
	}

	void INode::update(real delta)
	{
		// update
		on_update(delta);

		for (auto i = _children.begin(); i != _children.end();)
		{
			auto& child = (*i);
			if (child != nullptr && !child->_isPendingDestruction)
			{
				child->update(delta);
				i++;
			}
			else
			{
				i = _children.erase(i);
			}
		}

		// properly add newly created children
		for (const auto& i : _created)
		{
			if (i != nullptr)
			{
				i->_timestamp = utility::window().time_elapsed();
				_children.push_back(i);
			}
		}

		_created.clear();

		// if lifespan runs out, destroy
		if (_lifespan != -1.0f)
		{
			real time_elapsed = utility::window().time_elapsed() - _timestamp;

			if (time_elapsed > _lifespan)
			{
				destroy();
			}
		}

		//for (s32 i = 0; i < _children.size(); i++)
		//{
		//	auto& child = _children[i];
		//	if (child != nullptr)
		//	{
		//
		//		child->update(delta);
		//	}
		//	else
		//	{
		//		// remove child
		//	}
		//}
	}

	void INode::render_tree()
	{
		RenderGraph graph;
		render(graph);

		for (auto i = graph.graph.begin(); i != graph.graph.end(); i++)
		{
			auto& element = (*i);
			ASSERT(element.second != nullptr);
			element.second->on_render();
		}
	}

	void INode::render(RenderGraph& out_graph)
	{
		if (_visibility != ENodeVisibility::Invisible_Tree)
		{
			for (auto i = _children.begin(); i != _children.end();)
			{
				if ((*i) != nullptr)
				{
					(*i)->render(out_graph);
					i++;
				}
				else
				{
					i = _children.erase(i);
				}
			}
		}

		if (_visibility == ENodeVisibility::Visible)
		{
			// TODO: if element is type Node2D or Node3D, apply transformation
			out_graph.graph.insert({ _depth, shared_from_this() });
		}
	}

	void INode::remove(const string& name)
	{
		auto fn = [&](auto& v)
		{
			for (auto i = v.begin(); i < v.end(); )
			{
				if ((*i) != nullptr)
				{
					if ((*i)->name() == name) // TODO: fix crash
					{
						i = v.erase(i);
					}
					else
					{
						i++;
					}
				}
				else
				{
					i = v.erase(i);
				}
			}
		};

		fn(_children);
		fn(_created);
	}

	void INode::remove_all()
	{
		_children.clear();
		_created.clear();
	}

	ref<INode> INode::find(const string& name)
	{
		// TODO: update this function.
		for (auto i = _children.begin(); i < _children.end(); i++)
		{
			if ((*i) != nullptr)
			{
				if ((*i)->name() == name)
				{
					return (*i);
				}
			}
			else
			{
				// remove child
			}
		}

		return nullptr;
	}

	void INode::destroy()
	{
		_isPendingDestruction = true;
	}

}