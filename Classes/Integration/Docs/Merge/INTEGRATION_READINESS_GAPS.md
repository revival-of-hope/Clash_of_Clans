# Integration Readiness Gaps

## GAP-IR-001 — Real game configure blocked by missing GTK3

- **Claim:** Real-game configure cannot complete in this environment because GTK3 is missing.
  - **Evidence:** `cmake -S . -B build_real -DUSE_COCOS_ENGINE=ON -DBUILD_APP=ON -DBUILD_TESTS=OFF`
  - **Pointer:** Configure output (CMake error)
  - **Quote:** “Could NOT find GTK3 (missing: GTK3_LIBRARIES GTK3_INCLUDE_DIRS)”

**What’s needed:**
- Install GTK3 development libraries (headers + libs) or provide a build container/toolchain where Cocos2d-x dependencies (GTK3) are available for the real-engine configuration.

