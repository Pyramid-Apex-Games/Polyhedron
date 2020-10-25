#include "Camera.h"

std::vector<Camera*> Camera::s_Cameras;
std::vector<Camera*>::iterator Camera::s_ActiveCamera = Camera::s_Cameras.end();

void Camera::AddCamera(Camera* camera)
{
    auto activeCamera = GetActiveCamera();
    s_Cameras.push_back(camera);
    s_ActiveCamera = std::find(s_Cameras.begin(), s_Cameras.end(), activeCamera);

    if (s_ActiveCamera == s_Cameras.end())
    {
        s_ActiveCamera = s_Cameras.begin();
    }
}

void Camera::RemoveCamera(Camera* camera)
{
    auto activeCamera = GetActiveCamera();

    s_Cameras.erase(
        std::remove(s_Cameras.begin(), s_Cameras.end(), camera),
        s_Cameras.end()
    );

    if (activeCamera == camera)
    {
        s_ActiveCamera = s_Cameras.begin();
    }
    else
    {
        s_ActiveCamera = std::find(s_Cameras.begin(), s_Cameras.end(), activeCamera);
        if (s_ActiveCamera == s_Cameras.end())
        {
            s_ActiveCamera = s_Cameras.begin();
        }
    }
}

Camera::Camera()
{
    Camera::AddCamera(this);
}

Camera::~Camera()
{
    Camera::RemoveCamera(this);
}

void Camera::Update()
{
    extern const matrix4 viewmatrix;
    extern int drawtex;
    extern int worldsize;

    m_Matrix = viewmatrix;
    m_Matrix.rotate_around_y(d.z *  RAD);
    m_Matrix.rotate_around_x(d.y * -RAD);
    m_Matrix.rotate_around_z(d.x * -RAD);
    m_Matrix.translate(vec(o).neg());

    m_Matrix.transposedtransformnormal(vec(viewmatrix.b), m_Direction);
    m_Matrix.transposedtransformnormal(vec(viewmatrix.a).neg(), m_Right);
    m_Matrix.transposedtransformnormal(vec(viewmatrix.c), m_Up);

    if(!drawtex)
    {
        if(raycubepos(o, m_Direction, m_WorldPos, 0, RAY_CLIPMAT|RAY_SKIPFIRST) == -1)
            m_WorldPos = vec(m_Direction).mul(2*worldsize).add(o);
            // if nothing is hit, just far away in the view direction
    }
}

const matrix4& Camera::GetMatrix() const
{
    return m_Matrix;
}

const vec& Camera::GetDirection() const
{
    return m_Direction;
}

const vec& Camera::GetUp() const
{
    return m_Up;
}

const vec& Camera::GetRight() const
{
    return m_Right;
}

const vec& Camera::GetWorldPos() const
{
    return m_WorldPos;
}

Camera *Camera::GetActiveCamera()
{
    if (s_ActiveCamera == s_Cameras.end())
    {
        if (s_Cameras.empty())
        {
            return nullptr;
        }

        s_ActiveCamera = s_Cameras.begin();
    }

    return *s_ActiveCamera;
}

std::tuple<const vec&, const vec&, const vec&, const vec&>
    Camera::GetDirUpRightWorld() const
{
    return std::tie(
        m_Direction,
        m_Up,
        m_Right,
        m_WorldPos
    );
}

const std::vector<Camera *>& Camera::GetCameras()
{
    return s_Cameras;
}

void Camera::UpdateAll()
{
    for (const auto& camera : s_Cameras)
    {
        camera->Update();
    }
}

void Camera::on(const Event& event)
{
}

ADD_ENTITY_TO_FACTORY_SERIALIZED(Camera, "camera", MovableEntity);
