{ pkgs ? import <nixpkgs> {} }:

pkgs.stdenv.mkDerivation {
  pname = "launcher";
  version = "12.0.0";
  src = ./..;

  nativeBuildInputs = with pkgs; [
    cmake
    ninja
    pkg-config
    extra-cmake-modules
  ];

  buildInputs = with pkgs; [
    qt6.qtbase
    qt6.qtdeclarative
    qt6.qtnetworkauth
    cmark
    libarchive
    libqrencode
    tomlplusplus
    zlib
  ];

  dontWrapQtApps = true;

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
    "-DCMAKE_INSTALL_PREFIX=\${out}"
  ];
}
