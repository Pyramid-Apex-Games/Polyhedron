#pragma once
#include "engine/state/State.h"

class SkeletalEntity;
class Camera;

class FpsGameState : public State
{
public:
    FpsGameState();
    ~FpsGameState();
    void Render(RenderPass pass) override;
    void Update() override;
    bool OnEvent(const Event&) override;

private:
    unsigned int ShouldScale();
    void RenderGame(RenderPass pass);
    void RenderMain();
    void RenderGui();

    SkeletalEntity* m_Player;
    Camera* m_Camera;
};