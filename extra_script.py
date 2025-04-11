Import("env")

env.AddCustomTarget(
    "Upgrade OTA",
    "$BUILD_DIR/${PROGNAME}.bin",
    "python do_ota_upgrade.py $SOURCE"
)
