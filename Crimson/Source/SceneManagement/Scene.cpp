#include "Scene.h"

#include "Entity.h"
#include "Components.h"

#include "Renderer/RenderTarget.h"

#include "Input.h"

#include <filesystem>

namespace Crimson {
	static void PhysicsComponentDestroy(entt::registry& r, entt::entity ent) {
		delete r.get<PhysicsComponent>(ent).context;
	}

	void Scene::PhysicsComponentCreate(entt::registry& r, entt::entity ent) {
		if (!r.has<TransformComponent>(ent)) {
			CR_LOG_FATAL_ERROR("%s", "PhysicsComponent requires the entity to have a transform");
			abort();
		}

		auto& phys = r.get<PhysicsComponent>(ent);

		auto& trans = r.get<TransformComponent>(ent);

		phys.context = new Rigidbody(m_physicsScene.get(), trans.position, trans.rotation);

		if (r.has<SphereColliderComponent>(ent)) {
			phys.context->AddSphereCollider(r.get<SphereColliderComponent>(ent).radius);
		} else if (r.has<BoxColliderComponent>(ent)) {
			phys.context->AddBoxCollider(r.get<BoxColliderComponent>(ent).extents);
		} else {
			CR_LOG_WARNING("%s", "PhysicsComponent requires the entity to have a collider");
		}

		phys.context->EnableGravity(phys.useGravity);
		phys.context->SetMass(phys.mass);
		phys.context->SetCOG(phys.cog);
		phys.context->SetBounciness(phys.bounciness);
		phys.context->SetFriction(phys.friction);
		phys.context->SetKinematic(phys.isKinematic);
	}

	void Scene::ScriptComponentCreate(entt::registry& r, entt::entity ent) {
		m_scriptManager->SetupEntity(ent, this);
	}

	void Scene::ScriptComponentDestroy(entt::registry& r, entt::entity ent) {
		m_scriptManager->DeInitScript(r.get<ScriptComponent>(ent).id);
	}

	Scene::Scene(bool release) : m_assetManager(release) {
		m_lightScene = std::make_shared<LightScene>();
		m_physicsScene = std::make_shared<PhysicsScene>(this);
		m_scriptManager = std::make_shared<ScriptManager>();

		Input::Init();
		Input::LoadConfig(m_assetManager.LoadText("Data/InputConfig.conf").c_str());

		m_registry.on_construct<PhysicsComponent>().connect<&Scene::PhysicsComponentCreate>(this);
		m_registry.on_destroy<PhysicsComponent>().connect<&PhysicsComponentDestroy>();
		m_registry.on_destroy<ScriptComponent>().connect<&Scene::ScriptComponentDestroy>(this);
	}

	void Scene::LoadSkybox() {
		m_skybox = std::make_shared<Skybox>(
			std::vector{
				m_assetManager.LoadSurface(m_config.skyboxPosX),
				m_assetManager.LoadSurface(m_config.skyboxNegX),
				m_assetManager.LoadSurface(m_config.skyboxPosY),
				m_assetManager.LoadSurface(m_config.skyboxNegY),
				m_assetManager.LoadSurface(m_config.skyboxPosZ),
				m_assetManager.LoadSurface(m_config.skyboxNegZ),
			}
		);
	}

	Scene::~Scene() {
		auto view = m_registry.view<PhysicsComponent>();
		for (auto ent : view) {
			PhysicsComponent physics = view.get<PhysicsComponent>(ent);
			delete physics.context;
		}
	}


	void Scene::Init() {
		for (auto& p : m_assetManager.GetFilesFromDir("Data")) {
			if (p.second == ".as") {
				m_scriptManager->AddScript(m_assetManager.LoadText(p.first));
			}
		}

		m_scriptManager->Compile(m_assetManager);

		auto view = m_registry.view<ScriptComponent>();
		for (auto ent : view) {
			auto script = view.get<ScriptComponent>(ent);

			m_scriptManager->SetupEntity(ent, this);
		}

		m_scriptManager->Init();
	}

	void Scene::PhysicsUpdate(float delta) {
		m_scriptManager->PhysicsUpdate(delta);

		auto view = m_registry.view<TransformComponent, PhysicsComponent>();
		for (auto ent : view) {
			auto [transform, physics] = view.get<TransformComponent, PhysicsComponent>(ent);

			auto b = physics.context->m_body;

			if (b->isGravityEnabled() != physics.useGravity) {
				physics.context->EnableGravity(physics.useGravity);
			}
			if (physics.context->m_isKinematic != physics.isKinematic) {
				physics.context->SetKinematic(physics.isKinematic);
			}
			if (b->getMass() != physics.mass) {
				physics.context->SetMass(physics.mass);
			}
			if (physics.context->GetCOG() != physics.cog) {
				physics.context->SetCOG(physics.cog);
			}
			if (physics.context->GetBounciness() != physics.friction) {
				physics.context->SetBounciness(physics.bounciness);
			}
			if (physics.context->GetFriction() != physics.friction) {
				physics.context->SetFriction(physics.friction);
			}

			if (physics.isKinematic) {
				physics.context->SetMass(0.0f);
			}

			transform.position = physics.context->GetPosition();
			transform.rotation = physics.context->GetRotation();
		}
	}

	void Scene::Update(float delta) {
		m_scriptManager->Update(delta);
		m_physicsScene->Update(delta);
	}

	void Scene::Render(RenderTarget& renderTarget) {
		Camera* mainCamera = GetMainCamera();

		if (mainCamera) {
			ApplyLighting();
			RenderShadows(mainCamera);
			renderTarget.Bind();
			RenderMeshes(mainCamera);
			renderTarget.Unbind();
		}
	}

	void Scene::Render() {
		Camera* mainCamera = GetMainCamera();

		if (mainCamera) {
			ApplyLighting();
			RenderShadows(mainCamera);
			RenderMeshes(mainCamera);
		}
	}

	void Scene::Render(RenderTarget& renderTarget, Camera* camera) {
		ApplyLighting();
		RenderShadows(camera);
		renderTarget.Bind();
		RenderMeshes(camera);
		renderTarget.Unbind();
	}

	Camera* Scene::GetMainCamera() {
		Camera* mainCamera = NULL;
		{
			auto view = m_registry.view<TransformComponent, CameraComponent>();
			for (auto ent : view) {
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(ent);

				camera.camera.position = transform.position;
				camera.camera.rotation = glm::degrees(glm::eulerAngles(transform.rotation));

				if (camera.active) {
					mainCamera = &camera.camera;
					break;
				}
			}
		}

		return mainCamera;
	}

	void Scene::ApplyLighting() {
		m_lightScene->m_ambientLights.clear();
		m_lightScene->m_directionalLights.clear();
		m_lightScene->m_pointLights.clear();
		{
			auto view = m_registry.view<TransformComponent, AmbientLightComponent>();
			for (auto ent : view) {
				auto [t, l] = view.get<TransformComponent, AmbientLightComponent>(ent);

				m_lightScene->m_ambientLights.push_back(AmbientLight{l.color, l.intensity});
			}
		}
		{
			auto view = m_registry.view<TransformComponent, DirectionalLightComponent>();
			for (auto ent : view) {
				auto [t, l] = view.get<TransformComponent, DirectionalLightComponent>(ent);

				m_lightScene->m_directionalLights.push_back(DirectionalLight{glm::degrees(glm::eulerAngles(t.rotation)), l.color, l.intensity});
			}
		}
		{
			auto view = m_registry.view<TransformComponent, PointLightComponent>();
			for (auto ent : view) {
				auto [t, l] = view.get<TransformComponent, PointLightComponent>(ent);

				m_lightScene->m_pointLights.push_back(PointLight{t.position, l.constant, l.linear, l.quadratic, l.color, l.intensity});
			}
		}
	}

	void Scene::RenderShadows(Camera* mainCamera) {
		std::vector<Mesh*> meshes;
		std::vector<glm::mat4> transforms;
		auto view = m_registry.view<TransformComponent, MeshFilterComponent, MaterialComponent>();
		for (auto ent : view) {
			auto [transform, mesh, material] = view.get<TransformComponent, MeshFilterComponent, MaterialComponent>(ent);

			meshes.push_back(m_assetManager.LoadMesh(mesh.path));
			transforms.push_back(transform.GetTransform());

			Renderer::ShadowPass(*mainCamera, *m_lightScene, transforms, meshes);
		}
	}

	void Scene::RenderMeshes(Camera* mainCamera) {
		if (m_skybox) {
			m_skybox->Draw(*mainCamera);
		}

		auto view = m_registry.view<TransformComponent, MeshFilterComponent, MaterialComponent>();
		for (auto ent : view) {
			auto [transform, mesh, material] = view.get<TransformComponent, MeshFilterComponent, MaterialComponent>(ent);

			Renderer::ShaderPass(*mainCamera, *m_lightScene, transform.GetTransform(), *m_assetManager.LoadMaterial(material.path));
			Renderer::Draw(*m_assetManager.LoadMesh(mesh.path));
		}
	}

	void Scene::UpdateViewport(std::pair<int, int> size) {
		auto view = m_registry.view<TransformComponent, CameraComponent>();
		for (auto ent : view) {
			auto [transform, camera] = view.get<TransformComponent, CameraComponent>(ent);

			camera.camera.UpdateProjection(size);
		}
	}

	Entity Scene::CreateEntity(const std::string& name, const std::string& tag, const GUID& guid) {
		Entity ent(m_registry.create(), this);

		GUID g;

		if (guid.empty()) {
			g = GenerateGUID();
		} else {
			g = guid;
		}

		ent.AddComponent<TransformComponent>().name = name;
		ent.GetComponent<TransformComponent>().tag = tag;
		ent.GetComponent<TransformComponent>().guid = g;
		return ent;
	}

	Entity Scene::GetEntityByGUID(const GUID& guid) {
		auto view = m_registry.view<TransformComponent>();
		for (auto ent : view) {
			Entity e(ent, this);
			if (e.GetComponent<TransformComponent>().guid == guid) {
				return e;
			}
		}
		return Entity();
	}

	void Scene::DestroyEntity(Entity ent) {
		if (ent.GetComponent<TransformComponent>().parent) {
			auto& children = ent.GetComponent<TransformComponent>().parent.GetComponent<TransformComponent>().children;
			children.erase(std::remove(children.begin(), children.end(), ent), children.end());
		}

		for (auto child : ent.GetComponent<TransformComponent>().children) {
			child.GetComponent<TransformComponent>().parent = Entity();
			DestroyEntity(child);
		}
		ent.GetComponent<TransformComponent>().children.clear();

		m_registry.destroy(ent.m_entityHandle);
	}

	void Scene::ContactStay(rp3d::CollisionBody* body, rp3d::CollisionBody* other) {
		auto view = m_registry.view<TransformComponent, PhysicsComponent>();

		Entity currentEnt, otherEnt;

		for (auto ent : view) {
			auto [transform, physics] = view.get<TransformComponent, PhysicsComponent>(ent);

			if (physics.context->m_body == body) {
				currentEnt = Entity(ent, this);
			} else if (physics.context->m_body == other) {
				otherEnt = Entity(ent, this);
			}
		}

		if (currentEnt && currentEnt.HasComponent<ScriptComponent>()) {
			m_scriptManager->ContactStay(currentEnt.GetComponent<ScriptComponent>().id, otherEnt);
		}
		if (otherEnt && otherEnt.HasComponent<ScriptComponent>()) {
			m_scriptManager->ContactStay(otherEnt.GetComponent<ScriptComponent>().id, currentEnt);
		}
	}

	void Scene::ContactEnter(rp3d::CollisionBody* body, rp3d::CollisionBody* other) {
		auto view = m_registry.view<TransformComponent, PhysicsComponent>();

		Entity currentEnt, otherEnt;

		for (auto ent : view) {
			auto [transform, physics] = view.get<TransformComponent, PhysicsComponent>(ent);

			if (physics.context->m_body == body) {
				currentEnt = Entity(ent, this);
			} else if (physics.context->m_body == other) {
				otherEnt = Entity(ent, this);
			}
		}

		if (currentEnt && currentEnt.HasComponent<ScriptComponent>()) {
			m_scriptManager->ContactEnter(currentEnt.GetComponent<ScriptComponent>().id, otherEnt);
		}
		if (otherEnt && otherEnt.HasComponent<ScriptComponent>()) {
			m_scriptManager->ContactEnter(otherEnt.GetComponent<ScriptComponent>().id, currentEnt);
		}
	}

	void Scene::ContactExit(rp3d::CollisionBody* body, rp3d::CollisionBody* other) {
		auto view = m_registry.view<TransformComponent, PhysicsComponent>();

		Entity currentEnt, otherEnt;

		for (auto ent : view) {
			auto [transform, physics] = view.get<TransformComponent, PhysicsComponent>(ent);

			if (physics.context->m_body == body) {
				currentEnt = Entity(ent, this);
			} else if (physics.context->m_body == other) {
				otherEnt = Entity(ent, this);
			}
		}

		if (currentEnt && currentEnt.HasComponent<ScriptComponent>()) {
			m_scriptManager->ContactExit(currentEnt.GetComponent<ScriptComponent>().id, otherEnt);
		}
		if (otherEnt && otherEnt.HasComponent<ScriptComponent>()) {
			m_scriptManager->ContactExit(otherEnt.GetComponent<ScriptComponent>().id, currentEnt);
		}
	}

	float Scene::GetScriptEngineUpdateTime() {
		return m_scriptManager->GetUpdateTime();
	}

	float Scene::GetPhysicsEngineUpdateTime() {
		return m_physicsScene->GetUpdateTime();
	}
}
