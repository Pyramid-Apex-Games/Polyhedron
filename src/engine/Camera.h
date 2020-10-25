#pragma once
#include "shared/entities/MovableEntity.h"

class Camera : public MovableEntity
{
    ENTITY_FACTORY_IMPL(Camera);
public:
    Camera();
    virtual ~Camera();

    void Update();
    static void UpdateAll();

    const matrix4& GetMatrix() const;
    const vec& GetDirection() const;
    const vec& GetUp() const;
    const vec& GetRight() const;
    const vec& GetWorldPos() const;
    std::tuple<const vec&, const vec&, const vec&, const vec&> GetDirUpRightWorld() const;

    static Camera* GetActiveCamera();
    static const std::vector<Camera*>& GetCameras();

private:
    static std::vector<Camera*> s_Cameras;
    static std::vector<Camera*>::iterator s_ActiveCamera;
    static void AddCamera(Camera* camera);
    static void RemoveCamera(Camera* camera);

    DONTSERIALIZE matrix4 m_Matrix;
    DONTSERIALIZE vec m_Direction;
    DONTSERIALIZE vec m_Up;
    DONTSERIALIZE vec m_Right;
    DONTSERIALIZE vec m_WorldPos;
};

