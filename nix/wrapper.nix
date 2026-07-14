{ lib
, stdenv
, makeWrapper
, launcher
, debug ? false
}:

stdenv.mkDerivation {
  pname = if debug then "launcher-debug" else "launcher";
  version = "git";

  dontBuild = true;
  dontConfigure = true;

  nativeBuildInputs = [ makeWrapper ];

  installPhase = ''
    mkdir -p $out/bin
    makeWrapper ${launcher}/bin/launcher $out/bin/launcher \
      --set QT_QPA_PLATFORMTHEME "xdgdesktopportal"
  '';

  meta = launcher.meta // {
    mainProgram = "launcher";
  };
}
