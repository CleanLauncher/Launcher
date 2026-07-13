# CleanLauncher Program Info

This directory contains program metadata and branding information for CleanLauncher, including:

- **Application name and logo** - Branding assets such as SVG icons, PNG icons, and macOS `.icns` files
- **Desktop integration** - `.desktop` file for Linux desktop environments, metainfo XML for AppStream/software centers
- **Windows resources** - `.rc` resource file, `.manifest` for Windows compatibility, and `.ico` icon
- **macOS resources** - `.qrc` Qt resource file, `.icns` icon bundle
- **Man page** - `cleanlauncher.6.scd` scdoc source for the man page
- **Build configuration** - `CMakeLists.txt` that configures and copies these files into the build directory

## Files

| File | Purpose |
|------|---------|
| `CMakeLists.txt` | CMake build configuration that defines branding variables and configures template files |
| `io.github.cleanlauncher.CleanLauncher.desktop.in` | Linux desktop entry template |
| `io.github.cleanlauncher.CleanLauncher.metainfo.xml.in` | AppStream metainfo template |
| `cleanlauncher.6.scd.in` | Man page source template (scdoc format) |
| `cleanlauncher.manifest.in` | Windows application manifest |
| `cleanlauncher.rc.in` | Windows resource file template |
| `cleanlauncher.qrc.in` | Qt resource file template |
