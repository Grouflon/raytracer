local cppCorePath = "../cpp-core/"

workspace "Raytracer"
	architecture "x86_64"
	configurations { "Debug", "Release" }
	location ("project/" .. _ACTION)
	debugdir "./"
	defines {
		"_CRT_SECURE_NO_WARNINGS"
	}

include(cppCorePath .. "premake5.lua")
project "cpp-core"
	targetdir "libs/%{cfg.buildcfg}"
	objdir "obj/%{cfg.buildcfg}"

project "Raytracer"
	dependson { "cpp-core" }
	kind "ConsoleApp"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	objdir "obj/%{cfg.buildcfg}"

	files {
		"src/**.h",
		"src/**.cpp",
		"src/**.c",
		"shaders/**.vs",
		"shaders/**.fs",

		cppCorePath .. "extern/gl3w/**.h",
		cppCorePath .. "extern/gl3w/**.c",

		cppCorePath .. "extern/imgui/imgui_impl_glfw_gl3.h",
		cppCorePath .. "extern/imgui/imgui_impl_glfw_gl3.cpp",
	}

	includedirs {
		"src/",
		cppCorePath .. "src/",
		cppCorePath .. "extern/glm/",
		cppCorePath .. "extern/imgui/",
		cppCorePath .. "extern/glfw-3.2.1/x64/include",
		cppCorePath .. "extern/gl3w/",
	}

	libdirs {
		cppCorePath .. "extern/glfw-3.2.1/x64/lib-vc2015/",
		os.findlib("opengl32"),
	}

	links {
		"cpp-core",
		"glfw3",
		"opengl32"
	}						

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"


