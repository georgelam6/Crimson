#include "entity.h"

namespace Crimson {
	ComponentID GetUniqueComponentID() noexcept {
		static ComponentID lastID{0u};
		return lastID++;
	}

	void Entity::Update(float delta) {
		for (auto& c : m_components) {
			c->OnUpdate(delta);
		}
	}

	void Entity::Draw(const Camera& camera, Shader* shader) {
		for (auto& c : m_components) {
			c->OnDraw(camera, shader);
		}
	}

	Entity::~Entity() {
		for (auto& c : m_components) {
			c->OnDestroy();
		}
	}
}