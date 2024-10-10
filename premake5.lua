
-- TODO: change dialect to c++ 20 and define NIGHT_ENABLE_ASSERT

workspace "night"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["SDL2"] = "$(SolutionDir)/night/vendor/sdl2/include"
IncludeDir["SDL2_image"] = "$(SolutionDir)/night/vendor/SDL2_image/include"

LibDir = {}
LibDir["SDL2"] = "$(SolutionDir)/night/vendor/SDL2/lib/x64"
LibDir["SDL2_image"] = "$(SolutionDir)/night/vendor/SDL2_image/lib/x64"

project "night"
	location "night"
	kind "SharedLib" -- TODO: add static library macro
	language "C++"
	cppdialect "C++20"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "nightpch.h"
	pchsource "night/src/nightpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		--"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/vendor/glm",
		"%{IncludeDir.SDL2}",
		"%{IncludeDir.SDL2_image}"
	}

	libdirs 
	{
		"%{LibDir.SDL2}",
		"%{LibDir.SDL2_image}"
	}

	links
	{
		"SDL2.lib",
		"SDL2main.lib",
		"SDL2_image.lib"
	}

	filter "system:windows"
		cppdialect "C++20"
		--staticruntime "On"
		systemversion "latest"

		defines
		{
			"NIGHT_PLATFORM_WINDOWS",
			"NIGHT_USE_DOUBLE_PRECISION",
			"NIGHT_BUILD_DLL"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/sandbox")
		}

		filter "configurations:Debug"
			defines 
			{ 
				"NIGHT_DEBUG",
				"NIGHT_ENABLE_LOGGING",
				"NIGHT_CORE"
			}
			symbols "On"

		filter "configurations:Release"
			defines 
			{
				"NIGHT_RELEASE",
				"NIGHT_ENABLE_LOGGING",
				"NIGHT_CORE"
			}
			optimize "On"

		filter "configurations:Dist"
			defines 
			{
				"NIGHT_DIST",
				"NIGHT_CORE"
			}
			optimize "On"

project "sandbox"
	location "sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		--"night/vendor/spdlog/include",
		"%{prj.name}/src",
		"night/vendor/glm",
		"night/src",
		"%{IncludeDir.SDL2}", --TODO: fix this
		"%{IncludeDir.SDL2_image}" --TODO: fix this
	}

	libdirs 
	{
		"%{LibDir.SDL2}",
		"%{LibDir.SDL2_image}"
	}

	links
	{
		"SDL2.lib",
		"SDL2main.lib",
		"SDL2_image.lib",
		"night"
	}

	filter "system:windows"
		cppdialect "C++20"
		--staticruntime "On"
		systemversion "latest"

		defines
		{
			"NIGHT_PLATFORM_WINDOWS",
			"NIGHT_USE_DOUBLE_PRECISION"
		}

		filter "configurations:Debug"
			defines
			{
				"NIGHT_DEBUG",
				"NIGHT_ENABLE_LOGGING"
			}
			symbols "On"

		filter "configurations:Release"
			defines 
			{
				"NIGHT_RELEASE",
				"NIGHT_ENABLE_LOGGING"
			}
			optimize "On"

		filter "configurations:Dist"
			defines "NIGHT_DIST"
			optimize "On"