#pragma once

/* Crimson main header - to be included by Crimson applications */

/* Basic */
#include "logger.h"
#include "application.h"
#include "assets.h"
#include "memory.h"

/* Math */
#include "math/vec2.h"
#include "math/vec3.h"
#include "math/vec4.h"
#include "math/mat4.h"

/* Renderer */
#include "rendering/renderer.h"
#include "rendering/shader.h"
#include "rendering/window.h"
#include "rendering/mesh.h"
#include "rendering/meshfactory.h"
#include "rendering/model.h"
#include "rendering/camera.h"
#include "rendering/phongmaterial.h"
#include "rendering/rendertarget.h"

/* Scene Management */
#include "entity/entity.h"
#include "entity/scene.h"
#include "entity/components/renderable.h"
#include "entity/components/transform.h"
#include "entity/components/lights.h"
#include "entity/components/script.h"

/* Dear ImGui */
#include "imgui/imguimanager.h"
#include "imgui/gui.h"
#include "imgui/imguifontawesome.h"

/* Scripting */
#include "scripting/scriptmanager.h"

/* Input */
#include "input/keys.h"
#include "input/input.h"