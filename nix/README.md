# Building Launcher with Nix

## Building locally

With Nix installed, you can build Launcher directly:

```sh
nix build
```

This will produce a result symlink containing the built package.

## Development shell

To enter a development shell with all required dependencies:

```sh
nix develop
```

Then configure and build as usual:

```sh
cmake -B build -G Ninja
cmake --build build
```
