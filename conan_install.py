Import("env")

try:
    import conan
except ImportError:
    env.Execute("$PYTHONEXE -m pip install conan")

env.Execute("$PYTHONEXE -m conans.conan install . --output-folder=build --build=missing")

info = SConscript("./build/SConscript_conandeps")
env.MergeFlags(info["sdl"])
env.MergeFlags(info["libcurl"])

import sys
import os
import glob
import shutil

if sys.platform == "win32":
    build_dir = env.subst("$BUILD_DIR")
    os.makedirs(build_dir, exist_ok=True)
    for pkg in ["sdl", "libcurl"]:
        if pkg in info and "BINPATH" in info[pkg]:
            for bin_path in info[pkg]["BINPATH"]:
                for dll in glob.glob(os.path.join(bin_path, "*.dll")):
                    shutil.copy2(dll, build_dir)

# print(env.Dump())
