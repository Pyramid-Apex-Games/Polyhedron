#pragma once
#include "engine/state/State.h"

class MainMenuState : public State
{
public:
    MainMenuState();
    virtual ~MainMenuState();

    virtual void Render(RenderPass pass);
    virtual void Update();
    virtual bool OnEvent(const Event&);
};