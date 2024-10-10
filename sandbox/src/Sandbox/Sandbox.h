#pragma once

#include "application/Application.h"
//#include "Box/Box.h"
#include "Drawing/Canvas.h"
#include "node/INode.h"

namespace night
{

	struct IGameMode;

struct Sandbox : public Application
{
	Sandbox() = default;

protected:

	virtual pair<IWindow*, WindowParams> create_window() override;
	virtual INode* create_root() override;
	virtual void on_load_resources() override;
	//void on_initialize() override;
	//virtual void on_update(real delta);

private:

	//ref<IGameMode> _gameMode;
};

}