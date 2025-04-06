from os import path

Import("env")

# find library
LIBRARY_DIR = None
for lib_dir in env.get("LIBSOURCE_DIRS"):
    search_lib_path = path.join(env.subst(lib_dir), "U8g2_for_Adafruit_GFX")
    if path.isdir(search_lib_path):
        LIBRARY_DIR = search_lib_path
        break

if not LIBRARY_DIR:
    print("Can not find U8g2_for_Adafruit_GFX, exit")
    exit(1)

print("Find U8g2_for_Adafruit_GFX: " + LIBRARY_DIR)
patchflag_path = path.join(LIBRARY_DIR, ".patching-done")

# patch file only if we didn't do it before
if not path.isfile(patchflag_path):
    patch_file = path.join("patches", "1-u8g2-add-esp8266-fonts-support.patch")
    env.Execute("python tools/patch.py -d %s %s" % (LIBRARY_DIR, patch_file))

    def _touch(path):
        with open(path, "w") as fp:
            fp.write("")
    env.Execute(lambda *args, **kwargs: _touch(patchflag_path))
