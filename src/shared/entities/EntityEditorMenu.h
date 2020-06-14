#pragma once
#include "EntityFactory.h"
#include "engine/nui/nui.h"
#include <string>

class Entity;
struct nk_context;

class EditorRenderable
{
public:
    virtual void Render() = 0;

protected:
    nk_context* Context();
};

class EntityEditorWidget : public EditorRenderable
{
public:
    EntityEditorWidget(Entity* entity, const attributeRow_T& attributes)
        : m_Entity(entity)
        , m_Attributes(attributes)
    {
    }

protected:
    const Entity* GetEntity() const;
    Entity* GetEntity();
    const attributeRow_T& GetAttributes() const;

private:
    Entity* m_Entity;
    const attributeRow_T& m_Attributes;
};

class InputEntityEditorWidget : public EntityEditorWidget
{
public:
    InputEntityEditorWidget(Entity* entity, const attributeRow_T& attributes);
    void Render() override;

private:
    struct InputTypeConfig
    {
        using nk_filter_func_t = int (*)(const struct nk_text_edit*, nk_rune);
        attribute_T m_SourceVariable;

        std::string ToString(int fromStorageIndex = 0);
        void ToSource(const std::string& stringValue, int fromStorageIndex = 0);
        int StorageCount();
        explicit InputTypeConfig(const attribute_T variable);
        void Update(const attribute_T variable);
        nk_filter_func_t GetInputFilterer() const;


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

        nk_flags Render(nk_context* context, int fromStorageIndex, const std::string& variable);
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
    SliderEntityEditorWidget(Entity* entity, const attributeRow_T& attributes);
    void Render() override
    {
        auto variableKey = std::get<std::string>(GetAttributes()[1]);
        m_Storage = std::get<IntFloat>(GetEntity()->getAttribute(variableKey));

        auto minValue = std::get<IntFloat>(GetAttributes()[3]);
        auto maxValue = std::get<IntFloat>(GetAttributes()[4]);
        auto stepValue = std::get<IntFloat>(GetAttributes()[5]);

        auto workValue = m_Storage;

        auto bounds = nk_rect(0.5f, 0.0f, 0.5f, 1.0f);
        nk_layout_space_push(Context(), bounds);

        nk_slider_any(Context(), minValue, &workValue, maxValue, stepValue);

        if (workValue != m_Storage)
        {
            GetEntity()->setAttribute(variableKey, workValue);
            m_Storage = workValue;
        }
    }

private:
    int (*nk_slider_any)(struct nk_context*, IntFloat min, IntFloat *val, IntFloat max, IntFloat step);
    IntFloat m_Storage;
};

class CheckboxEntityEditorWidget : public EntityEditorWidget
{
public:
    CheckboxEntityEditorWidget(Entity* entity, const attributeRow_T& attributes);
    void Render() override;

private:
    int m_Storage;
};

class DummyEntityEditorWidget : public EntityEditorWidget
{
public:
    DummyEntityEditorWidget(Entity* entity, const attributeRow_T& attributes);
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
    explicit EditorWidgetGroup(Entity* entity, const attributeList_T& attributeList);

    void Render() override;

private:
    void AppendWidget(Entity* entity, const attributeRow_T& attributes);

    std::string m_Label;
    int m_OpenState = 1;
    std::vector<std::unique_ptr<EditorRenderable> > m_Widgets;
};

class EntityEditorMenu
{
public:
    explicit EntityEditorMenu(Entity* entity);

    void Render();

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

    const attributeTree_T m_AttributeTree;
    std::vector<std::unique_ptr<EditorRenderable> > m_Widgets;
};


