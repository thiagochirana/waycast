pkgname=waycast
pkgver=0.1.2
pkgrel=1
pkgdesc="Wayland app launcher built with raylib"
arch=('x86_64')
url="https://github.com/thiagochirana/waycast"
license=('unknown')
depends=('glib2' 'raylib')
makedepends=('git' 'meson' 'ninja' 'pkgconf')
source=("${pkgname}::git+file://${startdir}")
sha256sums=('SKIP')

build() {
  cd "$srcdir/$pkgname"
  meson setup build --prefix=/usr --buildtype=release
  meson compile -C build
}

package() {
  cd "$srcdir/$pkgname"

  install -d "$pkgdir/usr/lib/waycast"
  install -Dm755 build/waycast "$pkgdir/usr/lib/waycast/waycast"

  install -Dm644 data/config.toml "$pkgdir/usr/share/waycast/config.toml"
  cp -r fonts "$pkgdir/usr/share/waycast/"
  cp -r themes "$pkgdir/usr/share/waycast/"

  install -Dm755 /dev/stdin "$pkgdir/usr/bin/waycast" <<'EOF'
#!/bin/sh
cd /usr/share/waycast || exit 1
exec /usr/lib/waycast/waycast "$@"
EOF
}
