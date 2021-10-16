from conans import ConanFile, CMake

class IntervoxConan(ConanFile):
    generators = "cmake"
    settings = "os", "compiler", "build_type", "arch"
    requires =  "glm/0.9.9.8"


    def requirements(self):
        if self.settings.os == "Macos":
            self.requires("moltenvk/1.1.1")
        else:
            self.requires("vulkan-loader/1.2.190")

 
