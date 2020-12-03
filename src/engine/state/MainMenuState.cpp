#include "MainMenuState.h"
#include "engine/main/Compatibility.h"
#include "engine/main/Application.h"
#include "engine/main/Renderer.h"

MainMenuState::MainMenuState()
{
}

MainMenuState::~MainMenuState()
{

}

void MainMenuState::Render(RenderPass pass)
{
    Application::Instance().GetRenderer().RenderBackground({}, true);
}

void MainMenuState::Update()
{

}

bool MainMenuState::OnEvent(const Event&)
{

}