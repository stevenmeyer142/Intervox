cmake_minimum_required (VERSION 3.17)
project (Intervox)

set(USE_CONAN 0)
if (USE_CONAN)
    include(build/conan_toolchain.cmake)
 else()
 find_package(Vulkan REQUIRED)

 IF(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
ELSEIF(LINUX)
	IF (NOT Vulkan_FOUND)
		find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
		IF (Vulkan_LIBRARY)
			set(Vulkan_FOUND ON)
			MESSAGE("Using bundled Vulkan library version")
		ENDIF()
	ENDIF()
	find_package(Threads REQUIRED)
	IF(USE_D2D_WSI)
		MESSAGE("Using direct to display extension...")
		add_definitions(-D_DIRECT2DISPLAY)
	ELSEIF(USE_DIRECTFB_WSI)
		find_package(DirectFB REQUIRED)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_DIRECTFB_EXT")
		include_directories(${DIRECTFB_INCLUDE_DIR})
	ELSEIF(USE_WAYLAND_WSI)
		find_program(PKG_CONFIG pkg-config)
		if (NOT PKG_CONFIG)
			message(FATAL_ERROR "pkg-config binary not found")
		endif ()
		find_package(Wayland REQUIRED)
		if (NOT WAYLAND_FOUND)
			message(FATAL_ERROR "Wayland development package not found")
		endif ()
		pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)
		if (NOT WAYLAND_PROTOCOLS_FOUND)
			message(FATAL_ERROR "Wayland protocols package not found")
		endif ()
		find_program(WAYLAND_SCANNER wayland-scanner)
		if (NOT WAYLAND_SCANNER)
			message(FATAL_ERROR "wayland-scanner binary not found")
		endif ()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WAYLAND_KHR")
		include_directories(${WAYLAND_INCLUDE_DIR})
		pkg_get_variable(protocol_dir wayland-protocols pkgdatadir)
		execute_process(COMMAND ${WAYLAND_SCANNER} client-header ${protocol_dir}/stable/xdg-shell/xdg-shell.xml ${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.h
				COMMAND ${WAYLAND_SCANNER} private-code ${protocol_dir}/stable/xdg-shell/xdg-shell.xml ${CMAKE_BINARY_DIR}/xdg-shell-protocol.c)
		include_directories(${CMAKE_BINARY_DIR})
	ELSEIF(USE_HEADLESS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_HEADLESS_EXT")
	ELSE(USE_D2D_WSI)
		find_package(XCB REQUIRED)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_XCB_KHR")
	ENDIF(USE_D2D_WSI)
ELSEIF(APPLE)
	IF (CMAKE_GENERATOR MATCHES "Xcode")
		# Suppress regeneration for Xcode since environment variables will be lost if not set in Xcode locations/custom paths
		set(CMAKE_SUPPRESS_REGENERATION ON)
		set(CMAKE_XCODE_GENERATE_SCHEME ON)
		IF (Vulkan_FOUND)
			# If the Vulkan loader was found and its environment variables are defined, make them available within Xcode schemes
			IF (DEFINED ENV{VK_ADD_LAYER_PATH})
				set(CMAKE_XCODE_SCHEME_ENVIRONMENT "${CMAKE_XCODE_SCHEME_ENVIRONMENT};VK_ADD_LAYER_PATH=$ENV{VK_ADD_LAYER_PATH}")
			ENDIF()
			IF (DEFINED ENV{VK_ICD_FILENAMES})
				set(CMAKE_XCODE_SCHEME_ENVIRONMENT "${CMAKE_XCODE_SCHEME_ENVIRONMENT};VK_ICD_FILENAMES=$ENV{VK_ICD_FILENAMES}")
			ENDIF()
			IF (DEFINED ENV{VK_DRIVER_FILES})
				set(CMAKE_XCODE_SCHEME_ENVIRONMENT "${CMAKE_XCODE_SCHEME_ENVIRONMENT};VK_DRIVER_FILES=$ENV{VK_DRIVER_FILES}")
			ENDIF()
		ENDIF()
	ENDIF()
	message("Apple build detected")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_METAL_EXT -DVK_EXAMPLE_XCODE_GENERATED")
ENDIF(WIN32)


endif()

find_package(JNI)

if (NOT JNI_FOUND)
    set(JNI_INCLUDE_DIRS
    	$ENV{JAVA_HOME}/include
    	$ENV{JAVA_HOME}/include/darwin
    )
    # windows include/win32
    # linux include/linux
    # freebsd include/freebsd

    message("setting JNI_INCLUDE_DIRS ${JNI_INCLUDE_DIRS}")
endif()


set(VULKAN_SOURCE
    base/VulkanBuffer.cpp
    base/VulkanDebug.cpp
    base/VulkanDevice.cpp
    base/VulkanTools.cpp
    base/vulkanexamplebase.cpp
    extensions/VulkanContext.cpp
    extensions/VulkanMesh.cpp
extensions/VulkanMeshPipeline.cpp
)
 
set(INTERVOX_SOURCE
    GLMain.cpp
    IntervoxHeadlessVulkan.cpp
    JNICommon.cpp
    utility/C3DFloatPoint.cpp
    utility/C3DPoint.cpp
    utility/CCubeMarcher.cpp
    utility/CFacetLookup.cpp
    utility/CJavaArrSlicesSet.cpp
    utility/CMyError.cpp
    utility/CMySortedList.cpp
    utility/CSlicesSet.cpp
    utility/CTempMemPointer.cpp
    utility/CTriangle.cpp
    utility/CTriangleList.cpp
    utility/CVertex.cpp
    utility/CVertexList.cpp
)




add_library (Intervox 
	SHARED

	${INTERVOX_SOURCE}
	${VULKAN_SOURCE}
)


target_compile_options(Intervox
	PRIVATE 
	-w
	-std=gnu++14
	-DSPIRV_CROSS_NAMESPACE_OVERRIDE=spirv_cross
	-DENABLE_OPT
	-DINTERVOX_LIB=1
	-DENABLE_HLSL
	-DINTERVOX_JNI
	-DDEBUG=1
)
target_include_directories(Intervox
	PRIVATE
	.
	${JNI_INCLUDE_DIRS}
	/usr/local/include
)

set(CMAKE_CXX_FLAGS "--sysroot ${CMAKE_OSX_SYSROOT} ${CMAKE_CXX_FLAGS}")

target_link_libraries(Intervox ${CONAN_LIBS} Vulkan::Vulkan)
