# gitsync Makefile — CI contract
# Targets: build, test, lint, clean

.PHONY: build test lint clean help

PRESET ?= clang18-libcxx
BUILD_DIR := build/$(PRESET)

help: ## Show this help message
	@echo "gitsync Makefile targets:"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2}'

build: ## Configure and build with current preset
	cmake --preset $(PRESET)
	cmake --build --preset $(PRESET)

test: build ## Run all tests
	ctest --preset $(PRESET) --output-on-failure

lint: ## Run clang-format and clang-tidy checks
	@echo "Running clang-format check..."
	@find src include tests -name '*.cpp' -o -name '*.hpp' | xargs clang-format --dry-run --Werror
	@echo "Running clang-tidy..."
	@if [ -f "$(BUILD_DIR)/compile_commands.json" ]; then \
		find src include tests -name '*.cpp' | xargs clang-tidy -p $(BUILD_DIR); \
	else \
		echo "Error: compile_commands.json not found. Run 'make build' first."; \
		exit 1; \
	fi

clean: ## Remove build artifacts
	rm -rf build/

# CI Contract targets

ci-fast: ## Fast PR checks: lint + format + build + unit tests
	@echo "==> Running fast CI checks..."
	$(MAKE) lint
	cmake --preset clang18-libcxx -DGITSYNC_WARNINGS_AS_ERRORS=ON
	cmake --build --preset clang18-libcxx
	ctest --preset clang18-libcxx --output-on-failure

ci-deep: ## Deep scheduled checks: integration tests + sanitizers
	@echo "==> Running deep CI checks..."
	cmake --preset clang18-libcxx -DCMAKE_BUILD_TYPE=Debug -DGITSYNC_WARNINGS_AS_ERRORS=ON \
		-DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"
	cmake --build --preset clang18-libcxx
	ASAN_OPTIONS=detect_leaks=1 ctest --preset clang18-libcxx --output-on-failure

ci-coverage: ## Generate coverage report and badge
	@echo "==> Generating coverage report..."
	cmake --preset clang18-libcxx -DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_CXX_FLAGS="--coverage -fprofile-arcs -ftest-coverage"
	cmake --build --preset clang18-libcxx
	ctest --preset clang18-libcxx --output-on-failure
	@echo "Computing coverage..."
	@COVERAGE=$$(lcov --capture --directory build/clang18-libcxx --output-file coverage.info 2>/dev/null | \
		lcov --summary coverage.info 2>&1 | grep 'lines......:' | sed 's/.*: \([0-9.]*\)%.*/\1/'); \
	COLOR=$$(echo "$$COVERAGE >= 80" | bc -l | grep -q 1 && echo "brightgreen" || echo "orange"); \
	wget -qO coverage.svg "https://img.shields.io/badge/coverage-$${COVERAGE}%25-$$COLOR"

build-release: ## Build optimized release artifacts
	@echo "==> Building release artifacts..."
	cmake --preset release -DGITSYNC_WARNINGS_AS_ERRORS=ON
	cmake --build --preset release
	mkdir -p dist
	cp build/release/gitsync dist/gitsync-linux-x64
	strip dist/gitsync-linux-x64
