# FreeRTOS+FAT warning-cleanup changes and FSP regeneration impact

Project: `usb_hmsc_rzt2n_evk_ca55_0_ep` (RZ/T2N EVK, Cortex-A55, `aarch64-none-elf`, e²studio)
Library: `rz/freertos/Lab-Project-FreeRTOS-FAT` (FreeRTOS+FAT V2.3.3 — Labs)
Last updated: 2026-08-30

---

## Summary

- Twelve source-level warning-cleanup changes are applied across seven FreeRTOS+FAT files.
- The recorded comparison in this document shows the compiler-warning count reduced from 305 to 8.
- The changes are limited to explicit casts, one prototype, one removed redundant cast, one
  converted preprocessor warning and one `(void)` statement. No functional behavior change is
  intended, but functional regression testing is still required before customer release.
- Smart Configurator may overwrite `rz/`. If this occurs, reapply the changes with
  `script/fix_fat_warnings.ps1`, then clean, rebuild and retest the project.

---

## 1. Modified files

All paths are relative to the e²studio project root.

| # | File | Changes | Warning category addressed |
|---|------|---------|----------------------------|
| 1 | `rz/freertos/Lab-Project-FreeRTOS-FAT/include/ff_error.h` | 1 | `-Wsign-conversion` |
| 2 | `rz/freertos/Lab-Project-FreeRTOS-FAT/include/FreeRTOSFATConfigDefaults.h` | 1 | `-Wunused-value` |
| 3 | `rz/freertos/Lab-Project-FreeRTOS-FAT/ff_file.c` | 3 | `-Wconversion`, `-Wsign-conversion` |
| 4 | `rz/freertos/Lab-Project-FreeRTOS-FAT/ff_format.c` | 1 | `-Wconversion` |
| 5 | `rz/freertos/Lab-Project-FreeRTOS-FAT/ff_fat.c` | 3 | conversion warnings, `-Wcpp` |
| 6 | `rz/freertos/Lab-Project-FreeRTOS-FAT/ff_dir.c` | 2 | `-Wmissing-declarations`, `-Wconversion` |
| 7 | `rz/freertos/Lab-Project-FreeRTOS-FAT/ff_ioman.c` | 1 | `-Wunused-but-set-variable` |

Additional project files:

| File | Purpose |
|------|---------|
| `script/fix_fat_warnings.ps1` | Reapplies the twelve changes when the vendor sources are regenerated |
| `FreeRTOS-FAT-warning-fixes.md` | This document |

`rz_cfg/freertos/FreeRTOSFATConfig.h` is generated and must not be edited. The `FF_PRINTF` cleanup
is applied in `FreeRTOSFATConfigDefaults.h` so that it does not depend on the generated file.

---

## 2. Change details

Line numbers below refer to the unmodified library sources used when the cleanup was prepared.

| # | Location | Warning cause | Change |
|---|----------|---------------|--------|
| 1 | `ff_error.h:64` | `FF_ERRFLAG` is unsigned while `FF_isERR(x)` receives signed `FF_Error_t` | Cast `x` explicitly to `unsigned` before masking |
| 2 | `FreeRTOSFATConfigDefaults.h:460` | An empty `FF_PRINTF` macro turns calls into unused comma expressions | Replace it with the library's no-op variadic function when logging is disabled |
| 3 | `ff_file.c:1085` | Integer promotion makes the complemented mask signed | Cast the complemented mask to `uint8_t` |
| 4 | `ff_file.c:1440` | A 64-bit `BaseType_t` value is added to `uint32_t` | Cast `xIndex` to `uint32_t` |
| 5 | `ff_file.c:3027` | The complemented attribute mask is signed after integer promotion | Cast the mask to `uint8_t` |
| 6 | `ff_format.c:948` | `BaseType_t` is added to `uint32_t` | Cast the already range-checked value to `uint32_t` |
| 7 | `ff_fat.c:243` | `#warning` produces `-Wcpp` on every build | Convert the directive to a source comment |
| 8 | `ff_fat.c:417` | A `uint32_t` return value is unnecessarily cast through `int32_t` | Return `ulFATEntry` directly |
| 9 | `ff_fat.c:1177` | `BaseType_t` entry size is added to `uint32_t` | Cast `xEntrySize` to `uint32_t` |
| 10 | `ff_dir.c:154` | `strcasecmp` is defined without a preceding prototype | Add a prototype immediately before the definition |
| 11 | `ff_dir.c:1336`, `ff_dir.c:1918` | `BaseType_t` is added to a `uint16_t` item index | Cast the checked expression to `uint16_t` |
| 12 | `ff_ioman.c:722` | `ulFirstCluster` is read only when `ffconfigFAT_CHECK` is enabled | Add `(void) ulFirstCluster` for the disabled configuration |

---

## 3. Effect of FSP regeneration

| Directory | Regeneration behavior | Release impact |
|-----------|-----------------------|----------------|
| `rz_cfg/` | Generated configuration is rewritten | Do not place manual fixes here |
| `rz/` | Vendor sources may be restored from the selected FSP pack | The seven modified library files may lose their warning-cleanup changes |
| `src/`, `script/`, project-root files | Not expected to be replaced by library extraction | The reapply script and this document remain available |

After changing or regenerating the FSP configuration, check the script output, clean the project,
rebuild it and execute the normal functional test procedure.

---

## 4. Functional-impact statement and recorded evidence

The changes were designed as warning cleanups and are not intended to alter application behavior.
Most changes make conversions explicit that the compiler previously performed implicitly; the
prototype and `(void)` statement do not generate code, and converting `#warning` to a comment affects
only preprocessing diagnostics.

The recorded build-size comparison was:

```text
   text     data     bss      dec      hex   filename
 155092   115952   77560   348604    551bc   usb_hmsc_rzt2n_evk_ca55_0_ep.elf   (305 warnings)
 155092   115952   77560   348604    551bc   usb_hmsc_rzt2n_evk_ca55_0_ep.elf   (8 warnings)
```

Equal section sizes do not prove that two binaries are byte-for-byte identical. No hash or binary
comparison from those two builds was retained, so this document does not claim binary identity.
For a release decision, rebuild the current sources, review the complete warning log and run the
project's functional tests. If binary equivalence is required, compare deterministic `.bin` or `.hex`
outputs, or compare the relevant ELF sections and record the resulting hashes.

---

## 5. Reapplying the changes

From the e²studio project root, run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ".\script\fix_fat_warnings.ps1"
```

Expected output when all changes are already present:

```text
fix_fat_warnings: applied=0 already-ok=12 problems=0
```

Expected output after compatible vendor sources have been restored:

```text
fix_fat_warnings: applied=12 already-ok=0 problems=0
```

Then select **Project → Clean**, rebuild and retest.

To run the script before every build, open:

**Project → Properties → C/C++ Build → Settings → Build Steps → Pre-build steps**

Use this command for all configurations:

```text
powershell -NoProfile -ExecutionPolicy Bypass -File "${ProjDirPath}/script/fix_fat_warnings.ps1"
```

The script is idempotent and refuses to patch an unrecognized source fragment. It currently reports
problems in its output without failing the build, so the pre-build output must be reviewed. A release
build must not be accepted when the script reports `NOT FOUND` or a nonzero `problems` count.

---

## 6. Alternative: suppress warnings for vendor sources

Instead of maintaining source-level changes, warning options may be disabled only for the vendor
`rz` directory:

```text
-Wno-conversion -Wno-sign-conversion -Wno-unused-value -Wno-unused-but-set-variable -Wno-missing-declarations
```

This survives source regeneration because it is stored in `.cproject`, but it can also hide new
warnings in third-party code. If this alternative is selected, document it as part of the release
configuration and keep strict warnings enabled for application code in `src/`.

---

## 7. Enabling FreeRTOS+FAT logging

If FreeRTOS+FAT logging is required, define the following before
`FreeRTOSFATConfigDefaults.h` is included:

```c
#define ffconfigHAS_FF_PRINTF   1
#define FF_PRINTF( ... )        printf( __VA_ARGS__ )
```

When `ffconfigHAS_FF_PRINTF` is nonzero, the defaults header retains the application-provided
`FF_PRINTF` definition instead of using the no-op implementation.