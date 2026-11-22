# Windows Build Instructions

## First-time Setup

1. Install Visual Studio 2022 or later with C++ development tools
2. Install CMake 3.25 or later
3. Open "Developer Command Prompt for VS" or "x64 Native Tools Command Prompt"

## Build Release Binary

```bash
make clean
rm -rf build/Release-msvc/_deps/aw_cmake_modules-*
make release-msvc
```

The `rm` command clears cached CMake modules to ensure you get the latest version.

## Build Artifacts

- **Executable**: `build/Release-msvc/gitsync.exe`
- **Version info**: Embedded in binary (right-click → Properties → Details)
- **Icon**: `product.ico` (create this file in project root)
- **Digital signature**: Auto-signed if certificate "AI-Warevo Code Signing" is installed

## Icon Creation

If you don't have `product.ico`, create one:

```bash
# Using ImageMagick (if available)
magick convert icon-16.png icon-32.png icon-48.png icon-256.png product.ico

# Or use an online converter:
# https://www.icoconverter.com/
```

Recommended sizes: 16×16, 32×32, 48×48, 256×256

## Code Signing

The build system will attempt to sign `gitsync.exe` using:
- Certificate subject name: "AI-Warevo Code Signing"
- Timestamp servers: DigiCert, Comodo, GlobalSign, Certum, Microsoft (with fallback)

If the certificate is not installed, signing is skipped with a warning.
