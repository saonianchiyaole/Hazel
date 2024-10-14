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

include "vendor/GLFW"
include "vendor/GLAD"
include "vendor/imgui"
include "vendor/glm"

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
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.glm}"
	}

	links
	{
		"GLFW",
		"GLAD",
		"imgui",
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
			"HZ_ENABLE_ASSERTS",
			"GLFW_INCLUDE_NONE"
		}


		postbuildcommands {
			"xcopy /Y /F \"$(TargetDir)$(TargetName).dll\" \"$(SolutionDir)bin\\$(Configuration)-$(LlvmPlatformName)\\SandBox\\\""
		}
		

	filter "configurations:Debug"
		defines "HZ_DEBUG"
		buildoptions "/MDd"
		symbols "On"

	filter "configurations:Release"
		defines "HZ_Release"
		buildoptions "/MD"
		optimize "On"

	filter "configurations:Dist"
		defines "HZ_DEBUG"
		buildoptions "/MD"
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
		"$(SolutionDir)src/",
		"%{IncludeDir.glm}"
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
		buildoptions "/MDd"
		symbols "On"

	filter "configurations:Release"
		defines "HZ_Release"
		buildoptions "/MD"
		optimize "On"

	filter "configurations:Dist"
		defines "HZ_DEBUG"
		buildoptions "/MD"
		optimize "On"