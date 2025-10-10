workspace "confhub"
	configurations { "Debug", "Release" }

project "confhub"
	language "C++"
	kind "ConsoleApp"
	cppdialect "C++20"
	architecture "x64"
	targetdir "bin/%{cfg.buildcfg}"
	files { "./src/**.h", "./src/**.cpp" }
	includedirs { "./includes/src", "./includes/src/**", "./includes/uses/src/**" }
	libdirs { "./includes/lib/**", "./includes/uses/lib/**" }
	links {
		
	}
	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
