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

include "vendor/GLFW"

project "Hazel"
	location "Hazel"
	kind "SharedLib"
	language "C++"

	targetdir ("$(SolutionDir)bin/" .. outputDir .. "/%{prj.name}")
	objdir ("$(SolutionDir)bin-int/" .. outputDir .. "/%{prj.name}")

	pchheader "hzpch.h"
	pchsource "src/hzpch.cpp"

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"$(SolutionDir)src",
		"$(SolutionDir)vendor/spdlog/include",
		"$(SolutionDir)vendor/GLFW/include",
		"%{IncludeDir.GLFW}"
	}

	links
	{
		"GLFW",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"HZ_PLATFORM_WINDOWS",
			"HZ_BUILD_DLL",
			"HZ_ENABLE_ASSERTS"
		}


		

	filter "configurations:Debug"
		defines "HZ_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "HZ_Release"
		optimize "On"

	filter "configurations:Dist"
		defines "HZ_DEBUG"
		optimize "On"


project "SandBox"

	location "SandBox"
	kind "ConsoleApp"
	language "C++"

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
		"$(SolutionDir)src/"
	}


	links
	{
		"Hazel"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"HZ_PLATFORM_WINDOWS"
		}


	filter "configurations:Debug"
		defines "HZ_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "HZ_Release"
		optimize "On"

	filter "configurations:Dist"
		defines "HZ_DEBUG"
		optimize "On"