# gitsync Makefile — CI contract + comprehensive build system
# This merges the spoofiq multi-preset system with gitsync's CI contract.
# Targets: build, test, lint, clean, debug-*, release-*, all, [CI: ci-fast, ci-deep, ci-coverage, build-release]

.PHONY: help configure build test clean debug release debug-gcc debug-clang debug-clang-libcxx debug-msvc release-gcc release-clang release-clang-libcxx release-msvc all ci-fast ci-deep ci-coverage build-release lint

help: ## Show this help message
	@echo "gitsync Makefile — available targets:"
	@echo ""
	@echo "Debug builds:"
	@echo "  make debug-gcc          - Configure & build Debug with GCC"
	@echo "  make debug-clang        - Configure & build Debug with Clang"
	@echo "  make debug-clang-libcxx - Configure & build Debug with Clang + libc++"
	@echo "  make debug-msvc         - Configure & build Debug with MSVC (Windows)"
	@echo ""
	@echo "Release builds:"
	@echo "  make release-gcc        - Configure & build Release with GCC"
	@echo "  make release-clang      - Configure & build Release with Clang"
	@echo "  make release-clang-libcxx - Configure & build Release with Clang + libc++"
	@echo "  make release-msvc       - Configure & build Release with MSVC (Windows)"
	@echo ""
	@echo "Quick shortcuts:"
	@echo "  make debug              - Build Debug (default: GCC)"
	@echo "  make release            - Build Release (default: GCC)"
	@echo "  make all                - Build all presets"
	@echo ""
	@echo "Development:"
	@echo "  make configure PRESET=<name> - Configure only"
	@echo "  make build PRESET=<name>     - Build only"
	@echo "  make test PRESET=<name>      - Run tests"
	@echo "  make lint               - Run clang-format and clang-tidy checks"
	@echo "  make clean              - Remove all build directories"
	@echo "  make clean-<preset>     - Remove specific build directory"
	@echo ""
	@echo "CI/CD:"
	@echo "  make ci-fast            - Fast PR checks (lint + build + tests)"
	@echo "  make ci-deep            - Deep checks (sanitizers + tests)"
	@echo "  make ci-coverage        - Generate coverage report"
	@echo "  make build-release      - Build optimized release binaries"

# Default preset
PRESET ?= debug-gcc

# Configure only
configure:
	cmake --preset $(PRESET)

# Build only
build:
	cmake --build --preset $(PRESET)

# Configure + Build
configure-build: configure build

# Tests
test: build
	ctest --preset $(PRESET) --output-on-failure

# Individual preset targets — Debug
debug-gcc: PRESET=debug-gcc
debug-gcc: configure-build

debug-clang: PRESET=debug-clang
debug-clang: configure-build

debug-clang-libcxx: PRESET=debug-clang-libcxx
debug-clang-libcxx: configure-build

debug-msvc: PRESET=debug-msvc
debug-msvc: configure-build

# Individual preset targets — Release
release-gcc: PRESET=release-gcc
release-gcc: configure-build

release-clang: PRESET=release-clang
release-clang: configure-build

release-clang-libcxx: PRESET=release-clang-libcxx
release-clang-libcxx: configure-build

release-msvc: PRESET=release-msvc
release-msvc: configure-build

# Quick aliases
debug: debug-gcc
release: release-gcc

# Build all presets (skips MSVC on non-Windows)
all: debug-gcc release-gcc debug-clang release-clang debug-clang-libcxx release-clang-libcxx

# Clean specific build directory
clean-%:
	rm -rf build/$*

# Clean all
clean:
	rm -rf build/Debug-* build/Release-* build/_deps

# Lint targets
lint: ## Run clang-format and clang-tidy checks
	@echo "Running clang-format check..."
	@find src include tests -name '*.cpp' -o -name '*.hpp' | xargs clang-format --dry-run --Werror
	@echo "Running clang-tidy..."
	@if [ -f "build/Debug-clang18-libcxx/compile_commands.json" ]; then \
		find src include tests -name '*.cpp' | xargs clang-tidy -p build/Debug-clang18-libcxx; \
	else \
		echo "Error: compile_commands.json not found. Run 'make clang18-libcxx' first."; \
		exit 1; \
	fi

# --- CI Contract Targets ---

ci-fast: ## Fast PR checks: lint + format + build + unit tests
	@echo "==> Running fast CI checks..."
	$(MAKE) lint
	cmake --preset clang18-libcxx -DGITSYNC_WARNINGS_AS_ERRORS=ON
	cmake --build --preset clang18-libcxx
	ctest --preset clang18-libcxx --output-on-failure

ci-deep: ## Deep scheduled checks: sanitizers + tests
	@echo "==> Running deep CI checks..."
	cmake --preset clang18-libcxx -DCMAKE_BUILD_TYPE=Debug -DGITSYNC_WARNINGS_AS_ERRORS=ON \
		-DCMAKE_CXX_FLAGS="-stdlib=libc++ -fsanitize=address,undefined -fno-omit-frame-pointer"
	cmake --build --preset clang18-libcxx
	ASAN_OPTIONS=detect_leaks=1 ctest --preset clang18-libcxx --output-on-failure

ci-coverage: ## Generate coverage report and badge
	@echo "==> Generating coverage report..."
	cmake --preset clang18-libcxx -DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_CXX_FLAGS="-stdlib=libc++ --coverage -fprofile-arcs -ftest-coverage"
	cmake --build --preset clang18-libcxx
	ctest --preset clang18-libcxx --output-on-failure
	@echo "Computing coverage..."
	@COVERAGE=$$(lcov --capture --directory build/Debug-clang18-libcxx --output-file coverage.info 2>/dev/null | \
		lcov --summary coverage.info 2>&1 | grep 'lines......:' | sed 's/.*: \([0-9.]*\)%.*/\1/'); \
	COLOR=$$(echo "$$COVERAGE >= 80" | bc -l | grep -q 1 && echo "brightgreen" || echo "orange"); \
	wget -qO coverage.svg "https://img.shields.io/badge/coverage-$${COVERAGE}%25-$$COLOR"

build-release: ## Build optimized release artifacts
	@echo "==> Building release artifacts..."
ifeq ($(OS),Windows_NT)
	cmake --preset release-msvc
	cmake --build --preset release-msvc
	mkdir -p dist
	cp build/Release-msvc/Release/gitsync.exe dist/gitsync.exe
else
	cmake --preset release-gcc -DGITSYNC_WARNINGS_AS_ERRORS=ON
	cmake --build --preset release-gcc
	mkdir -p dist
	cp build/Release-gcc/gitsync dist/gitsync
	strip dist/gitsync
endif
