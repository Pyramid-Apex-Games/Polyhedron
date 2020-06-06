#include "EntityEditorMenu.h"
#include "shared/entities/Entity.h"
#include "engine/main/Application.h"
#include "engine/main/Window.h"
#include "engine/main/GLContext.h"
#include "engine/main/Input.h"
#include "engine/nui/nui.h"
#include "shared/Easing.h"
#include <variant>

EntityEditorMenu::EntityEditorMenu(Entity* entity)
    : m_Entity(entity)
    , m_AnimSlideInStart(totalmillis)
{
}

void EntityEditorMenu::Render()
{
    using namespace std::string_literals;

    if (m_Closed)
        return;

    auto& attrs = m_Entity->attributes();

    int screenw = 0, screenh = 0;
    Application::Instance().GetWindow().GetContext().GetFramebufferSize(screenw, screenh);

    if (totalmillis < m_AnimSlideInStart + m_AnimSlideInDuration)
    {
        float animRatio = Easing::ElasticEaseOut(float(totalmillis - m_AnimSlideInStart) / float(m_AnimSlideInDuration));

        nk_begin(engine::nui::GetNKContext(), "Properties", nk_rect(screenw - m_Width * animRatio, 0, std::max(m_Width, m_Width * animRatio), screenh), 0);
    }
    else if(totalmillis < m_AnimSlideOutStart + m_AnimSlideOutDuration)
    {
        float animRatio = 1.0f - Easing::ElasticEaseOut(float(totalmillis - m_AnimSlideOutStart) / float(m_AnimSlideOutDuration));

        if (animRatio <= 0.0f)
        {
            animRatio = 0.0f;
            m_Closed = true;
        }
        nk_begin(engine::nui::GetNKContext(), "Properties", nk_rect(screenw - m_Width * animRatio, 0, std::max(m_Width, m_Width * animRatio), screenh), 0);
    }
    else
    {
        nk_begin(engine::nui::GetNKContext(), "Properties", nk_rect(screenw - m_Width, 0, m_Width, screenh), 0);
    }

    for (auto& attrRow : attrs)
    {
        if (std::get<std::string>(attrRow[0]) == "header"s)
        {
            RenderHeader(attrRow);
        }
        else
        {
            nk_layout_row_dynamic(engine::nui::GetNKContext(), 30, 2);
            nk_label(engine::nui::GetNKContext(), std::get<std::string>(attrRow[2]).c_str(), NK_TEXT_LEFT);

            if (std::get<std::string>(attrRow[0]) == "slider"s)
            {
                auto variableKey = std::get<std::string>(attrRow[1]);
                auto variable = m_Entity->getAttribute(variableKey);
                if (std::holds_alternative<float>(variable))
                {
                    RenderSlider(attrRow);
                }
                else if (std::holds_alternative<int>(variable))
                {
                    RenderSliderInt(attrRow);
                }
            }
            else if (std::get<std::string>(attrRow[0]) == "checkbox"s)
            {
                RenderCheckbox(attrRow);
            }
            else if (std::get<std::string>(attrRow[0]) == "input"s)
            {
                RenderInput(attrRow);
            }
            else
            {
                nk_label(engine::nui::GetNKContext(), ("-(" + std::get<std::string>(attrRow[0]) + ")-").c_str(), NK_TEXT_CENTERED);
            }
        }
    }

    nk_end(engine::nui::GetNKContext());
}

void EntityEditorMenu::RenderHeader(const attributeRow_T& attrs)
{
    nk_layout_row_dynamic(engine::nui::GetNKContext(), 30, 1);
    auto headerString = std::get<std::string>(attrs[1]);
    __lastHeader = headerString;
    nk_label(engine::nui::GetNKContext(), headerString.c_str(), NK_TEXT_CENTERED);
}

void EntityEditorMenu::RenderInput(const attributeRow_T& attrs)
{
    auto variableKey = std::get<std::string>(attrs[1]);
    auto storageKey = __lastHeader + "_" + variableKey;
    auto variableValue = std::get<std::string>(m_Entity->getAttribute(variableKey));

    if (__inputStorage.find(storageKey) == __inputStorage.end())
    {
        std::string valueStorage = variableValue;
        valueStorage.reserve(255);
        __inputStorage[storageKey] = valueStorage;
    }

    nk_flags event = nk_edit_string_zero_terminated(
        engine::nui::GetNKContext(),
        NK_EDIT_FIELD,
        __inputStorage[storageKey].data(),
        255,
        nk_filter_ascii
    );
    if (event & NK_EDIT_ACTIVATED)
    {
        Application::Instance().GetInput().Text(true);
        conoutf("Input field %s activated", variableKey.c_str());
    }
    if (event & NK_EDIT_DEACTIVATED)
    {
        Application::Instance().GetInput().Text(false);
        conoutf("Input field %s deactivated", variableKey.c_str());
    }
    if (event & NK_EDIT_COMMITED)
    {
        m_Entity->setAttribute(variableKey, __inputStorage[storageKey]);
    }
}

//TODO: replace with template to switch between int/float
void EntityEditorMenu::RenderSlider(const attributeRow_T& attrs)
{
    auto variableKey = std::get<std::string>(attrs[1]);
    auto storageKey = __lastHeader + "_" + variableKey;
    auto variableValue = std::get<float>(m_Entity->getAttribute(variableKey));

    if (__sliderStorage.find(storageKey) == __sliderStorage.end())
    {
        float valueStorage = variableValue;
        __sliderStorage[storageKey] = valueStorage;
    }

    //FIXME: why are these attribs string to begin with? Issue with the PHUI macro? Issue with the python/libclang?
    auto minValue = std::stof(std::get<std::string>(attrs[3]));
    auto maxValue = std::stof(std::get<std::string>(attrs[4]));
    auto stepValue = std::stof(std::get<std::string>(attrs[5]));

    auto workValue = __sliderStorage[storageKey];
    nk_slider_float(engine::nui::GetNKContext(), minValue, &workValue, maxValue, stepValue);

    if (workValue != __sliderStorage[storageKey])
    {
        m_Entity->setAttribute(variableKey, workValue);
        __sliderStorage[storageKey] = workValue;
    }
}

//TODO: replace with template to switch between int/float
void EntityEditorMenu::RenderSliderInt(const attributeRow_T& attrs)
{
    auto variableKey = std::get<std::string>(attrs[1]);
    auto storageKey = __lastHeader + "_" + variableKey;
    auto variableValue = std::get<int>(m_Entity->getAttribute(variableKey));

    if (__sliderStorage.find(storageKey) == __sliderStorage.end())
    {
        float valueStorage = variableValue;
        __sliderStorage[storageKey] = valueStorage;
    }

    auto minValue = std::get<int>(attrs[3]);
    auto maxValue = std::get<int>(attrs[4]);
    auto stepValue = std::get<int>(attrs[5]);

    auto workValue = __sliderStorage[storageKey];
    nk_slider_float(engine::nui::GetNKContext(), minValue, &workValue, maxValue, stepValue);

    if (workValue != __sliderStorage[storageKey])
    {
        m_Entity->setAttribute(variableKey, workValue);
        __sliderStorage[storageKey] = workValue;
    }
}

void EntityEditorMenu::RenderCheckbox(const attributeRow_T& attrs)
{
    auto variableKey = std::get<std::string>(attrs[1]);
    auto storageKey = __lastHeader + "_" + variableKey;
    auto variable = m_Entity->getAttribute(variableKey);
    int variableValue = 0;
    if (std::holds_alternative<bool>(variable))
    {
        variableValue = std::get<bool>(variable) ? 1 : 0;
    }
    else if (std::holds_alternative<int>(variable))
    {
        variableValue = std::get<int>(variable) ? 1 : 0;
    }

    if (__checkboxStorage.find(storageKey) == __checkboxStorage.end())
    {
        __checkboxStorage[storageKey] = variableValue;
    }

    int workValue = __checkboxStorage[storageKey];
    nk_checkbox_label(engine::nui::GetNKContext(), "", &workValue);

    if (workValue != __checkboxStorage[storageKey])
    {
        if (std::holds_alternative<bool>(variable))
        {
            m_Entity->setAttribute(variableKey, workValue == 1 ? true : false);
        }
        else if (std::holds_alternative<int>(variable))
        {
            m_Entity->setAttribute(variableKey, workValue);
        }

        __checkboxStorage[storageKey] = workValue;
    }
}

bool EntityEditorMenu::HasEntity(Entity* entity)
{
    return entity == m_Entity;
}

void EntityEditorMenu::Hide()
{
    m_AnimSlideOutStart = totalmillis;
}

void EntityEditorMenu::Show()
{
    if (m_AnimSlideOutStart > 0)
    {
        m_AnimSlideOutStart = 0;
        m_AnimSlideInStart = totalmillis;
        m_Closed = false;
    }
}
