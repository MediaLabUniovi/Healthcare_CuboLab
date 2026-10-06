"""Avoid passing the accented project path to the Windows linker map option."""

import os

Import("env")

if os.name == "nt":
    map_path = os.path.relpath(
        os.path.join(env.subst("$BUILD_DIR"), env.subst("$PROGNAME") + ".map"),
        env.subst("$PROJECT_DIR"),
    ).replace("\\", "/")
    env.Replace(
        LINKFLAGS=[
            '-Wl,-Map="%s"' % map_path if "-Wl,-Map=" in str(flag) else flag
            for flag in env.get("LINKFLAGS", [])
        ]
    )
