# template_cpp Generator Improvements

## Current State

The `template_cpp` generator is well-designed with Bash and already supports most necessary features:
- Output directory parameter (`OUT=` / `AW_OUT`)
- Shared file copying (`.clang-format`, `.clang-tidy`, `.editorconfig`)
- Template-based project scaffolding

## Improvements to Implement

### 1. ✅ Remove `make eject` as default workflow
**Current**: User generates in template_cpp directory, then runs `make eject`  
**Problem**: Inconvenient and error-prone; requires cleanup  
**Solution**: Make output directory the standard pattern

**Recommended Usage**:
```bash
# Instead of:
cd template_cpp
make init TYPE=cli NAME=myapp
make eject  # Clean up templates

# Simply use:
make init TYPE=cli NAME=myapp OUT=../myapp
# Project is ready to use immediately
```

### 2. ✅ Template already supports output directory
**Status**: Already implemented  
**Current syntax**: `make init TYPE=cli OUT=../path`  
**or**: `AW_OUT=../path make init TYPE=cli`

### 3. ✅ Configuration files are already copied
**Status**: Already implemented in `scaffold.sh`  
**Files copied**:
- `.clang-format`
- `.clang-tidy`  
- `.editorconfig`
- `.gitignore`
- `.gitattributes`
- `CMakePresets.json`
- `VERSION`

**Example files copied**:
- `CMakeUserPresets.json.example` → `CMakeUserPresets.json`

### 4. ⚠️ `.gitignore` handling needs clarification
**Issue**: Generated projects might incorrectly treat `.clang-tidy` as build artifact  
**Current**: `.clang-tidy` is a configuration file (should be committed)  
**Recommendation**: Document that `.clang-tidy` and `.clang-format` should NOT be gitignored

**Proper .gitignore pattern**:
```bash
# CORRECT: Clang build artifacts and cache
.clangd/          # Cache directory for clang tools

# INCORRECT (don't add these):
# .clang-tidy      # This is a config file - keep it!
# .clang-format    # This is a config file - keep it!
```

## Documentation Improvements

### Add to Makefile help:
```makefile
  make init TYPE=cli OUT=../myapp    # Generate to specific directory
```

### Add to README:
```markdown
## Quick Start

Generate a new project directly to a specific directory:

\`\`\`bash
# Using Makefile
make init TYPE=cli NAME=myapp OUT=../myapp

# Using direct scaffold call
bash templates/scaffold.sh init cli --name myapp --out ../myapp
\`\`\`

The generated project is immediately ready to use:
- No need to call \`make eject\`
- Configuration files (.clang-format, .clang-tidy, .editorconfig) are copied
- All build targets are available: \`make build\`, \`make test\`, \`make lint\`
\`\`\`

## Workflow Comparison

### Old Workflow (Less Convenient)
```bash
cd template_cpp
make init TYPE=cli NAME=gitsync
# [project scaffolded in-place]
make eject
# [now move project elsewhere]
mv src include tests ... ../gitsync-new/
```

### Recommended Workflow (Convenient)
```bash
cd workspace
make -C template_cpp init TYPE=cli NAME=gitsync OUT=./gitsync-new
cd gitsync-new
make build
make test
```

## Implementation Notes

The template_cpp generator **already does everything correctly**. The improvements are primarily:

1. **User experience**: Recommend using `OUT=` parameter as standard
2. **Documentation**: Clarify that `.clang-tidy` and `.clang-format` are config files
3. **Process**: Deprecate/document `make eject` as optional cleanup only

## Status

✅ All technical requirements already met  
⚠️ Documentation and user guidance need update  
📝 Recommended actions:
- Add `OUT=` examples to Makefile help
- Update README with recommended workflow
- Add note about preserving `.clang-tidy` and `.clang-format`
