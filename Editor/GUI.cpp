#include "GUI.h"

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Utils/ImGuizmo.h>

GUI::GUI(SDL_Window* window, const SDL_GLContext glContext) {
   Init(window, glContext);
}

void GUI::Update(const SDL_Event& event) {
   ImGui_ImplSDL2_ProcessEvent(&event);
}


void GUI::Init(SDL_Window* window, const SDL_GLContext glContext) {
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO& io = ImGui::GetIO(); (void)io;
   io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
   io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

   ImGuiStyle& style = ImGui::GetStyle();
   if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
   {
     style.WindowRounding = 0.0f;
     style.Colors[ImGuiCol_WindowBg].w = 1.0f;
   }

   ImGui_ImplSDL2_InitForOpenGL(window, glContext);
   ImGui_ImplOpenGL3_Init("#version 330 core");
}

void GUI::Render(SDL_Window* window, ECS& ecs, Crimson::SceneManager& sceneManager, Crimson::Camera& camera) {
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplSDL2_NewFrame(window);
   ImGui::NewFrame();
   ImGuizmo::BeginFrame();

   ImGui::DockSpaceOverViewport(NULL, ImGuiDockNodeFlags_PassthruCentralNode);

   /* Scene Hierarchy */
   ImGui::Begin("Hierarchy");
   std::vector<EntityHandle> ents = sceneManager.GetEntities();

   static ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
   static glm::mat4 currentGizmoMatrix = Crimson::GetModelFromTransform(*ecs.GetComponent<Crimson::Transform>(m_selectedEntity));
   for (unsigned int i = 0; i < ents.size(); i++) {
      ImGuiTreeNodeFlags flags = baseFlags;
      if (ents[i] == m_selectedEntity) {
         flags |= ImGuiTreeNodeFlags_Selected;
      }
      if (ImGui::TreeNodeEx((void*)(EntityHandle)ents[i], flags, "%s", ecs.GetComponent<Crimson::Transform>(ents[i])->name.c_str())) {
         ImGui::TreePop();
      }
      if (ImGui::IsItemClicked()) {
         m_selectedEntity = ents[i];
         currentGizmoMatrix = Crimson::GetModelFromTransform(*ecs.GetComponent<Crimson::Transform>(m_selectedEntity));
      }

   }
   ImGui::End();

   ImGuizmo::Enable(true);

   ImGui::Begin("Inspector");
   static bool useSnap(false);
   static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
	static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);

   ImGui::Text("Gizmo mode"); ImGui::SameLine();
   if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
		mCurrentGizmoMode = ImGuizmo::WORLD;
	ImGui::SameLine();
	if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
		mCurrentGizmoMode = ImGuizmo::LOCAL;

	if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
		mCurrentGizmoOperation = ImGuizmo::SCALE;


   float matrixTranslation[3] = {ecs.GetComponent<Crimson::Transform>(m_selectedEntity)->position.x,ecs.GetComponent<Crimson::Transform>(m_selectedEntity)->position.y,ecs.GetComponent<Crimson::Transform>(m_selectedEntity)->position.z};
   float matrixRotation[3] = {ecs.GetComponent<Crimson::Transform>(m_selectedEntity)->rotation.x,ecs.GetComponent<Crimson::Transform>(m_selectedEntity)->rotation.y,ecs.GetComponent<Crimson::Transform>(m_selectedEntity)->rotation.z};
   float matrixScale[3] = {ecs.GetComponent<Crimson::Transform>(m_selectedEntity)->scale.x,ecs.GetComponent<Crimson::Transform>(m_selectedEntity)->scale.y,ecs.GetComponent<Crimson::Transform>(m_selectedEntity)->scale.z};
	ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(currentGizmoMatrix), matrixTranslation, matrixRotation, matrixScale);
   if (ImGui::CollapsingHeader("Transform")) {
	  ImGui::InputFloat3("Position", matrixTranslation, 3);
	  ImGui::InputFloat3("Rotation", matrixRotation, 3);
	  ImGui::InputFloat3("Scale", matrixScale, 3);
   }
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, glm::value_ptr(currentGizmoMatrix));
   ecs.GetComponent<Crimson::Transform>(m_selectedEntity)->position = glm::vec3(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2]);
   ecs.GetComponent<Crimson::Transform>(m_selectedEntity)->rotation = glm::vec3(glm::radians(matrixRotation[0]), glm::radians(matrixRotation[1]), glm::radians(matrixRotation[2]));
   ecs.GetComponent<Crimson::Transform>(m_selectedEntity)->scale = glm::vec3(matrixScale[0], matrixScale[1], matrixScale[2]);
   ImGui::End();

   ImGuiIO& io = ImGui::GetIO(); (void)io;
   int x, y, w, h;
   SDL_GetWindowSize(window, &w, &h);
   SDL_GetWindowPosition(window, &x, &y);
	ImGuizmo::SetRect(x, y, w, h);
	ImGuizmo::Manipulate(glm::value_ptr(camera.GetView()), glm::value_ptr(camera.GetProjection()), mCurrentGizmoOperation, mCurrentGizmoMode, glm::value_ptr(currentGizmoMatrix));


   if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
         if (ImGui::MenuItem("Save")) {
            sceneManager.Serialize("Resources/TestMap.txt", ecs);
         }
         ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
   }

   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

   if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
      SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
   }
}

GUI::~GUI() {
   ImGui_ImplOpenGL3_Shutdown();
   ImGui_ImplSDL2_Shutdown();
   ImGui::DestroyContext();
}