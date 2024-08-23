
workspace "night"
	architecture "x86"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["SDL3"] = "night/vendor/sdl3/include"

LibDir = {}
LibDir["SDL3"] = "%{prj.name}/vendor/sdl3/VisualC/Win32/Release"

project "night"
	location "night"
	kind "SharedLib"
	language "C++"

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
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.SDL3}"
	}

	libdirs 
	{
		"%{LibDir.SDL3}"
	}

	links
	{
		"SDL3.lib",
		"winmm.lib",
		"version.lib",
		"Imm32.lib",
		"Setupapi.lib",
		"libcmt.lib",
		"libucrtd.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"NIGHT_PLATFORM_WINDOWS",
			"NIGHT_BUILD_DLL"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/sandbox")
		}

		filter "configurations:Debug"
			defines "NIGHT_DEBUG"
			symbols "On"

		filter "configurations:Release"
			defines "NIGHT_RELEASE"
			optimize "On"

		filter "configurations:Dist"
			defines "NIGHT_DIST"
			optimize "On"

project "sandbox"
	location "sandbox"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"night/vendor/spdlog/include",
		"night/src"
	}

	links
	{
		"night"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"NIGHT_PLATFORM_WINDOWS",
		}

		filter "configurations:Debug"
			defines "NIGHT_DEBUG"
			symbols "On"

		filter "configurations:Release"
			defines "NIGHT_RELEASE"
			optimize "On"

		filter "configurations:Dist"
			defines "NIGHT_DIST"
			optimize "On"