create:
	rm -rf build
	meson setup build
	meson compile -C build

run:
	./build/waycast

dev:
	make create
	./build/waycast

dev_dependencies:
	sudo rm -rf /usr/share/waycast
	sudo ln -s $(PWD) /usr/share/waycast
	@echo "✅ /usr/share/waycast -> $(PWD)"

install:
	sudo pacman -Rns waycast
	makepkg -si
