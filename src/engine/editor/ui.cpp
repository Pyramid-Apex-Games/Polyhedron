#include "ui.h"
#include "engine/main/Application.h"
#include "engine/main/Window.h"
#include "engine/main/GLContext.h"
#include "engine/main/Input.h"
#include "engine/GLFeatures.h"
#include "engine/shader.h"
#include "shared/entities/EntityEditorMenu.h"

#include <imgui.h>
#include <examples/imgui_impl_sdl.h>
#include <examples/imgui_impl_opengl3.h>
#include <engine/scriptexport.h>

namespace {
    std::unique_ptr<EntityEditorMenu> ActiveEntityEditorMenu;
    bool m_WantTextInputChanged = false;

    bool show_demo_window = false;
}
extern bool editmode;

void EditorUI::Initialize()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(
        Application::Instance().GetWindow().GetWindowHandle(),
        Application::Instance().GetWindow().GetContext().GetContextHandle()
    );
    ImGui_ImplOpenGL3_Init(GetShaderVersionHeader().c_str());
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
}

void EditorUI::Update()
{
    bool wantTextInput = ImGui::GetIO().WantTextInput;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(
        Application::Instance().GetWindow().GetWindowHandle()
    );
    ImGui::NewFrame();

    if (wantTextInput != ImGui::GetIO().WantTextInput)
    {
        Application::Instance().GetInput().Text(ImGui::GetIO().WantTextInput);
        Application::Instance().GetConsole().Add("WantTextInput changed: " + std::string(ImGui::GetIO().WantTextInput ? "true" : "false") );
    }

    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    if (ActiveEntityEditorMenu)
    {
        ActiveEntityEditorMenu->Render();
    }
}

SCRIPTEXPORT void show_demo_ui()
{
    show_demo_window = true;
}

void EditorUI::Render()
{
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void EditorUI::InputProcessBegin()
{

}

EditorUI::InputEventProcessState EditorUI::InputEvent(const SDL_Event &evt)
{
    if (ImGui_ImplSDL2_ProcessEvent(&evt))
    {
        if (ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantCaptureMouse)
        {
            return InputEventProcessState::Handled;
        }
    }

    return InputEventProcessState::NotHandled;
}

void EditorUI::InputProcessEnd()
{

}

void EditorUI::Destroy()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}




void EditorUI::StartEntityEditor(size_t entityId)
{
    if (::editmode)
    {
        if (!ActiveEntityEditorMenu)
        {
            ActiveEntityEditorMenu = std::make_unique<EntityEditorMenu>(entityId);
        }
        else
        {
             if (ActiveEntityEditorMenu->GetEntity())
             {
                 ActiveEntityEditorMenu->Show();
             }
             else
             {
                 ActiveEntityEditorMenu = std::make_unique<EntityEditorMenu>(entityId);
             }
        }
    }
}

void EditorUI::StopEntityEditor(size_t entityId)
{
    if (ActiveEntityEditorMenu && ActiveEntityEditorMenu->GetEntity())
    {
        ActiveEntityEditorMenu->Hide();
    }
}

void EditorUI::Reset()
{
    ActiveEntityEditorMenu.release();
}