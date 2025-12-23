cmake_minimum_required (VERSION 3.28)
project (IntervoxLib)

# "Get cmake file directory"
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src/main/native")


 find_package(Vulkan REQUIRED)

IF(APPLE)
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
	set(CMAKE_CXX_FLAGS "--sysroot ${CMAKE_OSX_SYSROOT} ${CMAKE_CXX_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include ${SOURCE_DIR}/base/mvk_vulkan.h")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_METAL_EXT -DVK_EXAMPLE_XCODE_GENERATED")
ENDIF(APPLE)




set(VULKAN_SOURCE
    ${SOURCE_DIR}/base/VulkanBuffer.cpp
    ${SOURCE_DIR}/base/VulkanDebug.cpp
    ${SOURCE_DIR}/base/VulkanDevice.cpp
    ${SOURCE_DIR}/base/VulkanTools.cpp
    ${SOURCE_DIR}/base/vulkanexamplebase.cpp
    ${SOURCE_DIR}/extensions/VulkanMesh.cpp
    ${SOURCE_DIR}/extensions/VulkanMeshPipeline.cpp
)
 
set(INTERVOX_SOURCE
    ${SOURCE_DIR}/IntervoxHeadlessVulkan.cpp
    ${SOURCE_DIR}/utility/C3DFloatPoint.cpp
    ${SOURCE_DIR}/utility/C3DPoint.cpp
    ${SOURCE_DIR}/utility/CMyError.cpp
    ${SOURCE_DIR}/utility/CMySortedList.cpp
    ${SOURCE_DIR}/utility/CSlicesSet.cpp
    ${SOURCE_DIR}/utility/CTempMemPointer.cpp
    ${SOURCE_DIR}/utility/CTriangle.cpp
    ${SOURCE_DIR}/utility/CTriangleList.cpp
    ${SOURCE_DIR}/utility/CVertex.cpp
    ${SOURCE_DIR}/utility/CVertexList.cpp
)




add_library (IntervoxLib 
	SHARED

	${INTERVOX_SOURCE}
	${VULKAN_SOURCE}
)


target_compile_options(IntervoxLib
	PRIVATE 
	-w
	-std=gnu++14
	-DSPIRV_CROSS_NAMESPACE_OVERRIDE=spirv_cross
	-DENABLE_OPT
	-DINTERVOX_LIB=1
	-DENABLE_HLSL
	-DDEBUG=1
	-DFORCE_VALIDATION=1
)
target_include_directories(IntervoxLib
	PRIVATE
	.
	/usr/local/include
	${SOURCE_DIR}
)


target_link_libraries(IntervoxLib  Vulkan::Vulkan)
