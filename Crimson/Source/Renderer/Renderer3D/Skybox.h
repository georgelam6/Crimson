#pragma once

#include <vector>
#include <string>
#include <memory>

#include "Renderer/Surface.h"
#include "Renderer/Shader.h"
#include "Camera.h"

namespace Crimson {
	class Skybox {
	private:
		unsigned int m_textureID;

		unsigned int m_va, m_vb;

		std::shared_ptr<Shader> m_shader;

		friend class Renderer;
	public:
		Skybox(const std::vector<Surface*>& skyboxTextures);

		void Load(const std::vector<Surface*>& skyboxTextures);

		void Draw(Camera& camera);
	};
}
