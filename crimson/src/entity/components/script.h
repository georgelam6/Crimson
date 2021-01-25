#pragma once

#include "core.h"
#include "entity/entity.h"
#include "scripting/scriptmanager.h"

namespace Crimson {
	/* Controls an AngelScript behaviour */
	class CR_API ScriptComponent : public Component {
	private:
		BehaviourInstance m_behaviour;

	public:
		std::string m_behaviourDecl;
		
		ScriptComponent(const char* behaviourDecl);

		void OnInit();
		void OnUpdate(float delta);
		void OnDestroy();
	};
}
