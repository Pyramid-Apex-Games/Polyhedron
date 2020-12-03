#include "State.h"
#include "engine/Camera.h"

State::State()
{

}

State::~State()
{

}

bool State::OnEvent(const Event&)
{
    return true;
}

void State::Update()
{
    Camera::UpdateAll();
}
