"""Keep the linker map path relative to support Windows project paths with accents."""

import os

Import("env")

map_path = os.path.relpath(
    os.path.join(env.subst("$BUILD_DIR"), env.subst("${PROGNAME}.map")),
    env.subst("$PROJECT_DIR"),
).replace("\\", "/")

env.Replace(
    LINKFLAGS=[
        '-Wl,-Map="%s"' % map_path if str(flag).startswith("-Wl,-Map=") else flag
        for flag in env.get("LINKFLAGS", [])
    ]
)
