#pragma once

#include "shared/command.h"
#include "engine/scriptexport.h"
#include "shared/event/GameEvent.h"

class KeyControl {
    friend void backward(CommandTypes::KeyPress down);
    friend void forward(CommandTypes::KeyPress down);
    friend void left(CommandTypes::KeyPress down);
    friend void right(CommandTypes::KeyPress down);
    friend void jump(CommandTypes::KeyPress down);
    friend void crouch(CommandTypes::KeyPress down);

    template <EventType BoundEvent>
    class KeyAxisState
    {
        bool m_InputMin = false;
        bool m_InputMax = false;
        enum class Bias
        {
            None, Min, Max
        } m_Bias = Bias::None;

    public:
        void SetMin(bool pressed)
        {
            m_InputMin = pressed;
            m_Bias = Bias::Min;

            Signal();
        }
        void SetMax(bool pressed)
        {
            m_InputMax = pressed;
            m_Bias = Bias::Max;

            Signal();
        }
        int Axis() const
        {
            if (m_Bias == Bias::Min)
                return m_InputMin ? -1 : (m_InputMax ? 1 : 0);
            return m_InputMax ? 1 : (m_InputMin ? -1 : 0);
        }

    private:
        int m_LastAxis = 0;
        void Signal()
        {
            const auto currentAxis = Axis();
            if (m_LastAxis != currentAxis)
            {
                m_LastAxis = currentAxis;
            }
            GameEventData<int> evt(BoundEvent, currentAxis);
            Event::Broadcast(evt);
        }
    };

    template <EventType BoundEvent>
    class KeyState
    {
        bool m_Input = false;

    public:
        void Set(bool pressed)
        {
            m_Input = pressed;

            Signal();
        }
        bool Get() const
        {
            return m_Input;
        }

    private:
        bool m_LastInput = 0;
        void Signal()
        {
            const auto current = Get();
            if (m_LastInput != current)
            {
                m_LastInput = current;
            }
            GameEventData<bool> evt(BoundEvent, current);
            Event::Broadcast(evt);
        }
    };

    static KeyAxisState<EventType::Move> m_Movement;
    static KeyAxisState<EventType::Strafe> m_Strafe;
    static KeyState<EventType::Jump> m_Jump;
    static KeyState<EventType::Crouch> m_Crouch;
public:

};


void backward(CommandTypes::KeyPress down);
void forward(CommandTypes::KeyPress down);
void left(CommandTypes::KeyPress down);
void right(CommandTypes::KeyPress down);
void jump(CommandTypes::KeyPress down);
void crouch(CommandTypes::KeyPress down);
