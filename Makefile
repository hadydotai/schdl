RAYLIB_PATH=./deps/raylib
RAYLIB_INCLUDE=-I$(RAYLIB_PATH)/src

CFLAGS=-Wall -g
RAYLIB_STATIC_FLAGS=-L$(RAYLIB_PATH)/src -lraylib -lglfw -lGL -lm -lpthread -ldl

SRCS=main.c data.c scrollable.c flexbox.c scaling.c parser.c

default: schdl

release:
	@VERSION=$$(grep '#define VERSION' main.c | cut -d '"' -f2); \
	RELEASE_DIR="release-$$VERSION"; \
	echo "Creating release $$VERSION..."; \
	mkdir -p "$$RELEASE_DIR/deps"; \
	cp CHANGELOG data.c data.h flexbox.c flexbox.h main.c Makefile \
		parser.c parser.h scaling.c scaling.h scrollable.c scrollable.h \
		tuesday.schedule README.md LICENSE screenshot.png "$$RELEASE_DIR/"; \
	cp deps/DEPS "$$RELEASE_DIR/deps/"; \
	chmod +x "$$RELEASE_DIR/deps/DEPS"; \
	tar -cf "$$RELEASE_DIR.tar" "$$RELEASE_DIR"; \
	rm -rf "$$RELEASE_DIR"; \
	echo "Created $$RELEASE_DIR.tar"

install-deps:
	@echo "Installing dependencies for Debian/Ubuntu-based systems..."
	@if command -v apt-get >/dev/null; then \
		sudo apt-get update && \
		sudo apt-get install -y build-essential git \
		cmake libglfw3-dev libasound2-dev libx11-dev libxrandr-dev \
		libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev \
		libxinerama-dev libwayland-dev libxkbcommon-dev valgrind; \
	else \
		echo "This command is for Debian/Ubuntu systems."; \
		echo "For Fedora/RHEL: sudo dnf install gcc-c++ cmake glfw-devel valgrind"; \
		echo "For Arch Linux: sudo pacman -S base-devel cmake glfw valgrind"; \
	fi

install: schdl
	@echo "Installing schdl to /usr/local/bin..."
	@sudo cp schdl /usr/local/bin/
	@echo "Installation complete. You can now run 'schdl' from anywhere."


schdl: $(SRCS)
	gcc -o schdl $(SRCS) $(CFLAGS) $(RAYLIB_STATIC_FLAGS) $(RAYLIB_INCLUDE)

debug:
	gcc -o schdl $(SRCS) $(CFLAGS) $(RAYLIB_STATIC_FLAGS) $(RAYLIB_INCLUDE) -g

debug-run: debug
	gdb --batch --ex run --ex bt --ex q --args ./schdl

memcheck: debug
	valgrind --leak-check=full \
		--show-leak-kinds=all \
		--track-origins=yes \
		--verbose \
		--log-file=valgrind-out.txt \
		./schdl

run: schdl
	./schdl

clean:
	rm -f schdl valgrind-out.txt

.PHONY: install-deps default run clean memcheck