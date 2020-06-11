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
    , m_AttributeTree(m_Entity->attributeTree())
    , m_AnimSlideInStart(totalmillis)
{
    for (const auto& attribSection : m_AttributeTree)
    {
        m_Widgets.push_back(
            std::make_unique<EditorWidgetGroup>(m_Entity, attribSection)
        );
    }
}

void EntityEditorMenu::Render()
{
    using namespace std::string_literals;

    if (m_Closed)
        return;

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

    for (auto& widgetGroup : m_Widgets)
    {
        widgetGroup->Render();
    }

    nk_end(engine::nui::GetNKContext());
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

const Entity *EntityEditorWidget::GetEntity() const
{
    return m_Entity;
}

Entity *EntityEditorWidget::GetEntity()
{
    return m_Entity;
}

const attributeRow_T &EntityEditorWidget::GetAttributes() const
{
    return m_Attributes;
}

void InputEntityEditorWidget::Render()
{
    auto variableKey = std::get<std::string>(GetAttributes()[1]);
    auto attribValue = GetEntity()->getAttribute(variableKey);

    if (!m_InputTypeConfig)
    {
        m_InputTypeConfig = std::make_unique<InputTypeConfig>(attribValue);
    }

    auto inputFieldCount = m_InputTypeConfig->StorageCount();
    for (int i = 0; i < inputFieldCount; ++i)
    {
        if (m_Storages[i].bufferSize == 0)
        {
            m_Storages[i] = InputStorageSpace(m_InputTypeConfig->ToString(), 255);
        }

        nk_flags event = nk_edit_string_zero_terminated(
            engine::nui::GetNKContext(),
            NK_EDIT_FIELD,
            m_Storages[i].pointer(),
            m_Storages[i].bufferSize,
            m_InputTypeConfig->GetInputFilterer()
        );
        if (event & NK_EDIT_ACTIVATED)
        {
            Application::Instance().GetInput().Text(true);
        }
        if (event & NK_EDIT_DEACTIVATED)
        {
            Application::Instance().GetInput().Text(false);
        }
        if (event & NK_EDIT_COMMITED)
        {
            GetEntity()->setAttribute(variableKey, m_Storages[i]);
        }
        if (event & NK_EDIT_ACTIVE)
        {
            std::string currentValue = m_Storages[i];
            if (m_Storages[i].pointer() && m_InputTypeConfig->ToString() != currentValue)
            {
                m_InputTypeConfig->ToSource(currentValue, i);
                GetEntity()->setAttribute(variableKey, m_InputTypeConfig->m_SourceVariable);
            }
        }
    }
}

InputEntityEditorWidget::InputEntityEditorWidget(Entity *entity, const attributeRow_T &attributes)
    : EntityEditorWidget(entity, attributes)
{
}

template <>
SliderEntityEditorWidget<int>::SliderEntityEditorWidget(Entity *entity, const attributeRow_T &attributes)
    : EntityEditorWidget(entity, attributes)
    , nk_slider_any(&nk_slider_int)
{}

template <>
SliderEntityEditorWidget<float>::SliderEntityEditorWidget(Entity *entity, const attributeRow_T &attributes)
    : EntityEditorWidget(entity, attributes)
    , nk_slider_any(&nk_slider_float)
{}

CheckboxEntityEditorWidget::CheckboxEntityEditorWidget(Entity *entity, const attributeRow_T &attributes)
    : EntityEditorWidget(entity, attributes)
{
}

void CheckboxEntityEditorWidget::Render()
{
    auto variableKey = std::get<std::string>(GetAttributes()[1]);
    auto variable = GetEntity()->getAttribute(variableKey);
    m_Storage = 0;
    if (std::holds_alternative<bool>(variable))
    {
        m_Storage = std::get<bool>(variable) ? 1 : 0;
    }
    else if (std::holds_alternative<int>(variable))
    {
        m_Storage = std::get<int>(variable) ? 1 : 0;
    }

    int workValue = m_Storage;
    nk_checkbox_label(engine::nui::GetNKContext(), "", &workValue);

    if (workValue != m_Storage)
    {
        if (std::holds_alternative<bool>(variable))
        {
            GetEntity()->setAttribute(variableKey, workValue == 1 ? true : false);
        }
        else if (std::holds_alternative<int>(variable))
        {
            GetEntity()->setAttribute(variableKey, workValue);
        }

        m_Storage = workValue;
    }
}

nk_context *EditorRenderable::Context()
{
    return engine::nui::GetNKContext();
}

void WidgetLabelPair::Render()
{
    nk_layout_row_dynamic(Context(), 30, 2);
    nk_label(Context(), m_Label.c_str(), NK_TEXT_LEFT);

    m_Widget->Render();
}

EditorWidgetGroup::EditorWidgetGroup(Entity* entity, const attributeList_T& attributeList)
{
    using namespace std::string_literals;

    for (auto& attrRow : attributeList)
    {
        if (std::get<std::string>(attrRow[0]) == "header"s)
        {
            m_Label = std::get<std::string>(attrRow[1]);
        }
        else
        {
            AppendWidget(entity, attrRow);
        }
    }
}

void EditorWidgetGroup::AppendWidget(Entity* entity, const attributeRow_T& attributes)
{
    using namespace std::string_literals;
    auto widgetType = std::get<std::string>(attributes[0]);
    auto variableKey = std::get<std::string>(attributes[1]);

    if (widgetType == "slider"s)
    {
        auto variable = entity->getAttribute(variableKey);
        if (std::holds_alternative<float>(variable))
        {
            m_Widgets.push_back(
                std::make_unique<WidgetLabelPair>(
                    std::get<std::string>(attributes[2]),
                    std::make_unique<SliderEntityEditorWidget<float> >(entity, attributes)
                )
            );
        }
        else if (std::holds_alternative<int>(variable))
        {
            m_Widgets.push_back(
                std::make_unique<WidgetLabelPair>(
                    std::get<std::string>(attributes[2]),
                    std::make_unique<SliderEntityEditorWidget<int> >(entity, attributes)
                )
            );
        }
    }
    else if (widgetType == "checkbox"s)
    {
        m_Widgets.push_back(
            std::make_unique<WidgetLabelPair>(
                std::get<std::string>(attributes[2]),
                std::make_unique<CheckboxEntityEditorWidget>(entity, attributes)
            )
        );
    }
    else if (
        widgetType == "input"s ||
        widgetType == "generic_prop"s ||
        widgetType == "vec"s ||
        widgetType == "vec4"s ||
        widgetType == "ivec"s ||
        widgetType == "ivec4"s
    )
    {
        m_Widgets.push_back(
            std::make_unique<WidgetLabelPair>(
                std::get<std::string>(attributes[2]),
                std::make_unique<InputEntityEditorWidget>(entity, attributes)
            )
        );
    }
    else
    {
        m_Widgets.push_back(
            std::make_unique<WidgetLabelPair>(
                std::get<std::string>(attributes[2]),
                std::make_unique<DummyEntityEditorWidget>(entity, attributes)
            )
        );
    }
}

void EditorWidgetGroup::Render()
{
    nk_layout_row_dynamic(Context(), 30, 1);
    nk_label(Context(), m_Label.c_str(), NK_TEXT_CENTERED);

    for (auto& widget : m_Widgets)
    {
        widget->Render();
    }
}

DummyEntityEditorWidget::DummyEntityEditorWidget(Entity *entity, const attributeRow_T &attributes)
    : EntityEditorWidget(entity, attributes)
    , m_Storage(std::get<std::string>(attributes[0]))
{
}

void DummyEntityEditorWidget::Render()
{
    nk_label(Context(), ("-(" + m_Storage + ")-").c_str(), NK_TEXT_CENTERED);
}

std::string InputEntityEditorWidget::InputTypeConfig::ToString(int fromStorageIndex)
{
    if (std::holds_alternative<vec4>(m_SourceVariable))
    {
        assert(fromStorageIndex >= 0 && fromStorageIndex < 4 && "StorageIndex out of bounds!");

        auto unwrappedVariable = std::get<vec4>(m_SourceVariable);
        return stringConverter(unwrappedVariable[fromStorageIndex]);
    }
    else if (std::holds_alternative<vec>(m_SourceVariable))
    {
        assert(fromStorageIndex >= 0 && fromStorageIndex < 3 && "StorageIndex out of bounds!");

        auto unwrappedVariable = std::get<vec>(m_SourceVariable);
        return stringConverter(unwrappedVariable[fromStorageIndex]);
    }
    else if (std::holds_alternative<ivec4>(m_SourceVariable))
    {
        assert(fromStorageIndex >= 0 && fromStorageIndex < 4 && "StorageIndex out of bounds!");

        auto unwrappedVariable = std::get<ivec4>(m_SourceVariable);
        return stringConverter(unwrappedVariable[fromStorageIndex]);
    }
    else if (std::holds_alternative<ivec>(m_SourceVariable))
    {
        assert(fromStorageIndex >= 0 && fromStorageIndex < 3 && "StorageIndex out of bounds!");

        auto unwrappedVariable = std::get<ivec>(m_SourceVariable);
        return stringConverter(unwrappedVariable[fromStorageIndex]);
    }

    return std::visit(stringConverter, m_SourceVariable);
}

void InputEntityEditorWidget::InputTypeConfig::ToSource(const std::string& stringValue, int fromStorageIndex)
{
    if (std::holds_alternative<std::string>(m_SourceVariable))
    {
        m_SourceVariable = stringValue;
    }
    else if (std::holds_alternative<float>(m_SourceVariable))
    {
        m_SourceVariable = floatConverter(stringValue);
    }
    else if (std::holds_alternative<int>(m_SourceVariable))
    {
        m_SourceVariable = intConverter(stringValue);
    }
    else if (std::holds_alternative<bool>(m_SourceVariable))
    {
        m_SourceVariable = boolConverter(stringValue);
    }
    else if (std::holds_alternative<vec4>(m_SourceVariable))
    {
        assert(fromStorageIndex >= 0 && fromStorageIndex < 4 && "StorageIndex out of bounds!");

        auto unwrappedVariable = std::get<vec4>(m_SourceVariable);
        unwrappedVariable[fromStorageIndex] = floatConverter(stringValue);
        m_SourceVariable = unwrappedVariable;
    }
    else if (std::holds_alternative<vec>(m_SourceVariable))
    {
        assert(fromStorageIndex >= 0 && fromStorageIndex < 3 && "StorageIndex out of bounds!");

        auto unwrappedVariable = std::get<vec>(m_SourceVariable);
        unwrappedVariable[fromStorageIndex] = floatConverter(stringValue);
        m_SourceVariable = unwrappedVariable;
    }
    else if (std::holds_alternative<ivec4>(m_SourceVariable))
    {
        assert(fromStorageIndex >= 0 && fromStorageIndex < 4 && "StorageIndex out of bounds!");

        auto unwrappedVariable = std::get<ivec4>(m_SourceVariable);
        unwrappedVariable[fromStorageIndex] = intConverter(stringValue);
        m_SourceVariable = unwrappedVariable;
    }
    else if (std::holds_alternative<ivec>(m_SourceVariable))
    {
        assert(fromStorageIndex >= 0 && fromStorageIndex < 3 && "StorageIndex out of bounds!");

        auto unwrappedVariable = std::get<ivec>(m_SourceVariable);
        unwrappedVariable[fromStorageIndex] = intConverter(stringValue);
        m_SourceVariable = unwrappedVariable;
    }
}

int InputEntityEditorWidget::InputTypeConfig::StorageCount()
{
    if (std::holds_alternative<vec4>(m_SourceVariable))
    {
        return 4;
    }
    else if (std::holds_alternative<vec>(m_SourceVariable))
    {
        return 3;
    }
    else if (std::holds_alternative<ivec4>(m_SourceVariable))
    {
        return 3;
    }
    else if (std::holds_alternative<ivec>(m_SourceVariable))
    {
        return 4;
    }

    return 1;
}

InputEntityEditorWidget::InputTypeConfig::InputTypeConfig(const attribute_T variable)
    : m_SourceVariable(variable)
{
}

InputEntityEditorWidget::InputTypeConfig::nk_filter_func_t
InputEntityEditorWidget::InputTypeConfig::GetInputFilterer() const
{
    if (std::holds_alternative<std::string>(m_SourceVariable))
    {
        return &nk_filter_ascii;
    }
    else if (std::holds_alternative<float>(m_SourceVariable))
    {
        return &nk_filter_float;
    }
    else if (std::holds_alternative<int>(m_SourceVariable))
    {
        return &nk_filter_decimal;
    }
    else if (std::holds_alternative<bool>(m_SourceVariable))
    {
        return &nk_filter_default;
    }
    else if (std::holds_alternative<vec4>(m_SourceVariable))
    {
        return &nk_filter_float;
    }
    else if (std::holds_alternative<vec>(m_SourceVariable))
    {
        return &nk_filter_float;
    }
    else if (std::holds_alternative<ivec4>(m_SourceVariable))
    {
        return &nk_filter_decimal;
    }
    else if (std::holds_alternative<ivec>(m_SourceVariable))
    {
        return &nk_filter_decimal;
    }

    return &nk_filter_default;
}
