from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout

class JHToolkitPOD(ConanFile):
    name = "jh-toolkit-pod"
    version = "1.3.1"
    license = "Apache-2.0"
    url = "https://github.com/JeongHan-Bae/jh-toolkit"
    description = "Header-only POD system from JH Toolkit"
    settings = "os", "arch", "compiler", "build_type"
    exports_sources = "CMakeLists.txt", "include/*"
    generators = "CMakeDeps", "CMakeToolchain"
    no_copy_source = True

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["TAR"] = "POD"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_id(self):
        self.info.clear()  # header-only

    def package_info(self):
        self.cpp_info.includedirs = ["include"]
