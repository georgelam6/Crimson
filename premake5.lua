require "premake-extensions/compiledb"

workspace "Crimson"
	architecture "x64"
	startproject "Game"

	configurations {
		"Debug",
		"Release"
	}

	flags {
		"MultiProcessorCompile"
	}

	defines {
		"GLFW_INCLUDE_NONE"
	}

	linkgroups "On"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Relative to Crimson
IncludeDir = {}
IncludeDir["GLFW"] = "Dependencies/glfw/include"
IncludeDir["Glad"] = "Dependencies/glad/include"
IncludeDir["PhysFS"] = "Dependencies/physfs/src"

ExternalDependencies = {}
ExternalDependencies["GLFW"] = {}
ExternalDependencies["OpenGL"] = {}
ExternalDependencies["Glad"] = {"Glad"}
ExternalDependencies["PhysFS"] = {"PhysFS"}

if os.host() == "linux" then
	ExternalDependencies["GLFW"] = {
		"pthread",
		"X11",
		"dl",
		"GLFW"
	}
	ExternalDependencies["OpenGL"] = {"GL"}
elseif os.host() == "windows" then
	ExternalDependencies["GLFW"] = {"GLFW"}
	ExternalDependencies["OpenGL"] = {"opengl32"}
end


group "Dependencies"
	include "Crimson/Dependencies/glfw"
	include "Crimson/Dependencies/glad"
	include "Crimson/Dependencies/physfs"
group ""

include "Crimson"
include "Game"
