# Launcher

Launcher is a clean and maintained Minecraft launcher, forked from PrismLauncher.

No emojis, symbols, or special characters in commit messages, code, documentation, or UI text. ASCII only.

## Building

See the [build instructions](https://prismlauncher.org/wiki/development/build-instructions) for details on building from source.

The basic steps are:

1. Clone the repository with submodules:

```sh
git clone --recurse-submodules https://github.com/CleanLauncher/Launcher.git
cd Launcher
```

1. Configure and build:

```sh
cmake -B build -G Ninja
cmake --build build
```

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on how to get started.

If you have questions or want to discuss changes, please open a GitHub Issue.

## License

All launcher code is available under the GPL-3.0-only license. See [licenses/](licenses/) for full details including third-party license notices.
