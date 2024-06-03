	workspace "Distributed Gaming System"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release"
	}



outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"



project "Middleware"
	location "Middleware"
	kind "StaticLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/src/**.cpp",
		"libs/BOOST1.82.0/libs/iostreams/src/zlib.cpp"
	}

	includedirs
	{
		"libs/ZLIB1.2.13",
		"libs/BOOST1.82.0"
	}
	
	links 
	{
		"libs/BOOST1.82.0/stage/lib/libboost_iostreams-vc143-mt-s-x64-1_82.lib",
		"libs/BOOST1.82.0/stage/lib/libboost_zlib-vc143-mt-s-x64-1_82.lib"
	}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "On"
		systemversion "latest"

		defines 
		{
			"DGS_PLATFORM_WINDOWS"
		}

		buildoptions
		{
			"/Zc:preprocessor"
		}
	
	filter "configurations:Debug"
		defines 
		{
			"DGS_BUILD_DEBUG"
		}
	
	filter "configurations:Release"
		defines 
		{
			"DGS_BUILD_RELEASE"
		}

		optimize "On"



project "Client"
	location "Client"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/src/**.cpp"
	}
	
	includedirs
	{
		"libs/GLEW2.1.0/include",
		"libs/GLFW3.3.8/include",
		"Middleware/src"
	}

	links 
	{
		"Middleware",
		"libs/GLFW3.3.8/lib-vc2022/glfw3.lib",
		"libs/GLEW2.1.0/lib/Release/x64/glew32s.lib",
		"opengl32.lib",
		"user32.lib",
		"gdi32.lib",
		"shell32.lib",
		"vcruntime.lib",
		"msvcrt.lib",
		"libs/BOOST1.82.0/stage/lib/libboost_iostreams-vc143-mt-s-x64-1_82.lib",
		"libs/BOOST1.82.0/stage/lib/libboost_zlib-vc143-mt-s-x64-1_82.lib"
	}

	defines 
	{
		"GLEW_STATIC"
	}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "On"
		systemversion "latest"

		defines 
		{
			"DGS_PLATFORM_WINDOWS"
		}

		buildoptions
		{
			"/Zc:preprocessor"
		}

	filter "configurations:Debug"
		defines 
		{
			"DGS_BUILD_DEBUG"
		}
	
	filter "configurations:Release"
		defines 
		{
			"DGS_BUILD_RELEASE"
		}

		optimize "On"



project "Server"
	location "Server"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/src/**.cpp"
	}
	
	includedirs
	{
		"libs/GLEW2.1.0/include",
		"libs/GLFW3.3.8/include",
		"libs/GLM0.9.9.8",
		"Middleware/src"
	}

	links 
	{
		"Middleware",
		"libs/GLFW3.3.8/lib-vc2022/glfw3.lib",
		"libs/GLEW2.1.0/lib/Release/x64/glew32s.lib",
		"opengl32.lib",
		"user32.lib",
		"gdi32.lib",
		"shell32.lib",
		"vcruntime.lib",
		"msvcrt.lib",
		"libs/BOOST1.82.0/stage/lib/libboost_iostreams-vc143-mt-s-x64-1_82.lib",
		"libs/BOOST1.82.0/stage/lib/libboost_zlib-vc143-mt-s-x64-1_82.lib"
	}

	defines 
	{
		"GLEW_STATIC"
	}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "On"
		systemversion "latest"

		defines 
		{
			"DGS_PLATFORM_WINDOWS"
		}

		buildoptions
		{
			"/Zc:preprocessor"
		}

	filter "configurations:Debug"
		defines 
		{
			"DGS_BUILD_DEBUG"
		}
	
	filter "configurations:Release"
		defines 
		{
			"DGS_BUILD_RELEASE"
		}

		optimize "On"