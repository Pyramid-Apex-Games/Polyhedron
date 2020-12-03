#include "KeyControl.h"

KeyControl::KeyAxisState<EventType::Move> KeyControl::m_Movement;
KeyControl::KeyAxisState<EventType::Strafe> KeyControl::m_Strafe;
KeyControl::KeyState<EventType::Jump> KeyControl::m_Jump;
KeyControl::KeyState<EventType::Crouch> KeyControl::m_Crouch;

SCRIPTEXPORT void backward(CommandTypes::KeyPress down)
{
    KeyControl::m_Movement.SetMin(*down != 0);
}

SCRIPTEXPORT void forward(CommandTypes::KeyPress down)
{
    KeyControl::m_Movement.SetMax(*down != 0);
}

SCRIPTEXPORT void left(CommandTypes::KeyPress down)
{
    KeyControl::m_Strafe.SetMax(*down != 0);
}

SCRIPTEXPORT void right(CommandTypes::KeyPress down)
{
    KeyControl::m_Strafe.SetMin(*down != 0);
}

SCRIPTEXPORT void jump(CommandTypes::KeyPress down)
{
    KeyControl::m_Jump.Set(*down);
}

SCRIPTEXPORT void crouch(CommandTypes::KeyPress down)
{
    KeyControl::m_Crouch.Set(*down);
}