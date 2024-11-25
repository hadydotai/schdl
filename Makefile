RAYLIB_PATH=./deps/raylib
RAYLIB_INCLUDE=-I$(RAYLIB_PATH)/src

CFLAGS=-Wall -g
RAYLIB_STATIC_FLAGS=-lraylib -lglfw -lGL -lm -lpthread -ldl -L./deps/raylib/src

SRCS=main.c data.c scrollable.c flexbox.c scaling.c

default: schdl

# Add new install-deps target
install-deps:
	@echo "Installing dependencies for Debian/Ubuntu-based systems..."
	@if command -v apt-get >/dev/null; then \
		sudo apt-get update && \
		sudo apt-get install -y build-essential git \
		cmake libglfw3-dev libasound2-dev libx11-dev libxrandr-dev \
		libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev \
		libxinerama-dev libwayland-dev libxkbcommon-dev; \
	else \
		echo "This command is for Debian/Ubuntu systems."; \
		echo "For Fedora/RHEL: sudo dnf install gcc-c++ cmake glfw-devel"; \
		echo "For Arch Linux: sudo pacman -S base-devel cmake glfw"; \
	fi

schdl: $(SRCS)
	gcc -o schdl $(SRCS) $(CFLAGS) $(RAYLIB_STATIC_FLAGS) $(RAYLIB_INCLUDE)

debug:
	gcc -o schdl $(SRCS) $(CFLAGS) $(RAYLIB_STATIC_FLAGS) $(RAYLIB_INCLUDE) -g

debug-run: debug
	gdb --batch --ex run --ex bt --ex q --args ./schdl

run: schdl
	./schdl

clean:
	rm -f schdl

.PHONY: install-deps default run clean