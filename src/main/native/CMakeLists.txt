cmake_minimum_required (VERSION 3.17)
project (Intervox)

set(USE_CONAN 1)
if (USE_CONAN)
    include(build/conan_toolchain.cmake)
 else()
 find_package(Vulkan)

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

target_link_libraries(Intervox ${CONAN_LIBS})
