#pragma once
#include "engine/engine.h"
#include "shared/event/Event.h"

class State
{
public:
    State();
    virtual ~State();
    virtual void Render(RenderPass pass) = 0;
    virtual void Update();
    virtual bool OnEvent(const Event&) = 0;

protected:

};