way:
	rm -rf build
	meson setup build
	meson compile -C build

dev_dependencies:
	sudo rm -rf /usr/share/waycast
	sudo ln -s $(PWD) /usr/share/waycast
	@echo "✅ /usr/share/waycast -> $(PWD)"