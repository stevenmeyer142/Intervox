cmake_minimum_required (VERSION 3.17)
project (Intervox)

include(build/conanbuildinfo.cmake)
conan_basic_setup()

set(OPENGL_SOURCE
	source/CGLFiducials.cpp
	source/CGLMesh.cpp
	source/CGLMeshHolder.cpp
	source/CGLMeshRenderer.cpp
	source/CGLObject.cpp
	source/CGLPerspective.cpp
	source/CGLPointer.cpp
	source/CGLRenderable.cpp
	source/CGLTexFont.cpp
	source/COpenGLContext.cpp
	source/GLMain.cpp
)

set(VULKAN_SOURCE
	source/CVulkanContext.cpp
#	source/Vulkan/VulkanOffscreenRenderer.cpp
#	source/Vulkan/VulkanTools.cpp
)
 
set(BASE_SOURCE 
	source/C3DMesh.cpp
	source/JNICommon.cpp
	source/utility/C3DFloatPoint.cpp
	source/utility/C3DPoint.cpp
	source/utility/CCubeMarcher.cpp
	source/utility/CFacetLookup.cpp
	source/utility/CJavaArrSlicesSet.cpp
	source/utility/CMySortedList.cpp
	source/utility/CMyError.cpp
	source/utility/CSlicesSet.cpp
	source/utility/CTempMemPointer.cpp
	source/utility/CTriangle.cpp
	source/utility/CTriangleList.cpp
	source/utility/CVertex.cpp
	source/utility/CVertexList.cpp
)




add_library (Intervox 
	SHARED

	${BASE_SOURCE}
#	${VULKAN_SOURCE}
)


target_compile_options(Intervox
	PRIVATE 
	-w
	-std=gnu++14
	-DJNI_LIB
)
target_include_directories(Intervox
	PRIVATE
	source
	"/usr/local/Cellar/openjdk/17/libexec/openjdk.jdk/Contents/Home/include"
	"/usr/local/Cellar/openjdk/17/libexec/openjdk.jdk/Contents/Home/include/darwin"
)


target_link_libraries(Intervox ${CONAN_LIBS})