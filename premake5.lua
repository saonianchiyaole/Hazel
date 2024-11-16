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

include "vendor/GLFW"
include "vendor/GLAD"
include "vendor/imgui"

project "Hazel"
	location "Hazel"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

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
		"%{IncludeDir.entt}"
	}

	links
	{
		"GLFW",
		"GLAD",
		"imgui",
		"opengl32.lib"
	}

	filter "system:windows"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"HZ_PLATFORM_WINDOWS",
			"HZ_BUILD_DLL",
			"HZ_ENABLE_ASSERTS",
			"GLFW_INCLUDE_NONE"
		}


		postbuildcommands {
			"xcopy /Y /F \"$(TargetDir)$(TargetName).lib\" \"$(SolutionDir)bin\\$(Configuration)-$(LlvmPlatformName)\\SandBox\\\""
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


project "SandBox"

	location "SandBox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

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
		staticruntime "on"
	
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
			"$(SolutionDir)vendor/imgui",
			"$(SolutionDir)vendor/GLAD/include",
			"%{IncludeDir.entt}"
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