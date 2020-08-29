#ifndef MODELCOMP_H
#define MODELCOMP_H

#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Graphics/Model.h"
#include "Graphics/Camera.h"
#include "Graphics/Material.h"
#include "SceneManagement/SceneManager.h"
#include "SLECS.h"
#include "Transform.h"

namespace Crimson {
   struct ModelComponent {
      Texture texture;
      Shader shader;
      Model model;
      Material material;
   };

   static void RenderModels(ECS& ecs, Camera& camera, SceneManager& sceneManager) {
      for (EntityHandle ent : System<Transform, ModelComponent>(ecs)) {
         glm::mat4 model = GetModelFromTransform(*ecs.GetComponent<Transform>(ent));

         ecs.GetComponent<ModelComponent>(ent)->texture.Bind(0);
         ecs.GetComponent<ModelComponent>(ent)->shader.Bind();

         ecs.GetComponent<ModelComponent>(ent)->shader.SetUniform1i("tex", 0);
         ecs.GetComponent<ModelComponent>(ent)->shader.SetUniform3f("eyePosition", camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
         ecs.GetComponent<ModelComponent>(ent)->shader.SetUniformMatrix4("view", camera.GetViewProjection());
         ecs.GetComponent<ModelComponent>(ent)->shader.SetUniformMatrix4("modl", model);

         sceneManager.GetConfig()->directionalLight.UseLight(ecs.GetComponent<ModelComponent>(ent)->shader.GetUniformLocation("directionalLight.ambientIntensity"),
                                                             ecs.GetComponent<ModelComponent>(ent)->shader.GetUniformLocation("directionalLight.color"),
                                                             ecs.GetComponent<ModelComponent>(ent)->shader.GetUniformLocation("directionalLight.diffuseIntensity"),
                                                             ecs.GetComponent<ModelComponent>(ent)->shader.GetUniformLocation("directionalLight.direction"));

         ecs.GetComponent<ModelComponent>(ent)->material.UseMaterial(ecs.GetComponent<ModelComponent>(ent)->shader.GetUniformLocation("material.specularIntensity"),
                                                                     ecs.GetComponent<ModelComponent>(ent)->shader.GetUniformLocation("material.shininess"));

         ecs.GetComponent<ModelComponent>(ent)->model.Render();
      }
   }
}
#endif /* end of include guard: MODELCOMP_H */
