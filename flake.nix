{
  description = "Launcher - Minecraft Launcher";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        launcher = pkgs.callPackage ./nix/unwrapped.nix { };
      in {
        packages = {
          launcher = pkgs.callPackage ./nix/wrapper.nix { inherit launcher; };
          launcher-debug = pkgs.callPackage ./nix/wrapper.nix {
            inherit launcher;
            debug = true;
          };
          default = self.packages.${system}.launcher;
        };

        checks.launcher = self.packages.${system}.launcher;

        devShells.default = pkgs.mkShell {
          inputsFrom = [ self.packages.${system}.launcher ];
          packages = with pkgs; [ cmake ninja ];
        };
      });
}
