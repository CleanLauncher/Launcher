# Contributing to Launcher

Thank you for your interest in contributing to Launcher!

## Rules

- No emojis, symbols, or special characters in commit messages, code, documentation, or UI text. ASCII only.
- All commits must be signed off with `Signed-off-by` using the original author's name and email.

## Development Setup

1. Clone the repository with submodules:

```sh
git clone --recurse-submodules https://github.com/CleanLauncher/Launcher.git
cd Launcher
```

1. Install dependencies and configure:

```sh
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
```

1. Build:

```sh
cmake --build build
```

## Code Style

All files are formatted with `clang-format` using the configuration in `.clang-format`. Run it on changed files before committing.

C++ conventions:

- Class and type names: `PascalCase`
- Private/protected data members: `camelCase` prefixed with `m_`
- Private/protected static data members: `camelCase` prefixed with `s_`
- Public data members: `camelCase`
- Static const members: `SCREAMING_SNAKE_CASE`
- Function members: `camelCase`
- Global functions and non-const global variables: `camelCase`
- Const global variables and macros: `SCREAMING_SNAKE_CASE`
- Enum constants: `PascalCase`

## Pull Request Process

1. Fork the repository and create a feature branch.
2. Make your changes following the code style above.
3. Ensure all formatting checks pass before submitting.
4. Open a pull request with a clear description of your changes.
5. A maintainer will review your PR. Please respond to any feedback.

## Questions

If you have questions about contributing, please open a GitHub Issue. We do not use Discord or email for development discussion.

## Signing Your Work

All contributions must be signed off. Add `-s` to your `git commit` call, or manually append:

```text
Signed-off-by: Author name <Author email>
```

By signing off, you agree to the Developer Certificate of Origin (DCO): see the full text in the original PrismLauncher contributing guide.
