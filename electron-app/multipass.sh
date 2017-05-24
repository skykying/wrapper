#!/bin/sh
env TMPDIR=$XDG_RUNTIME_DIR XDG_CURRENT_DESKTOP=$( echo $XDG_CURRENT_DESKTOP | sed s/Unity:Unity7/Unity/  ) desktop-launch $SNAP/electron-app/multipass --no-sandbox
