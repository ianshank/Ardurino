# Wires Core sources into the native (host) test build.
# PlatformIO native env does not auto-discover project libs the same way
# as embedded envs, so we add wildlife_core/src explicitly via SCons.
#
# Import() and env are SCons/PlatformIO globals injected at runtime;
# ruff F821 is suppressed only on the lines that reference them.

import os

Import("env")

# Link libgcov explicitly — MinGW/Windows ld does not auto-add it from
# -fprofile-arcs / --coverage the way Linux GCC does.
env.Append(LIBS=["gcov"])

core_src = os.path.join(env["PROJECT_DIR"], "lib", "wildlife_core", "src")
if os.path.isdir(core_src):
    build_dir = env.subst("$BUILD_DIR")
    env.BuildSources(
        os.path.join(build_dir, "wildlife_core"),
        core_src,
    )
