from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout


class BongoConan(ConanFile):
    name = "bongo"
    version = "01"

    # Optional metadata
    license = "Apache"
    author = "<abhis> <abhilash.kollam@gmail.com>"
    url = "https://github.com/abhilashraju/Bongo"
    description = "An HTTP client using sender reciever pattern"
    topics = ("<http client>", "<libcurl wrapper>", "<sender reciever>")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [False, False], "fPIC": [False, False]}
    default_options = {"shared": False, "fPIC": False}

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*", "include/*"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    # def build(self):
        cmake = CMake(self)
        cmake.configure()
    #     cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        # self.copy("include/*.h")
        # self.copy("include/*.hpp")
        
    # def package_info(self):
    #     self.cpp_info.libs = ["Bongo"]
