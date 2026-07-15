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
    wrapQtAppsHook
  ];

  buildInputs = with pkgs; [
    qt6.qtbase
    qt6.qtdeclarative
    qt6.qtnetworkauth
    qt6.qtbase.qtdeclarative
    cmark
    libarchive
    libqrencode
    tomlplusplus
    zlib
  ];

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
    "-DCMAKE_INSTALL_PREFIX=/"
  ];

  postFixup = ''
    wrapProgram $out/bin/launcher \
      --prefix QT_PLUGIN_PATH : ${pkgs.qt6.qtbase}/lib/qt-6/plugins
  '';

  meta = with pkgs.lib; {
    description = "Clean and maintained Minecraft launcher";
    homepage = "https://github.com/CleanLauncher/Launcher";
    license = licenses.gpl3Only;
    platforms = platforms.linux;
  };
}
