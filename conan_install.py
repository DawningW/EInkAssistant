Import("env")

try:
    import conan
except ImportError:
    env.Execute("$PYTHONEXE -m pip install conan")

env.Execute("$PYTHONEXE -m conans.conan install . --output-folder=build --build=missing")

info = SConscript("./build/SConscript_conandeps")
env.MergeFlags(info["sdl"])
env.MergeFlags(info["libcurl"])

# print(env.Dump())
