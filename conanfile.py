from conan import ConanFile
from conan.tools.scons import SConsDeps

class NativeRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "SConsDeps"
    default_options = {"sdl/*:shared": True, "libcurl/*:shared": True}

    def requirements(self):
        self.requires("sdl/3.4.0")
        self.requires("libcurl/8.19.0")
