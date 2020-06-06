#pragma once
#include "EntityFactory.h"
#include <string>

class Entity;


class EntityEditorMenu
{
public:
    EntityEditorMenu(Entity* entity);

    void Render();
    void RenderHeader(const attributeRow_T& attrs);
    void RenderInput(const attributeRow_T& attrs);
    void RenderSlider(const attributeRow_T& attrs);
    void RenderSliderInt(const attributeRow_T& attrs);
    void RenderCheckbox(const attributeRow_T& attrs);

    bool HasEntity(Entity* entity);
    void Hide();
    void Show();

private:
    Entity* m_Entity;
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


