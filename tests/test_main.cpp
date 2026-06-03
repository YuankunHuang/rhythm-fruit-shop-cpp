#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <filesystem>
#include <doctest/doctest.h>

#ifndef RFS_PROJECT_ROOT
#error "Define RFS_PROJECT_ROOT in CMake for rfs_tests"
#endif

namespace {
    struct ForceProjectRootCwd {
        ForceProjectRootCwd() {
            std::filesystem::current_path(RFS_PROJECT_ROOT);
        }
    };
    const ForceProjectRootCwd g_force_project_root_cwd{};
}