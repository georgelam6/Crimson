#include <vector>
#include <string>
#include <sstream>
#include <string.h>

#include "assets.h"
#include "model.h"
#include "logger.h"
#include "renderdata.h"
#include "entity/components/lights.h"
#include "entity/components/transform.h"
#include "math/vec3.h"
#include "math/vec2.h"

namespace Crimson {
	Model::Model() {

	}

	Model::~Model() {

	}

	void Model::Draw(const Camera& camera) {
		for (const ref<Mesh>& mesh : m_meshes) {
			ref<Shader> s = AssetManager::LoadShader(mesh->m_material->m_shader.c_str());
			s->Bind();

			s->SetUniformVec3("u_cameraPos", camera.position);
			s->SetUniformInt("u_pointLightCount", 1);
			s->SetUniformVec3("u_pointLights[0].position", vec3(2.0f, 0.0f, 5.0f));
			s->SetUniformVec3("u_pointLights[0].color", vec3(1.0f, 1.0f, 1.0f));
			s->SetUniformFloat("u_pointLights[0].intensity", 5.0f);

			s->SetUniformMat4("u_model", m_transform);
			s->SetUniformMat4("u_view", camera.GetView());
			s->SetUniformMat4("u_projection", camera.projection);
			s->SetUniformVec3("u_cameraPos", camera.position);

			mesh->Draw();
		}
	}

	void Model::DrawLitScene(const Camera& camera, Scene* scene) {
		for (const ref<Mesh>& mesh : m_meshes) {
			ref<Shader> s = AssetManager::LoadShader(mesh->m_material->m_shader.c_str());
			s->Bind();

			s->SetUniformVec3("u_cameraPosition", camera.position);

			int pointLightCount = 0;
			int skyLightCount = 0;

			for (unsigned int i = 0; i < scene->GetLights()->size(); i++) {
				Entity* ent = scene->GetLights()->at(i);

				if (!ent->HasComponent<TransformComponent>()) { continue; }
				TransformComponent* tc = ent->GetComponent<TransformComponent>();
				mat4 tmat = tc->GetMatrix();

				if (ent->HasComponent<PointLightComponent>()) {
					pointLightCount++;
					PointLightComponent* plc = ent->GetComponent<PointLightComponent>();

					s->SetUniformVec3(("u_pointLights[" + std::to_string(i) + "].position").c_str(), tmat.GetPosition());
					s->SetUniformVec3(("u_pointLights[" + std::to_string(i) + "].color").c_str(), plc->color);
					s->SetUniformFloat(("u_pointLights[" + std::to_string(i) + "].intensity").c_str(), plc->intensity);
					s->SetUniformFloat(("u_pointLights[" + std::to_string(i) + "].constant").c_str(), plc->constant);
					s->SetUniformFloat(("u_pointLights[" + std::to_string(i) + "].linear").c_str(), plc->linear);
					s->SetUniformFloat(("u_pointLights[" + std::to_string(i) + "].quadratic").c_str(), plc->quadratic);
				}

				if (ent->HasComponent<SkyLightComponent>()) {
					skyLightCount++;
					SkyLightComponent* slc = ent->GetComponent<SkyLightComponent>();

					s->SetUniformVec3(("u_skyLights[" + std::to_string(i) + "].color").c_str(), slc->color);
					s->SetUniformFloat(("u_skyLights[" + std::to_string(i) + "].intensity").c_str(), slc->intensity);
				}
			}

			s->SetUniformInt("u_pointLightCount", pointLightCount);
			s->SetUniformInt("u_skyLightCount", skyLightCount);

			s->SetUniformMat4("u_model", m_transform);
			s->SetUniformMat4("u_view", camera.GetView());
			s->SetUniformMat4("u_projection", camera.projection);
			s->SetUniformVec3("u_cameraPos", camera.position);

			mesh->Draw();
		}
	}

	void Model::AddMesh(const ref<Mesh>& mesh) {
		m_meshes.push_back(mesh);
	}
}
