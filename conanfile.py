#!/usr/bin/env python

# -*- coding: utf-8 -*-

import re
from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration
from conans.model.version import Version


class RedisClientConan(ConanFile):
    name = "redis-client"
    url = " https://github.com/cblauvelt/redis-client"
    homepage = url
    author = "Christopher Blauvelt"
    description = "A C++ REDIS library."
    license = "MIT"
    topics = ("redis", "asio")
    exports = ["LICENSE"]
    exports_sources = ["CMakeLists.txt", "conan.cmake",
                       "conanfile.py", "redis/*", "test/*"]
    generators = "cmake"
    settings = "os", "arch", "compiler", "build_type"
    requires = "cpool/main_23c5e65a0f9b", "boost/1.78.0", "openssl/1.1.1m", "fmt/8.1.1"
    build_requires = "gtest/cci.20210126"
    options = {"cxx_standard": [20], "build_testing": [
        True, False], "trace_logging": [True, False]}
    default_options = {"cxx_standard": 20,
                       "build_testing": True, "trace_logging": False}

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.settings.os == "Windows" and \
           self.settings.compiler == "Visual Studio" and \
           Version(self.settings.compiler.version.value) < "16":
            raise ConanInvalidConfiguration(
                "redis-client does not support MSVC < 16")

    def sanitize_tag(self, version):
        return re.sub(r'^v', '', version)

    def sanitize_branch(self, branch):
        return re.sub(r'/', '_', branch)[:12]

    def set_version(self):
        git = tools.Git(folder=self.recipe_folder)
        self.version = self.sanitize_tag(git.get_tag()) if git.get_tag(
        ) else "%s_%s" % (self.sanitize_branch(git.get_branch()), git.get_revision()[:12])

    def build(self):
        cmake = CMake(self)
        cmake.definitions["CMAKE_CXX_STANDARD"] = self.options.cxx_standard
        cmake.definitions["BUILD_TESTING"] = self.options.build_testing
        cmake.definitions["CPOOL_TRACE_LOGGING"] = self.options.trace_logging
        cmake.configure()
        cmake.build()
        cmake.test()

    def package(self):
        self.copy("LICENSE", dst="licenses")
        self.copy("*.hpp", dst="include/redis", src="redis")
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["redis_client"]
