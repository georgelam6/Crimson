#ifndef GUI_H
#define GUI_H

#include <SDL2/SDL.h>
#include <Crimson.h>

class GUI {
private:
   EntityHandle m_selectedEntity = 0;
public:
   GUI(){}
   GUI(SDL_Window* window, const SDL_GLContext glContext);
   void Init(SDL_Window* window, const SDL_GLContext glContext);
   void Render(SDL_Window* window, ECS& ecs, Crimson::SceneManager& sceneManager, Crimson::Camera& camera);
   void Update(const SDL_Event& event);
   ~GUI();
};

#endif /* end of include guard: GUI_H */
