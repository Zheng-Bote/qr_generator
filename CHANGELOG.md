<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Changelog](#changelog)
  - [[0.2.0] - 2026-03-02](#020---2026-03-02)
    - [Added](#added)
    - [Changed](#changed)
  - [[0.1.0] - 2026-03-01](#010---2026-03-01)
    - [Added](#added-1)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.2.0] - 2026-03-02

### Added

- **macOS Installer**: Added CPack support for generating macOS `.pkg` installers.
- **SVG to Standard Output**: Added support for outputting SVG directly to standard output (`std::cout`).

### Changed

- **README.md**: Updated project documentation with new features.

## [0.1.0] - 2026-03-01

### Added

- **Initial Release**: C++23 based QR Code Generator.
- **Header-Only Library**: Encapsulated core QR generation logic in `include/qr_generator.hpp` for easy integration into custom projects.
- **CLI Application**: Created a command-line wrapper (`qrgen`) to generate QR codes from the terminal.
- **Dual Format Support**: Generate both scalable vector graphics (`.svg`) and high-compression raster images (`.webp`).
- **Custom Colors**: Added support for customizable foreground and background colors via hex codes, including alpha (opacity) channels.
- **Self-Contained Build**: Implemented CMake `FetchContent` to automatically download and statically link `libqrencode` and `libwebp`, removing the need for pre-installed system dependencies.
- **Automated CI/CD**: Added comprehensive GitHub Actions workflows for CodeQL security scanning, Cppcheck static analysis, Clang-Tidy linting, SBOM generation (SPDX & CycloneDX), and automated CVE vulnerability checks.
- **Modern C++23 Idioms**: Fully utilizes C++23 features such as `std::print`, `std::ranges`, and `std::format`.
- **Documentation**: Provided complete Doxygen-style documentation and English translations for all source files, compliant with standards.
- **README.md**: Added detailed project documentation including setup instructions, CLI usage examples, and a Mermaid architecture diagram.
