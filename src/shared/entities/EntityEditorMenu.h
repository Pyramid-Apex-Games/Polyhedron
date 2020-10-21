#pragma once
#include "EntityFactory.h"
#include "engine/editor/ui.h"
#include <string>
#include <cstring>

class Entity;
class ImGuiContext;
class ImGuiInputTextCallbackData;

class EditorRenderable
{
public:
    virtual void Render() = 0;
    virtual ~EditorRenderable() {};
};

class EntityEditorWidget : public EditorRenderable
{
public:
    EntityEditorWidget(size_t entityId, const attributeRow_T& attributes)
        : m_EntityID(entityId)
        , m_Attributes(attributes)
        , m_Label(std::get<std::string>(attributes[2]))
    {
    }

protected:
    const Entity* GetEntity() const;
    Entity* GetEntity();
    const attributeRow_T& GetAttributes() const;

private:
    size_t m_EntityID;
    const attributeRow_T& m_Attributes;

protected:
    const std::string m_Label;
};

class InputEntityEditorWidget : public EntityEditorWidget
{
public:
    InputEntityEditorWidget(size_t entityId, const attributeRow_T& attributes);
    void Render() override;

private:
    struct InputTypeConfig
    {
        using filter_func_t = int (*)(ImGuiInputTextCallbackData*);
        attribute_T m_SourceVariable;

        std::string ToString(int fromStorageIndex = 0);
        void ToSource(const std::string& stringValue, int fromStorageIndex = 0);
        int StorageCount();
        explicit InputTypeConfig(const attribute_T variable);
        void Update(const attribute_T variable);
        filter_func_t GetInputFilterer() const;


        static AttributeVisitCoercer<std::string> stringConverter;
        static AttributeVisitCoercer<float> floatConverter;
        static AttributeVisitCoercer<int> intConverter;
        static AttributeVisitCoercer<bool> boolConverter;
    };

    struct InputStorageSpace
    {
        std::vector<char> buffer;
        size_t bufferSize = 0;
        InputTypeConfig& inputConfig;
        operator std::string() const
        {
            auto len = std::strlen(pointer());
            return std::string(pointer(), len);
        }
        char* pointer()
        {
            return buffer.data();
        }
        const char* pointer() const
        {
            return buffer.data();
        }
        InputStorageSpace() = delete;
        InputStorageSpace(InputTypeConfig& inputConfig, size_t bufferSize)
            : inputConfig(inputConfig)
            , bufferSize(bufferSize)
        {
        }
        InputStorageSpace& operator=(InputStorageSpace other);

        ~InputStorageSpace()
        {
            buffer.clear();
            bufferSize = 0;
        }

        void Update(int sourceIndex)
        {
            auto current = inputConfig.ToString(sourceIndex);
            buffer.clear();
            std::copy(current.begin(), current.end(), std::back_inserter(buffer));
            buffer.resize(bufferSize);
        }

        void Render(const std::string& label, int fromStorageIndex, const std::string& variable);
    };

    std::vector<InputStorageSpace> m_Storages;
    std::unique_ptr<InputTypeConfig> m_InputTypeConfig;
};

template< class T >
struct is_int_or_float
     : std::integral_constant<
         bool,
         std::is_same<float, typename std::remove_cv<T>::type>::value ||
         std::is_same<int, typename std::remove_cv<T>::type>::value
     > {};

template <
        typename IntFloat,
        typename = typename std::enable_if_t<is_int_or_float<IntFloat>::value>
    >
class SliderEntityEditorWidget : public EntityEditorWidget
{
public:
    SliderEntityEditorWidget(size_t entityId, const attributeRow_T& attributes);
    void Render() override
    {
        auto variableKey = std::get<std::string>(GetAttributes()[1]);
        m_Storage = std::get<IntFloat>(GetEntity()->getAttribute(variableKey));

        auto minValue = std::get<IntFloat>(GetAttributes()[3]);
        auto maxValue = std::get<IntFloat>(GetAttributes()[4]);
        auto stepValue = std::get<IntFloat>(GetAttributes()[5]);

        auto workValue = m_Storage;

        ImGuiSliderAnyImpl("", &workValue, minValue, maxValue, stepValue);

        if (workValue != m_Storage)
        {
            GetEntity()->setAttribute(variableKey, workValue);
            m_Storage = workValue;
        }
    }

private:
    void ImGuiSliderAnyImpl(const char* label, IntFloat *val, IntFloat min, IntFloat max, IntFloat step);
    IntFloat m_Storage;
};

class CheckboxEntityEditorWidget : public EntityEditorWidget
{
public:
    CheckboxEntityEditorWidget(size_t entityId, const attributeRow_T& attributes);
    void Render() override;

private:
    bool m_Storage;
};

class DummyEntityEditorWidget : public EntityEditorWidget
{
public:
    DummyEntityEditorWidget(size_t entityId, const attributeRow_T& attributes);
    void Render() override;

private:
    std::string m_Storage;
};

class WidgetLabelPair : public EditorRenderable
{
public:
    template <
        class EntityEditorWidget_T,
        typename = std::is_base_of<EntityEditorWidget, EntityEditorWidget_T>
    >
    WidgetLabelPair(const std::string& label, std::unique_ptr<EntityEditorWidget_T> widget)
        : m_Label(label)
        , m_Widget(std::move(widget))
    {
    }

    void Render() override;

private:
    std::string m_Label;
    std::unique_ptr<EntityEditorWidget> m_Widget;
};

class EditorWidgetGroup : public EditorRenderable
{
public:
    explicit EditorWidgetGroup(size_t entityId, const attributeList_T& attributeList);

    void Render() override;

private:
    void AppendWidget(size_t entityId, const attributeRow_T& attributes);

    std::string m_Label;
    bool m_OpenState = true;
    std::vector<std::unique_ptr<EditorRenderable> > m_Widgets;
};

class EntityEditorMenu
{
public:
    explicit EntityEditorMenu(size_t entityId);

    void Render();

    void Hide();
    void Show();
    Entity* GetEntity() const;
    bool HasEntity(Entity* entity) const;

private:
    //Listen to global event EventEntityRemovedFromMap, if == m_Entity -> destroy
    size_t m_EntityID;
    bool m_Open = true;
    float m_Width = 400;
    int m_AnimSlideInDuration = 500;
    int m_AnimSlideOutDuration = 500;

    int m_AnimSlideInStart = 0;
    int m_AnimSlideOutStart = 0;

    attributeTree_T m_AttributeTree;
    std::vector<std::unique_ptr<EditorRenderable> > m_Widgets;
};


