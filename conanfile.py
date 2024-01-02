from conans import ConanFile

class GameServer(ConanFile):
    name = "GameServer"
    version = "0.1"
    license = "MIT"
    description = "Game Server for voxel prototype."

    requires = (
        "spdlog/1.8.0",
        "boost/1.81.0",
        "glm/cci.20230113",
        #"vk-bootstrap/0.4",
        #"opencl-headers/2022.09.30",
        #"opencl-icd-loader/2022.09.30",
        "vulkan-headers/1.3.239.0",
        "vulkan-memory-allocator/3.0.1",
        "stb/cci.20220909",
        "tinyobjloader/2.0.0-rc10",
        "glfw/3.3.8"
    )

    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    exports_sources = "*", "!*build/*"

    def build(self):
        cmake.configure()
        cmake.build()

    def imports(self):
        self.copy('*.so*', dst='lib', src='lib')
