#!/bin/sh
# CEP tools, pinned for Linux/amd64. Xephyr creates an isolated
# graphical window; the container never receives the desktop session's cookie.
set -eu

usage() {
    printf '%s\n' \
        'Usage: ./scripts/run-container.sh' \
        'Opens this clock example in a Xephyr window.' \
        'Requires Docker, xserver-xephyr, xauth and an X11 or XWayland session.' \
        'Ctrl+C stops QEMU and removes the temporary container and display credentials.'
}
case ${1-} in
    -h|--help) usage; exit 0 ;;
    '') ;;
    *) usage >&2; exit 2 ;;
esac
if [ "$#" -ne 0 ]; then
    usage >&2
    exit 2
fi
demo=clock
for tool in docker Xephyr xauth; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        printf 'Missing tool: %s. See the setup instructions in README.md.\n' "$tool" >&2
        exit 1
    fi
done
if ! docker info >/dev/null 2>&1; then
    printf 'Docker is not accessible. Check the service and your user permissions.\n' >&2
    exit 1
fi
if [ -z "${DISPLAY-}" ]; then
    printf 'Run this from your X11 or XWayland desktop session with DISPLAY set.\n' >&2
    exit 1
fi
repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
image=sebastienviardot/pcsefriscv@sha256:701a53695beefb420e003b39c18c04f3c43de67cddbcf8ab534fa8378855e250
mkdir -p "$repo_dir/build/container"
session_dir=$(mktemp -d "$repo_dir/build/container/session.XXXXXX")
xephyr_pid=
docker_pid=
cid_file=$session_dir/container.cid
stop_container() {
    if [ -s "$cid_file" ]; then
        container_id=$(cat "$cid_file")
        # This file is created by our docker run in a private session directory.
        docker stop --time 2 "$container_id" >/dev/null 2>&1 || true
        docker rm --force "$container_id" >/dev/null 2>&1 || true
    fi
}
cleanup() {
    trap '' INT TERM HUP
    stop_container
    if [ -n "$docker_pid" ]; then
        kill "$docker_pid" 2>/dev/null || true
        wait "$docker_pid" 2>/dev/null || true
        # Also handle cancellation while Docker was creating the container.
        stop_container
    fi
    if [ -n "$xephyr_pid" ]; then
        kill "$xephyr_pid" 2>/dev/null || true
        wait "$xephyr_pid" 2>/dev/null || true
    fi
    rm -f "$session_dir/Xauthority" "$session_dir/display" "$session_dir/xephyr.log" "$cid_file"
    rmdir "$session_dir" 2>/dev/null || true
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM HUP
umask 077
session_auth=$session_dir/Xauthority
: > "$session_auth"
# This random credential is new for this window; no host cookies are copied.
cookie=$(od -An -N16 -tx1 /dev/urandom | tr -d ' \n')
xauth -f "$session_auth" add :0 MIT-MAGIC-COOKIE-1 "$cookie"
TMPDIR="$session_dir" Xephyr -displayfd 3 -screen 1280x720 -nolisten tcp -auth "$session_auth" \
    -title "OS - $demo (container)" 3> "$session_dir/display" \
    > "$session_dir/xephyr.log" 2>&1 &
xephyr_pid=$!
attempt=0
while [ ! -s "$session_dir/display" ]; do
    if ! kill -0 "$xephyr_pid" 2>/dev/null || [ "$attempt" -ge 100 ]; then
        cat "$session_dir/xephyr.log" >&2
        printf 'Could not open a Xephyr window in this desktop session.\n' >&2
        exit 1
    fi
    sleep 0.1
    attempt=$((attempt + 1))
done
nested_display=$(cat "$session_dir/display")
xauth -f "$session_auth" add ":$nested_display" MIT-MAGIC-COOKIE-1 "$cookie"
unset cookie
printf 'Demo: %s. The interface appears after compilation.\n' "$demo"
printf 'The first run downloads the toolchain image if it is not already installed.\n'
printf 'UART output appears in this terminal. Press Ctrl+C to stop.\n'

# Only the isolated server's socket and newly created credential are mounted.
# Matching the hostname lets Xlib find the FamilyLocal Xauthority entry.
# No host-session cookie, xhost changes, privileged mode or TCP ports are used.
docker run --rm --init --cidfile "$cid_file" --platform linux/amd64 --ulimit core=0 \
    --tmpfs /tmp:rw,size=64m \
    --user "$(id -u):$(id -g)" --hostname "$(hostname)" --network none \
    --mount "type=bind,source=$repo_dir,target=/workspace" \
    --mount "type=bind,source=/tmp/.X11-unix/X$nested_display,target=/tmp/.X11-unix/X$nested_display,readonly" \
    --mount "type=bind,source=$session_auth,target=/tmp/os-xauthority,readonly" \
    --env "DISPLAY=:$nested_display" --env XAUTHORITY=/tmp/os-xauthority \
    --env SDL_VIDEODRIVER=x11 --workdir /workspace \
    --entrypoint sh "$image" -ec '
        demo=$1
        course_tools=/matieres/4MMPCSEA/xinul/bin
        build_dir=build/container/cep
        make BUILD="$build_dir" \
            CROSS_COMPILE="$course_tools/riscv64-unknown-elf-" \
            RISCV_ARCH=rv64ima all
        exec "$course_tools/qemu-system-riscv64" \
            -machine cep -bios none -kernel "$build_dir/kernel.elf" \
            -m 32M -smp 1 -name "OS - $demo" \
            -display sdl,show-cursor=on -serial stdio -monitor none -no-user-config
    ' sh "$demo" &
docker_pid=$!
# QEMU can keep running after the X server closes. Watch both processes so
# closing the window has the same cleanup behavior as interrupting the script.
while kill -0 "$docker_pid" 2>/dev/null; do
    if ! kill -0 "$xephyr_pid" 2>/dev/null; then
        exit 0
    fi
    sleep 0.2
done
status=0
wait "$docker_pid" || status=$?
docker_pid=
exit "$status"
