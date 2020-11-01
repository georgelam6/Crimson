#include "Crimson"

class Monkey : CrimsonBehaviour {
	private float m_speed = 3.0f;

	void OnInit() {
		print("Monkey says hello");
	}

	void OnUpdate(float delta) {
		if (Input::GetButton("up").pressed) {
			m_entity.GetTransformComponent().position.y += m_speed * delta;
		} else if (Input::GetButton("down").pressed) {
			m_entity.GetTransformComponent().position.y -= m_speed * delta;
		}

		if (Input::GetButton("right").pressed) {
			m_entity.GetTransformComponent().position.x += m_speed * delta;
		} else if (Input::GetButton("left").pressed) {
			m_entity.GetTransformComponent().position.x -= m_speed * delta;
		}
	}

	void Collision() {
		print("hi from angelscript");
	}
}
