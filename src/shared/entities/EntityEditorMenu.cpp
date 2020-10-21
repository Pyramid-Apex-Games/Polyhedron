#include "EntityEditorMenu.h"
#include "shared/entities/Entity.h"
#include "engine/main/Application.h"
#include "engine/main/Window.h"
#include "engine/main/GLContext.h"
#include "engine/main/Input.h"
#include "shared/Easing.h"
#include <imgui.h>
#include <variant>
#include <imgui_internal.h>

EntityEditorMenu::EntityEditorMenu(size_t entityId)
    : m_EntityID(entityId)
    , m_AnimSlideInStart(totalmillis)
{
    if (!GetEntity())
    {
        return;
    }

    m_AttributeTree = GetEntity()->attributeTree();
    for (const auto& attribSection : m_AttributeTree)
    {
        if (attribSection.size() > 1)
        {
            m_Widgets.push_back(
                std::make_unique<EditorWidgetGroup>(m_EntityID, attribSection)
            );
        }
    }
}

void EntityEditorMenu::Render()
{
    using namespace std::string_literals;

//    if (!m_Open)
//        return;
//
    int screenw = 0, screenh = 0;
    Application::Instance().GetWindow().GetContext().GetFramebufferSize(screenw, screenh);

    if (totalmillis < m_AnimSlideInStart + m_AnimSlideInDuration)
    {
        float animRatio = Easing::ElasticEaseOut(float(totalmillis - m_AnimSlideInStart) / float(m_AnimSlideInDuration));

        ImGui::SetNextWindowSize(ImVec2(m_Width * animRatio, screenh));
        ImGui::SetNextWindowPos(ImVec2(screenw - m_Width * animRatio, 0));
    }
    else if(totalmillis < m_AnimSlideOutStart + m_AnimSlideOutDuration)
    {
        float animRatio = 1.0f - Easing::ElasticEaseOut(float(totalmillis - m_AnimSlideOutStart) / float(m_AnimSlideOutDuration));

        if (animRatio <= 0.0f)
        {
            animRatio = 0.0f;
            m_Open = false;
        }
        ImGui::SetNextWindowSize(ImVec2(m_Width * animRatio, screenh));
        ImGui::SetNextWindowPos(ImVec2(screenw - m_Width * animRatio, 0));
    }
    else
    {
        ImGui::SetNextWindowSize(ImVec2(m_Width, screenh));
        ImGui::SetNextWindowPos(ImVec2(screenw - m_Width, 0));
    }

    ImGui::Begin("Properties", &m_Open, ImGuiWindowFlags_NoMove);
    ImGui::Columns(2, nullptr, true);

    for (auto& widgetGroup : m_Widgets)
    {
        widgetGroup->Render();
//        ImGui::NextColumn();
    }

    ImGui::Columns(1);
    ImGui::End();
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
        m_Open = true;
    }
}

Entity* EntityEditorMenu::GetEntity() const
{
    const auto& ents = getents();
    if (ents.inrange(m_EntityID))
    {
        return ents[m_EntityID];
    }
    return nullptr;
}

bool EntityEditorMenu::HasEntity(Entity* entity) const
{
    return entity == GetEntity() && entity != nullptr;
}

const Entity* EntityEditorWidget::GetEntity() const
{
    const auto& ents = getents();
    if (ents.inrange(m_EntityID))
    {
        return ents[m_EntityID];
    }

    return nullptr;
}

Entity* EntityEditorWidget::GetEntity()
{
    const auto& ents = getents();
    if (ents.inrange(m_EntityID))
    {
        return ents[m_EntityID];
    }

    return nullptr;
}

const attributeRow_T &EntityEditorWidget::GetAttributes() const
{
    return m_Attributes;
}

void InputEntityEditorWidget::Render()
{
    auto label = std::get<std::string>(GetAttributes()[0]);
    auto variableKey = std::get<std::string>(GetAttributes()[1]);
    auto attribValue = GetEntity()->getAttribute(variableKey);

    if (!m_InputTypeConfig)
    {
        m_InputTypeConfig = std::make_unique<InputTypeConfig>(attribValue);
    }
    else
    {
        m_InputTypeConfig->Update(attribValue);
    }

    const auto inputFieldCount = m_InputTypeConfig->StorageCount();
    if (inputFieldCount > 1)
    {
//        ImGui::BeginGroup();
        ImGui::PushMultiItemsWidths(inputFieldCount, ImGui::CalcItemWidth());
    }
    bool needsUpdate = false;
    for (int i = 0; i < inputFieldCount; ++i)
    {
        ImGui::PushID(i);

        if (m_Storages.size() <= i)
        {
            m_Storages.push_back(InputStorageSpace(*(m_InputTypeConfig.get()), 255));
        }

        if (i > 0)
            ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);

        m_Storages[i].Update(i);

        m_Storages[i].Render(inputFieldCount == 1 ? m_Label.c_str() : "", i, variableKey);

        std::string currentValue = m_Storages[i];
        if (m_Storages[i].pointer() && m_Storages[i].inputConfig.ToString() != currentValue)
        {
            m_Storages[i].inputConfig.ToSource(currentValue, i);
            needsUpdate = true;
        }

        ImGui::PopID();
        if (inputFieldCount > 1)
        {
            ImGui::PopItemWidth();
        }
    }

    if (needsUpdate)
    {
        GetEntity()->setAttribute(variableKey, m_InputTypeConfig->m_SourceVariable);
    }

//    if (inputFieldCount > 1)
//    {
//        ImGui::EndGroup();
//    }
}

void InputEntityEditorWidget::InputStorageSpace::Render(const std::string& label, int fromStorageIndex, const std::string& variable)
{
    ImGui::InputText(
        "",
        pointer(),
        bufferSize/*,
        ImGuiInputTextFlags_CallbackCharFilter,
        inputConfig.GetInputFilterer()*/
    );
}

InputEntityEditorWidget::InputEntityEditorWidget(size_t entityId, const attributeRow_T &attributes)
    : EntityEditorWidget(entityId, attributes)
{
}

template <>
SliderEntityEditorWidget<int>::SliderEntityEditorWidget(size_t entityId, const attributeRow_T &attributes)
    : EntityEditorWidget(entityId, attributes)
{}

template <>
void SliderEntityEditorWidget<int>::ImGuiSliderAnyImpl(const char* label, int *val, int min, int max, int step)
{
    ImGui::SliderInt(label, val, min, max);
}

template <>
SliderEntityEditorWidget<float>::SliderEntityEditorWidget(size_t entityId, const attributeRow_T &attributes)
    : EntityEditorWidget(entityId, attributes)
{
}

template <>
void SliderEntityEditorWidget<float>::ImGuiSliderAnyImpl(const char* label, float *val, float min, float max, float step)
{
    ImGui::SliderFloat(label, val, min, max);
}

CheckboxEntityEditorWidget::CheckboxEntityEditorWidget(size_t entityId, const attributeRow_T &attributes)
    : EntityEditorWidget(entityId, attributes)
{
}

void CheckboxEntityEditorWidget::Render()
{
    auto variableKey = std::get<std::string>(GetAttributes()[1]);
    auto variable = GetEntity()->getAttribute(variableKey);
    m_Storage = false;
    if (std::holds_alternative<bool>(variable))
    {
        m_Storage = std::get<bool>(variable) ? true : false;
    }
    else if (std::holds_alternative<int>(variable))
    {
        m_Storage = std::get<int>(variable) ? true : false;
    }

    bool workValue = m_Storage;

    ImGui::Checkbox("", &workValue);

    if (workValue != m_Storage)
    {
        if (std::holds_alternative<bool>(variable))
        {
            GetEntity()->setAttribute(variableKey, workValue ? true : false);
        }
        else if (std::holds_alternative<int>(variable))
        {
            GetEntity()->setAttribute(variableKey, workValue ? 1 : 0);
        }

        m_Storage = workValue;
    }
}

//ImGuiContext& EditorRenderable::Context()
//{
//    return ImGui;
//}

void WidgetLabelPair::Render()
{
    ImGui::PushID(m_Label.c_str());
    ImGui::Text(m_Label.c_str());
    ImGui::NextColumn();

    ImGui::SetNextItemWidth(-1);
    m_Widget->Render();
    ImGui::NextColumn();
    ImGui::PopID();
}

EditorWidgetGroup::EditorWidgetGroup(size_t entityId, const attributeList_T& attributeList)
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
            AppendWidget(entityId, attrRow);
        }
    }
}

void EditorWidgetGroup::AppendWidget(size_t entityId, const attributeRow_T& attributes)
{
    const auto& ents = getents();
    if (!ents.inrange(entityId))
    {
        return;
    }
    auto entity = ents[entityId];

    using namespace std::string_literals;
    auto widgetType = std::get<std::string>(attributes[0]);
    auto variableKey = std::get<std::string>(attributes[1]);
    auto label = std::get<std::string>(attributes[2]);

    if (widgetType == "slider"s)
    {
        auto variable = entity->getAttribute(variableKey);
        if (std::holds_alternative<float>(variable))
        {
            m_Widgets.push_back(
                std::make_unique<WidgetLabelPair>(
                    label,
                    std::make_unique<SliderEntityEditorWidget<float> >(entityId, attributes)
                )
            );
        }
        else if (std::holds_alternative<int>(variable))
        {
            m_Widgets.push_back(
                std::make_unique<WidgetLabelPair>(
                    label,
                    std::make_unique<SliderEntityEditorWidget<int> >(entityId, attributes)
                )
            );
        }
    }
    else if (widgetType == "checkbox"s)
    {
        m_Widgets.push_back(
            std::make_unique<WidgetLabelPair>(
                label,
                std::make_unique<CheckboxEntityEditorWidget>(entityId, attributes)
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
                label,
                std::make_unique<InputEntityEditorWidget>(entityId, attributes)
            )
        );
    }
    else
    {
        m_Widgets.push_back(
            std::make_unique<WidgetLabelPair>(
                label,
                std::make_unique<DummyEntityEditorWidget>(entityId, attributes)
            )
        );
    }
}

void EditorWidgetGroup::Render()
{
    ImGui::AlignTextToFramePadding();
    ImGui::SetNextItemOpen(m_OpenState, ImGuiCond_Once);
    if (ImGui::TreeNode(m_Label.c_str()))
    {
        ImGui::NextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("");
        ImGui::NextColumn();

        for (auto &widget : m_Widgets) {
            widget->Render();
        }

        ImGui::TreePop();
        m_OpenState = true;
    }
    else
    {
        ImGui::NextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("");
        ImGui::NextColumn();
        m_OpenState = false;
    }
}

DummyEntityEditorWidget::DummyEntityEditorWidget(size_t entityId, const attributeRow_T &attributes)
    : EntityEditorWidget(entityId, attributes)
    , m_Storage(std::get<std::string>(attributes[0]))
{
}

void DummyEntityEditorWidget::Render()
{
    ImGui::Text(m_Storage.c_str());
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
        return 4;
    }
    else if (std::holds_alternative<ivec>(m_SourceVariable))
    {
        return 3;
    }

    return 1;
}

InputEntityEditorWidget::InputTypeConfig::InputTypeConfig(const attribute_T variable)
    : m_SourceVariable(variable)
{
}

void InputEntityEditorWidget::InputTypeConfig::Update(const attribute_T variable)
{
    m_SourceVariable = variable;
}

AttributeVisitCoercer<std::string> InputEntityEditorWidget::InputTypeConfig::stringConverter;
AttributeVisitCoercer<float> InputEntityEditorWidget::InputTypeConfig::floatConverter;
AttributeVisitCoercer<int> InputEntityEditorWidget::InputTypeConfig::intConverter;
AttributeVisitCoercer<bool> InputEntityEditorWidget::InputTypeConfig::boolConverter;

namespace {
    int InputFilterAny(ImGuiInputTextCallbackData* data)
    {
        return 1;
    }
}

InputEntityEditorWidget::InputTypeConfig::filter_func_t
InputEntityEditorWidget::InputTypeConfig::GetInputFilterer() const
{
    /*if (std::holds_alternative<std::string>(m_SourceVariable))
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
    }*/

    return &InputFilterAny;
}


InputEntityEditorWidget::InputStorageSpace &
InputEntityEditorWidget::InputStorageSpace::operator=(InputEntityEditorWidget::InputStorageSpace other)
{
    std::swap(buffer, other.buffer);
    std::swap(bufferSize, other.bufferSize);
    std::swap(inputConfig, other.inputConfig);
    return *this;
}

//InputEntityEditorWidget::InputStorageSpace::InputStorageSpace()
//    : buffer()
//    , bufferSize(0)
//    , inputConfig()
//{
//}
