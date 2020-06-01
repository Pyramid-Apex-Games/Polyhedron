#pragma once
#include "entityfactory.h"
#include <string>

namespace entities
{
    namespace classes
    {
        class CoreEntity;
    }
}

class EntityEditorMenu
{
public:
    EntityEditorMenu(entities::classes::CoreEntity* entity);

    void Render();
    void RenderHeader(const entities::attrubuteRow_T& attrs);
    void RenderInput(const entities::attrubuteRow_T& attrs);
    void RenderSlider(const entities::attrubuteRow_T& attrs);
    void RenderSliderInt(const entities::attrubuteRow_T& attrs);
    void RenderCheckbox(const entities::attrubuteRow_T& attrs);

    bool HasEntity(entities::classes::CoreEntity* entity);
    void Hide();
    void Show();

private:
    entities::classes::CoreEntity* m_Entity;
    bool m_Closed = false;
    float m_Width = 400;
    int m_AnimSlideInDuration = 500;
    int m_AnimSlideOutDuration = 500;

    int m_AnimSlideInStart = 0;
    int m_AnimSlideOutStart = 0;

    std::string __lastHeader;
    std::map<std::string, std::string> __inputStorage;
    std::map<std::string, float> __sliderStorage;
    std::map<std::string, int> __checkboxStorage;
};

