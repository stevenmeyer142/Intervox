from conans import ConanFile, CMake

class IntervoxConan(ConanFile):
    generators = "cmake", "xcode"
    settings = "os", "compiler", "build_type", "arch"
    requires =  "glm/0.9.9.8"
    default_options = {"moltenvk:shared": True}


    def requirements(self):
        if self.settings.os == "Macos":
            self.requires("moltenvk/1.1.10")
        else:
            self.requires("vulkan-loader/1.2.190")

 
