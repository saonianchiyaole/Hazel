workspace "Hazel"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputDir = "%{cfg.buildcfg}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "vendor/GLFW/include"
IncludeDir["GLAD"] = "vendor/GLAD/include"
IncludeDir["imgui"] = "vendor/imgui"
IncludeDir["glm"] = "vendor/glm"
IncludeDir["stb_image"] = "vendor/stb_image"
IncludeDir["entt"] = "vendor/entt"
IncludeDir["yaml"] = "vendor/yaml-cpp/include"
IncludeDir["ImGuizmo"] = "vendor/ImGuizmo"
IncludeDir["box2d"] = "vendor/box2d/include"
IncludeDir["mono"] = "vendor/mono/include"
IncludeDir["assimp"] = "vendor/assimp/include"

SolutionDir = "G:/Project/visualStudio/Hazel"


-- Initialize LibDir
LibDir = {} 
LibDir["mono"] = "%{SolutionDir}/vendor/mono/lib/%{cfg.buildcfg}"

Lib = {}
Lib["mono"] = "%{LibDir.mono}/mono-2.0-sgen.lib"
Lib["WinSock"] = "Ws2_32.lib"
Lib["WinMM"] = "Winmm.lib"
Lib["WinVersion"] = "Version.lib"
Lib["BCrypt"] = "Bcrypt.lib"
Lib["Ucrt"] = "libucrt.lib"
Lib["VCruntime"] = "libvcruntime.lib"
Lib["assimpd"] = "%{SolutionDir}/vendor/assimp/lib/Debug/assimp-vc143-mtd.lib"
Lib["assimp"] = "%{SolutionDir}/vendor/assimp/lib/Release/assimp-vc143-mt.lib"



include "vendor/GLFW"
include "vendor/GLAD"
include "vendor/imgui"
include "vendor/yaml-cpp"
include "vendor/box2d"
include "ScriptCore"


project "Hazel"
	location "Hazel"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("$(SolutionDir)bin/" .. outputDir .. "/%{prj.name}")
	objdir ("$(SolutionDir)bin-int/" .. outputDir .. "/%{prj.name}")

	pchheader "hzpch.h"
	pchsource "Hazel/src/hzpch.cpp"

	files
	{
		"Hazel/src/**.h",
		"Hazel/src/**.cpp",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/ImGuizmo/ImGuizmo.h",
		"vendor/ImGuizmo/ImGuizmo.cpp",
	}

	includedirs
	{
		"$(SolutionDir)Hazel/src",
		"$(SolutionDir)vendor/spdlog/include",
		"$(SolutionDir)vendor/GLFW/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.box2d}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.assimp}"
	}

	libdirs
	{
		"%{LibDir.mono}",
	}

	links
	{
		"GLFW",
		"GLAD",
		"imgui",
		"opengl32.lib",
		"yaml-cpp",
		"box2d",
		"%{Lib.mono}",
	}

	filter "files:vendor/ImGuizmo/**.cpp"
	flags { "NoPCH" }

	filter "system:windows"
		staticruntime "On"
		systemversion "latest"

		links
		{
			"%{Lib.WinSock}",
			"%{Lib.WinMM}",
			"%{Lib.WinVersion}",
			"%{Lib.BCrypt}",
		}

		defines
		{
			"HZ_PLATFORM_WINDOWS",
			"HZ_BUILD_DLL",
			"HZ_ENABLE_ASSERTS",
			"GLFW_INCLUDE_NONE",
			"YAML_CPP_STATIC_DEFINE"
		}


		postbuildcommands {
			"xcopy /Y /F \"$(TargetDir)$(TargetName).lib\" \"$(SolutionDir)bin\\$(Configuration)-$(LlvmPlatformName)\\SandBox\\\""
		}
		

	filter "configurations:Debug"
		defines "HZ_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			"%{Lib.assimpd}"
		}

	filter "configurations:Release"
		defines "HZ_RELEASE"
		runtime "Release"
		optimize "on"

		links
		{
			"%{Lib.assimp}"
		}

	filter "configurations:Dist"
		defines "HZ_DIST"
		runtime "Release"
		optimize "on"


project "SandBox"

	location "SandBox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("$(SolutionDir)bin/" .. outputDir .. "/%{prj.name}")
	objdir ("$(SolutionDir)bin-int/" .. outputDir .. "/%{prj.name}")

	files
	{
		"SandBox/src/**.h",
		"SandBox/src/**.cpp"
	}

	includedirs
	{
		"$(SolutionDir)vendor/spdlog/include",
		"$(SolutionDir)Hazel/src/",
		"%{IncludeDir.glm}",
		"$(SolutionDir)vendor/imgui",
		"$(SolutionDir)vendor/GLAD/include"
	}


	links
	{
		"Hazel"
	}

	filter "system:windows"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"HZ_PLATFORM_WINDOWS"
		}


	filter "configurations:Debug"
		defines "HZ_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "HZ_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "HZ_DIST"
		runtime "Release"
		optimize "on"


project "Editor"
		location "Editor"
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++17"
		staticruntime "off"
	
		targetdir ("$(SolutionDir)bin/" .. outputDir .. "/%{prj.name}")
		objdir ("$(SolutionDir)bin-int/" .. outputDir .. "/%{prj.name}")
	
		files
		{
			"Editor/src/**.h",
			"Editor/src/**.cpp"
		}
	
		includedirs
		{
			"$(SolutionDir)Editor/src",
			"$(SolutionDir)vendor/spdlog/include",
			"$(SolutionDir)Hazel/src/",
			"%{IncludeDir.glm}",
			"%{IncludeDir.GLFW}",
			"$(SolutionDir)vendor/imgui",
			"$(SolutionDir)vendor/GLAD/include",
			"%{IncludeDir.entt}",
			"%{IncludeDir.yaml}",
			"%{IncludeDir.ImGuizmo}",
			"%{IncludeDir.box2d}",
			"%{IncludeDir.mono}",
		}
	
	
		links
		{
			"Hazel",
			"%{Lib.mono}",
			"%{Lib.WinSock}",
			"%{Lib.WinMM}",
			"%{Lib.WinVersion}",
			"%{Lib.BCrypt}",
		}
		
		filter "system:windows"
			staticruntime "On"
			systemversion "latest"
	
			defines
			{
				"HZ_PLATFORM_WINDOWS"
			}
	
	
		filter "configurations:Debug"
			defines "HZ_DEBUG"
			runtime "Debug"
			symbols "on"
	
		filter "configurations:Release"
			defines "HZ_RELEASE"
			runtime "Release"
			optimize "on"
	
		filter "configurations:Dist"
			defines "HZ_DIST"
			runtime "Release"
			optimize "on"