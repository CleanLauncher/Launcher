# Launcher Program Info

This directory contains program metadata and branding information for Launcher, including:

- **Application name and logo**: Branding assets such as SVG icons, PNG icons, and macOS `.icns` files
- **Desktop integration**: `.desktop` file for Linux desktop environments, metainfo XML for AppStream/software centers
- **Windows resources**: `.rc` resource file, `.manifest` for Windows compatibility, and `.ico` icon
- **macOS resources**: `.qrc` Qt resource file, `.icns` icon bundle
- **Man page**: `launcher.6.scd` scdoc source for the man page
- **Build configuration**: `CMakeLists.txt` that configures and copies these files into the build directory

## Files

| File | Purpose |
| ------ | --------- |
| `CMakeLists.txt` | CMake build configuration that defines branding variables and configures template files |
| `io.github.launcher.Launcher.desktop.in` | Linux desktop entry template |
| `io.github.launcher.Launcher.metainfo.xml.in` | AppStream metainfo template |
| `launcher.6.scd.in` | Man page source template (scdoc format) |
| `launcher.manifest.in` | Windows application manifest |
| `launcher.rc.in` | Windows resource file template |
| `launcher.qrc.in` | Qt resource file template |
