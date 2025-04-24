from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout

class JHToolkitFull(ConanFile):
    name = "jh-toolkit"
    version = "1.3.1"
    license = "Apache-2.0"
    url = "https://github.com/JeongHan-Bae/jh-toolkit"
    description = "Full modular toolkit for modern C++20 generic programming"
    settings = "os", "arch", "compiler", "build_type"
    exports_sources = "CMakeLists.txt", "include/*", "src/*"
    generators = "CMakeDeps", "CMakeToolchain"

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["TAR"] = "ALL"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.libdirs = ["lib"]
