{ lib
, stdenv
, cmake
, ninja
, pkg-config
, extra-cmake-modules
, qt6Packages
, qt6
, libarchive
, cmark
, libqrencode
, tomlplusplus
, zlib
}:

stdenv.mkDerivation {
  pname = "launcher-unwrapped";
  version = "git";

  src = ./..;

  nativeBuildInputs = [
    cmake
    ninja
    pkg-config
    extra-cmake-modules
  ];

  buildInputs = [
    qt6.qtbase
    qt6.qtsvg
    qt6.qtimageformats
    qt6.qt5compat
    qt6.qtwebengine
    libarchive
    cmark
    libqrencode
    tomlplusplus
    zlib
  ];

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
    "-DENABLE_LTO=ON"
    "-DLauncher_BUILD_PLATFORM=nix"
  ];

  meta = with lib; {
    description = "Launcher - Minecraft Launcher";
    license = licenses.gpl3Only;
    platforms = platforms.linux ++ platforms.darwin;
  };
}
