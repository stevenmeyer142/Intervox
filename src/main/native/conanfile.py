from conan import ConanFile

class IntervoxConan(ConanFile):
    generators = "CMakeToolchain", "XcodeToolchain"
    settings = "os", "compiler", "build_type", "arch"
    requires =  "glm/0.9.9.8"
    default_options = {"moltenvk/*:shared": True}


    def requirements(self):
        if self.settings.os == "Macos":
            self.requires("moltenvk/1.2.2")
        else:
            self.requires("vulkan-loader/1.2.190")

 
