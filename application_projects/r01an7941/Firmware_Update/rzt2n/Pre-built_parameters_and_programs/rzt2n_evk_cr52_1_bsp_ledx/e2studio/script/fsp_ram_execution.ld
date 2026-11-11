/*
                  Linker File for Renesas RZ/T2 FSP

  ############################################################################
  # APPLICATION customization for RZ/T2N EVK - CR52 **CPU1** - FSP v4.2.0
  # (bsp_ledx / user program - NOT the SSBL)
  #
  # THIS FILE IS THE CPU1 VARIANT.  Do not copy it into a CPU0 project: CPU0 is
  # copied into ATCM 0x00000000 and entered at .loader_text/system_init, while
  # CPU1 is copied into SYSTEM_RAM 0x10000000 and entered at the reset vector.
  #
  # DESIGN RULE OF THIS FILE: the image that the SSBL copies into SYSTEM_RAM
  # must be a byte-for-byte picture of what SYSTEM_RAM should contain, and the
  # SSBL must
  # enter it at exactly the address the ELF declares as its entry point.
  # Everything below follows from those two rules.
  #
  #   1  .intvec is FIRST, pinned to SYSTEM_RAM 0x10000000, and
  #      ENTRY(__Vectors) pins the ELF entry point to the same address -
  #      which is what the SSBL branches to (--app_start_addr 10000000).
  #      The SSBL brings CPU1 out of reset, so the core has to enter through
  #      the vector table: only Reset_Handler enables BTCM and sets
  #      g_bsp_software_reset_occurred.  Entering at .loader_text/system_init
  #      instead - the CPU0 path - hangs CPU1 immediately.
  #      Without an ENTRY directive GNU ld picks the start of .text, so a JTAG
  #      debug session and an SSBL boot start at DIFFERENT addresses and a
  #      working debug session proves nothing about the flashed image.
  #
  #   2  Only ONE hardcoded address: .intvec at 0x10000000, which the SSBL
  #      requires.  .loader_text sits at 0x10001000 (the stock
  #      LOADER_START + LOADER_TEXT_OFFSET for a SYSTEM_RAM loader location),
  #      leaving a full 0x1000 window for the vector table; .text follows the
  #      loader block automatically, aligned to 0x100.  The ASSERTs at the end
  #      of this file catch any overlap.
  #
  #   3  No NOLOAD sections inside the image.  .loader_bss and .noinit are loaded,
  #      so the .bin has no holes: it does not depend on --gap-fill, and
  #      .software_reset is always 0 on a cold boot.  .heap / .aarch64_stack /
  #      .thread_stack stay NOLOAD but sit at the very end, so they add no hole.
  #
  #   4  .loader_stack moves OUT of BTCM into SYSTEM_RAM.  CPU1 leaves reset
  #      with BTCM disabled, so a stack in BTCM would only become usable part
  #      way through Reset_Handler.  LOADER_PROGRAM_LOCATION = 2 puts it in
  #      SYSTEM_RAM, which is what the working FSP v3.1.0 CPU1 project does.
  #      It stays NOLOAD at the end of SECTIONS so it adds no hole to the .bin.
  #
  # Resulting layout:
  #   SYSTEM_RAM
  #        0x10000000  .intvec       (vector table + .reset_handler)  <-- SSBL
  #        0x10001000  .loader_text
  #                    .loader_data
  #                    .loader_bss   (loaded, zero-filled)
  #                    .noinit       (loaded, zero-filled)
  #        ALIGN 0x100 .text / .ARM.* / .data / .bss
  #                    .heap / .aarch64_stack / .thread_stack (NOLOAD, no hole)
  #                    .loader_stack (NOLOAD)
  #
  # AFTER CHANGING THIS FILE THE IMAGE SIZE CHANGES.  parameter_generator.py
  # userapp and fwupdate_utility.py setupfile must both be re-run and the
  # initial image rewritten to flash - the SSBL validates the size it was told.
  #
  # LOADER_PROGRAM_LOCATION / USER_PROGRAM_LOCATION are set to 2 (SYSTEM_RAM)
  # in CPU1 MOD 0.  ASSERT_LOADER_PROGRAM_LOCATION only checks the primary
  # project, so this is allowed here.
  #
  # IMPORTANT: do NOT apply the SSBL modifications to this file.  The SSBL is
  # resident in BTCM 0x00102000..0x00107200 while it copies this application
  # into SYSTEM_RAM and releases CPU1.
  ############################################################################
*/

INCLUDE memory_regions.ld

INCLUDE redefinition_memory_regions_gcc.h

/* >>> CPU1 MOD 0 <<< ---------------------------------------------------------
 * This project is the SECONDARY project (CR52_1).  The SSBL does not copy it
 * into TCM: it copies it into SYSTEM_RAM and makes CPU1 branch to
 * --app_start_addr 10000000.  So every section of this application - the
 * startup ("loader") block, the vector table, .text/.data and the AArch32
 * stacks - must be linked in SYSTEM_RAM, not in ATCM/BTCM.
 *
 * Declaring the two location symbols here (0 = ATCM, 1 = BTCM, 2 = SYSTEM_RAM)
 * makes the stock formulas below resolve to SYSTEM_RAM on their own:
 *   LOADER_START = RAM_START = SYSTEM_RAM_ALIGN_START = 0x10000000
 *   LOADER_TEXT_ADDRESS      = 0x10000000 + 0x1000 = 0x10001000
 *   LOADER_DATA_STACK region = SYSTEM_RAM  (so .loader_stack leaves BTCM)
 * ASSERT_LOADER_PROGRAM_LOCATION only restricts the PRIMARY project
 * (_RZ_ORDINAL == 1), so changing it here is legal.
 * --------------------------------------------------------------------------*/
LOADER_PROGRAM_LOCATION = 2;
USER_PROGRAM_LOCATION   = 2;

/*============================================================================*/
/* A D D R E S S   S Y M B O L                                                */
/*============================================================================*/

/* Determine whether it is a primary project or not. */
_RZ_ORDINAL = (0 == SECONDARY_PRV) ? 1 : 2;

/* Check whether TCM is implemented on the core you intend to use. */
IMPLEMENTED_TCM = ((DEFINED(ATCM_CR52_0_START) || DEFINED(ATCM_CR52_1_START)) && (DEFINED(BTCM_CR52_0_START) || DEFINED(BTCM_CR52_1_START))) ? 1 : 0;

/* Determines whether the loader program can be placed in SYSTEM_RAM based on the presence or absence of the PICEn_MIRROR address space. */
LOADER_SYSTEM_RAM_ENABLE = (DEFINED(PCIE0_MIRROR_START) && !DEFINED(PCIE1_MIRROR_START)) ? 1 : 0;

/* Change the location region of the loader program and user program.
 * For details, refer to the "Change Program Location" chapter in the FSP documentation (BSP -> MCU Board Support Package). */

/* The following values can be set.
 * 0: ATCM
 * 1: BTCM
 * 2: SYSTEM_RAM */

LOADER_PROGRAM_LOCATION_PRV = DEFINED(LOADER_PROGRAM_LOCATION) ?
                                  LOADER_PROGRAM_LOCATION :
                                  (1 == IMPLEMENTED_TCM) ?
                                      1 :
                                      2;

USER_PROGRAM_LOCATION_PRV   = DEFINED(USER_PROGRAM_LOCATION) ?
                                  USER_PROGRAM_LOCATION :
                                  (1 == IMPLEMENTED_TCM) ?
                                      0 :
                                      2;

/* In the primary project, selecting any loader program location other than BTCM will cause a build error.
 * SYSTEM_RAM is available as a loader program location only for RZ/T2N. */
ASSERT_LOADER_PROGRAM_LOCATION = ((1 == _RZ_ORDINAL) && (1 == IMPLEMENTED_TCM) && 
                                  ((0 == LOADER_PROGRAM_LOCATION_PRV) || ((2 == LOADER_PROGRAM_LOCATION_PRV) && (0 == LOADER_SYSTEM_RAM_ENABLE)))) ? 0 : 1;
ASSERT(ASSERT_LOADER_PROGRAM_LOCATION, "ERROR: In the primary project, the location of the loader program cannot be changed.");

/* In cores without TCM, setting TCM for the loader program and user program will cause a build error. */
ASSERT_TCM = ((1 != IMPLEMENTED_TCM) && ((2 != LOADER_PROGRAM_LOCATION_PRV) || (2 != USER_PROGRAM_LOCATION_PRV))) ? 0 : 1;
ASSERT(ASSERT_TCM, "ERROR: There is no TCM in this core.");

/* The purpose of this process is to prevent the aforementioned build error from being buried among other build errors. */
LOADER_PROGRAM_LOCATION_PRV = (0 == ASSERT_TCM) ? 2 : LOADER_PROGRAM_LOCATION_PRV;
USER_PROGRAM_LOCATION_PRV = (0 == ASSERT_TCM) ? 2 : USER_PROGRAM_LOCATION_PRV;

/* The start address of the SYSTEM_RAM in the secondary project is located 0x20000 from the end address of the primary project. */
SYSTEM_RAM_ALIGN_START             = ALIGN(SYSTEM_RAM_PRV_START, 0x00020000);
SYSTEM_RAM_ALIGN_LENGTH            = SYSTEM_RAM_PRV_LENGTH - (SYSTEM_RAM_PRV_LENGTH % 0x00020000);

/* This is the offset of the mirror region of SYSTEM_RAM. It varies depending on the device. */
SYSTEM_RAM_MIRROR_OFFSET           = DEFINED(SYSTEM_RAM_START) ?
                                         DEFINED(SYSTEM_RAM_MIRROR_START) ?
                                             0x20000000 :
                                             (DEFINED(CR52_0) || DEFINED(CR52_1)) ?
                                                 0x00000000 :
                                                 0x00200000 :
                                         0;

/* Specifies the region where the user program is placed. */
RAM_START                          = (0 == USER_PROGRAM_LOCATION_PRV) ? ATCM_PRV_START :
                                     (1 == USER_PROGRAM_LOCATION_PRV) ? BTCM_PRV_START :
                                     (2 == USER_PROGRAM_LOCATION_PRV) ? SYSTEM_RAM_ALIGN_START : 0;
RAM_LENGTH                         = (0 == USER_PROGRAM_LOCATION_PRV) ? ATCM_PRV_LENGTH :
                                     (1 == USER_PROGRAM_LOCATION_PRV) ? BTCM_PRV_LENGTH :
                                     (2 == USER_PROGRAM_LOCATION_PRV) ? SYSTEM_RAM_ALIGN_LENGTH : 0;
TEXT_OFFSET                        = (0 == USER_PROGRAM_LOCATION_PRV) ?
                                         (0 == LOADER_PROGRAM_LOCATION_PRV) ? 0x00008000 : 0x00000100 :
                                     (1 == USER_PROGRAM_LOCATION_PRV) ?
                                         (1 == LOADER_PROGRAM_LOCATION_PRV) ? 0x00007000 : 0x00002000 :
                                     (2 == USER_PROGRAM_LOCATION_PRV) ?
                                         (2 == LOADER_PROGRAM_LOCATION_PRV) ? 0x00010000 : 0x00001000 :
                                     0;

/* >>> APP MOD 1 <<< ----------------------------------------------------------
 * Stock: .text is placed at TEXT_ADDRESS = RAM_START + TEXT_OFFSET.  In this
 * file .text is no longer placed at TEXT_ADDRESS: it follows the loader block
 * automatically, aligned to 0x100 (see SECTIONS).
 * TEXT_ADDRESS and TEXT_OFFSET are left unchanged and are not used.
 * --------------------------------------------------------------------------*/
/* Unused: kept so the stock symbol still exists for anything that
 * references it. */
TEXT_ADDRESS                       = RAM_START + TEXT_OFFSET;

/* Specifies the region where the loader program (SSBL) is placed. */
LOADER_START                       = (0 == LOADER_PROGRAM_LOCATION_PRV) ? ATCM_PRV_START :
                                     (1 == LOADER_PROGRAM_LOCATION_PRV) ? BTCM_PRV_START :
                                     (2 == LOADER_PROGRAM_LOCATION_PRV) ? SYSTEM_RAM_ALIGN_START : 0;
LOADER_LENGTH                      = (0 == LOADER_PROGRAM_LOCATION_PRV) ? ATCM_PRV_LENGTH :
                                     (1 == LOADER_PROGRAM_LOCATION_PRV) ? BTCM_PRV_LENGTH :
                                     (2 == LOADER_PROGRAM_LOCATION_PRV) ? SYSTEM_RAM_ALIGN_LENGTH : 0;
LOADER_TEXT_OFFSET                 = (0 == LOADER_PROGRAM_LOCATION_PRV) ? 0x00000100 :
                                     (1 == LOADER_PROGRAM_LOCATION_PRV) ? 0x00002000 :
                                     (2 == LOADER_PROGRAM_LOCATION_PRV) ? 0x00001000 : 0;

/* >>> APP MOD 2 <<< ----------------------------------------------------------
 * CPU1: LOADER_TEXT_ADDRESS keeps the stock formula.  With
 * LOADER_PROGRAM_LOCATION = 2 (CPU1 MOD 0) it resolves to SYSTEM_RAM
 * 0x10000000 + 0x1000 = 0x10001000, right after the vector table.
 * Do NOT apply the CPU0 change (LOADER_TEXT_ADDRESS = 0x00000000 in ATCM) here.
 * --------------------------------------------------------------------------*/
LOADER_TEXT_ADDRESS                = LOADER_START + LOADER_TEXT_OFFSET;   /* CPU1: 0x10001000 */

/* Specifies the region where the vector table is placed. */
INTVEC_START                       = (1 == IMPLEMENTED_TCM) ? ATCM_PRV_START : SYSTEM_RAM_ALIGN_START;
INTVEC_LENGTH                      = (1 == IMPLEMENTED_TCM) ? ATCM_PRV_LENGTH : SYSTEM_RAM_ALIGN_LENGTH;

/* >>> APP MOD 3 <<< ----------------------------------------------------------
 * CPU1: .intvec must be FIRST, at SYSTEM_RAM 0x10000000, because the SSBL
 * branches to --app_start_addr 10000000 and CPU1 has to enter through the
 * reset vector (see the header and ENTRY below).  INTVEC_ADDRESS is therefore
 * set to SYSTEM_RAM_ALIGN_START.  INTVEC_START / INTVEC_LENGTH keep the stock
 * values; the INTVEC memory region is not used for placement.
 * --------------------------------------------------------------------------*/
INTVEC_ADDRESS                     = SYSTEM_RAM_ALIGN_START;   /* CPU1 MOD: 0x10000000, where the SSBL branches */

/* Calculate the offset address of the noncache region. */
NONCACHE_OFFSET                    = DEFINED(NONCACHE_START) ? 0x00080000 : 0;
SHARED_NONCACHE_BUFFER_OFFSET      = DEFINED(NONCACHE_START) ? 0x00020000 : 0;
LCDC_FRAME_BUFFER_OFFSET           = ((0 != DDR_MIRROR0_PRV_START) || (0 != DDR_MIRROR1_PRV_START)) ? 0x04000000 : 0;

/* Calculate the usage of the noncache region. For multi-core projects, the calculation also takes into account the usage on the primary core. */
NONCACHE_USAGE                     = (DEFINED(NONCACHE_START) && (NONCACHE_OFFSET >= NONCACHE_PRV_LENGTH)) ? ALIGN((NONCACHE_OFFSET - NONCACHE_PRV_LENGTH), 0x20) : 0;

/* Calculate the start address of the noncache region. */
NONCACHE_AREA_INIT_START           = NONCACHE_PRV_START + NONCACHE_PRV_LENGTH - NONCACHE_OFFSET + NONCACHE_USAGE;
NONCACHE_AREA_INIT_LENGTH          = NONCACHE_OFFSET - SHARED_NONCACHE_BUFFER_OFFSET - NONCACHE_USAGE;

NONCACHE_AREA_START                = NONCACHE_AREA_INIT_START + SYSTEM_RAM_MIRROR_OFFSET;
NONCACHE_AREA_LENGTH               = NONCACHE_AREA_INIT_LENGTH;

/* Calculate the start address of the core common noncache region. */
SHARED_NONCACHE_BUFFER_INIT_START  = NONCACHE_PRV_START + NONCACHE_PRV_LENGTH - SHARED_NONCACHE_BUFFER_OFFSET;
SHARED_NONCACHE_BUFFER_INIT_LENGTH = SHARED_NONCACHE_BUFFER_OFFSET;

SHARED_NONCACHE_BUFFER_START       = SHARED_NONCACHE_BUFFER_INIT_START + SYSTEM_RAM_MIRROR_OFFSET;
SHARED_NONCACHE_BUFFER_LENGTH      = SHARED_NONCACHE_BUFFER_INIT_LENGTH;

/* Calculate the start address of the noncache region used by the LCDC. */
LCDC_FRAME_BUFFER_START            = (DEFINED(CR52_0) || DEFINED(CR52_1)) ?
                                         DDR_MIRROR1_PRV_START + DDR_MIRROR1_PRV_LENGTH - LCDC_FRAME_BUFFER_OFFSET :
                                         DDR_MIRROR0_PRV_START + DDR_MIRROR0_PRV_LENGTH - LCDC_FRAME_BUFFER_OFFSET;
LCDC_FRAME_BUFFER_LENGTH           = LCDC_FRAME_BUFFER_OFFSET;

/*============================================================================*/
/* M E M O R Y   R E G I O N                                                  */
/*============================================================================*/

MEMORY
{
    ATCM                   : ORIGIN = ATCM_PRV_START,                   LENGTH = ATCM_PRV_LENGTH
    BTCM                   : ORIGIN = BTCM_PRV_START,                   LENGTH = BTCM_PRV_LENGTH
    SYSTEM_RAM             : ORIGIN = SYSTEM_RAM_ALIGN_START,           LENGTH = SYSTEM_RAM_ALIGN_LENGTH
    CPU0_ATCM_VIA_AXIS     : ORIGIN = CPU0_ATCM_VIA_AXIS_PRV_START,     LENGTH = CPU0_ATCM_VIA_AXIS_PRV_LENGTH
    CPU0_BTCM_VIA_AXIS     : ORIGIN = CPU0_BTCM_VIA_AXIS_PRV_START,     LENGTH = CPU0_BTCM_VIA_AXIS_PRV_LENGTH
    CPU1_ATCM_VIA_AXIS     : ORIGIN = CPU1_ATCM_VIA_AXIS_PRV_START,     LENGTH = CPU1_ATCM_VIA_AXIS_PRV_LENGTH
    CPU1_BTCM_VIA_AXIS     : ORIGIN = CPU1_BTCM_VIA_AXIS_PRV_START,     LENGTH = CPU1_BTCM_VIA_AXIS_PRV_LENGTH
    SYSTEM_RAM_MIRROR      : ORIGIN = SYSTEM_RAM_MIRROR_PRV_START,      LENGTH = SYSTEM_RAM_MIRROR_PRV_LENGTH
    xSPI0_CS0_SPACE_MIRROR : ORIGIN = xSPI0_CS0_SPACE_MIRROR_PRV_START, LENGTH = xSPI0_CS0_SPACE_MIRROR_PRV_LENGTH
    xSPI0_CS1_SPACE_MIRROR : ORIGIN = xSPI0_CS1_SPACE_MIRROR_PRV_START, LENGTH = xSPI0_CS1_SPACE_MIRROR_PRV_LENGTH
    xSPI1_CS0_SPACE_MIRROR : ORIGIN = xSPI1_CS0_SPACE_MIRROR_PRV_START, LENGTH = xSPI1_CS0_SPACE_MIRROR_PRV_LENGTH
    xSPI1_CS1_SPACE_MIRROR : ORIGIN = xSPI1_CS1_SPACE_MIRROR_PRV_START, LENGTH = xSPI1_CS1_SPACE_MIRROR_PRV_LENGTH
    CS0_SPACE_MIRROR       : ORIGIN = CS0_SPACE_MIRROR_PRV_START,       LENGTH = CS0_SPACE_MIRROR_PRV_LENGTH
    CS2_SPACE_MIRROR       : ORIGIN = CS2_SPACE_MIRROR_PRV_START,       LENGTH = CS2_SPACE_MIRROR_PRV_LENGTH
    CS3_SPACE_MIRROR       : ORIGIN = CS3_SPACE_MIRROR_PRV_START,       LENGTH = CS3_SPACE_MIRROR_PRV_LENGTH
    CS5_SPACE_MIRROR       : ORIGIN = CS5_SPACE_MIRROR_PRV_START,       LENGTH = CS5_SPACE_MIRROR_PRV_LENGTH
    xSPI0_CS0_SPACE        : ORIGIN = xSPI0_CS0_SPACE_PRV_START,        LENGTH = xSPI0_CS0_SPACE_PRV_LENGTH
    xSPI0_CS1_SPACE        : ORIGIN = xSPI0_CS1_SPACE_PRV_START,        LENGTH = xSPI0_CS1_SPACE_PRV_LENGTH
    xSPI1_CS0_SPACE        : ORIGIN = xSPI1_CS0_SPACE_PRV_START,        LENGTH = xSPI1_CS0_SPACE_PRV_LENGTH
    xSPI1_CS1_SPACE        : ORIGIN = xSPI1_CS1_SPACE_PRV_START,        LENGTH = xSPI1_CS1_SPACE_PRV_LENGTH
    CS0_SPACE              : ORIGIN = CS0_SPACE_PRV_START,              LENGTH = CS0_SPACE_PRV_LENGTH
    CS2_SPACE              : ORIGIN = CS2_SPACE_PRV_START,              LENGTH = CS2_SPACE_PRV_LENGTH
    CS3_SPACE              : ORIGIN = CS3_SPACE_PRV_START,              LENGTH = CS3_SPACE_PRV_LENGTH
    CS5_SPACE              : ORIGIN = CS5_SPACE_PRV_START,              LENGTH = CS5_SPACE_PRV_LENGTH
    PCIE0_MIRROR           : ORIGIN = PCIE0_MIRROR_PRV_START,           LENGTH = PCIE0_MIRROR_PRV_LENGTH
    PCIE1_MIRROR           : ORIGIN = PCIE1_MIRROR_PRV_START,           LENGTH = PCIE1_MIRROR_PRV_LENGTH
    DDR_MIRROR0            : ORIGIN = DDR_MIRROR0_PRV_START,            LENGTH = DDR_MIRROR0_PRV_LENGTH
    DDR_MIRROR1            : ORIGIN = DDR_MIRROR1_PRV_START,            LENGTH = DDR_MIRROR1_PRV_LENGTH
    DDR                    : ORIGIN = DDR_PRV_START,                    LENGTH = DDR_PRV_LENGTH
    PCIE0                  : ORIGIN = PCIE0_PRV_START,                  LENGTH = PCIE0_PRV_LENGTH
    PCIE1                  : ORIGIN = PCIE1_PRV_START,                  LENGTH = PCIE1_PRV_LENGTH

    /* CPU1: every section of this application is placed with "> SYSTEM_RAM"
     * inside SECTIONS below, so it shares ONE allocation pointer.  The regions
     * below still exist because the stock file defines them, but INTVEC /
     * RAM_TEXT / RAM_DATA / LOADER_TEXT_STACK / LOADER_DATA_STACK are no
     * longer used as placement targets - they overlap SYSTEM_RAM while each
     * keeping its own pointer, which silently stacks sections on top of one
     * another. */
    INTVEC                 : ORIGIN = INTVEC_START,                     LENGTH = INTVEC_LENGTH
    RAM_TEXT               : ORIGIN = RAM_START,                        LENGTH = RAM_LENGTH
    RAM_DATA               : ORIGIN = RAM_START,                        LENGTH = RAM_LENGTH
    LOADER_TEXT_STACK      : ORIGIN = LOADER_START,                     LENGTH = LOADER_LENGTH
    LOADER_DATA_STACK      : ORIGIN = LOADER_START,                     LENGTH = LOADER_LENGTH
    NONCACHE               : ORIGIN = NONCACHE_AREA_START,              LENGTH = NONCACHE_AREA_LENGTH
    SHARED_NONCACHE_BUFFER : ORIGIN = SHARED_NONCACHE_BUFFER_START,     LENGTH = SHARED_NONCACHE_BUFFER_LENGTH
    LCDC_FRAME_BUFFER      : ORIGIN = LCDC_FRAME_BUFFER_START,          LENGTH = LCDC_FRAME_BUFFER_LENGTH
}

/*============================================================================*/
/* S E C T I O N   L A Y O U T                                                */
/*============================================================================*/

SECTIONS
{
/*-----------------------------------------------*/
/*  VECTOR TABLE                                 */
/*-----------------------------------------------*/
    /* Vector table plus the reset handler, same as the stock file.  Placed
     * FIRST, at INTVEC_ADDRESS = 0x10000000, where the SSBL branches
     * (APP MOD 3). */
    .intvec INTVEC_ADDRESS :
    {
        _fvector_start = .;
        KEEP(*(.intvec))
        KEEP(*(.reset_handler))
        . = ALIGN(0x80);
        KEEP(*(.intvec_0x080))
        . = ALIGN(0x80);
        KEEP(*(.intvec_0x100))
        . = ALIGN(0x80);
        KEEP(*(.intvec_0x180))
        . = ALIGN(0x80);
        KEEP(*(.intvec_0x200))
        . = ALIGN(0x80);
        KEEP(*(.intvec_0x280))
        . = ALIGN(0x80);
        KEEP(*(.intvec_0x300))
        . = ALIGN(0x80);
        KEEP(*(.intvec_0x380))
        . = ALIGN(0x80);
        KEEP(*(.intvec_0x400))
        . = ALIGN(0x80);
        KEEP(*(.intvec_0x480))
        . = ALIGN(0x80);
        KEEP(*(.intvec_0x500))
        . = ALIGN(0x80);
        KEEP(*(.intvec_0x580))
        . = ALIGN(0x80);
        KEEP(*(.intvec_0x600))
        . = ALIGN(0x80);
        KEEP(*(.intvec_0x680))
        . = ALIGN(0x80);
        KEEP(*(.intvec_0x700))
        . = ALIGN(0x80);
        KEEP(*(.intvec_0x780))
        _fvector_end = .;
    } > SYSTEM_RAM

/*-----------------------------------------------*/
/*  LOADER PROGRAM                               */
/*-----------------------------------------------*/
    /* Place the functions to be used in the loader program (SSBL). */
    .loader_text LOADER_TEXT_ADDRESS :
    {
        _loader_text_start = .;
        *(.loader_text)
        */startup_core.o(.text*)
        */system_core.o(.text*)
        */startup.o(.text*)
        */system.o(.text*)
        */all*/bsp_*.o(.text*)
        */r_ioport.o(.text*)
        KEEP(*(.warm_start))
        _loader_text_end = .;
    } > SYSTEM_RAM     /* >>> APP MOD 4 <<< stock: LOADER_TEXT_STACK (BTCM) */

    /* >>> APP MOD 5 <<< ------------------------------------------------------
     * Stock v4.2.0 splits this into .loader_data (loaded) + .loader_bss
     * (NOLOAD).  The working v3.1.0 script kept both in ONE loaded section,
     * so the whole loader block arrives in RAM as a single contiguous
     * blob from the SSBL.  That shape is reproduced here.  All four symbols the
     * v4.2.0 startup code expects are still defined:
     *   __loader_data_start / __loader_data_end / __loader_bss_start /
     *   __loader_bss_end
     * ----------------------------------------------------------------------*/
    .loader_data _loader_text_end :
    {
        __loader_data_start = .;
        */startup_core.o(.data*)
        */system_core.o(.data*)
        */startup.o(.data*)
        */system.o(.data*)
        */all*/bsp_*.o(.data*)
        */r_ioport.o(.data*)
        */bsp_mpu_core.o(.rodata*)
        */bsp_mmu_core.o(.rodata*)
        */bsp_register_protection.o(.rodata*)
        */bsp_tzc400.o(.rodata*)
        KEEP(*(.warm_start.data*))
        . = ALIGN(8);
        __loader_data_end = .;
    } > SYSTEM_RAM     /* >>> APP MOD 5 <<< stock: LOADER_DATA_STACK (BTCM) */

    /* Place the variables in the loader program that do not have initial values. */
    .loader_bss :
    {
        __loader_bss_start = .;
        */startup_core.o(.bss*)
        */system_core.o(.bss*)
        */startup.o(.bss*)
        */system.o(.bss*)
        */all*/bsp_*.o(.bss*)
        */r_ioport.o(.bss*)
        KEEP(*(.warm_start.bss*))
        KEEP(*(.ttbr))
        */startup_core.o(COMMON)
        */system_core.o(COMMON)
        */startup.o(COMMON)
        */system.o(COMMON)
        */all*/bsp_*.o(COMMON)
        */r_ioport.o(COMMON)
        . = ALIGN(8);
        __loader_bss_end = . ;

        KEEP(*(.software_reset)) /* Do not initialize */
    } > SYSTEM_RAM     /* >>> APP MOD 5b <<< stock: LOADER_DATA_STACK (BTCM) */

    /* Place the uninitialized variables. */
    /* Kept as a separate section: the startup code does not initialise
     * .noinit.  It is not NOLOAD (rule 3 in the header), and it is placed in
     * SYSTEM_RAM with the rest of the image, away from the SSBL's BTCM window. */
    .noinit :
    {
        . = ALIGN(8);
        __noinit_start = .;
        KEEP(*(.noinit*))
        __noinit_end = .;
    } > SYSTEM_RAM     /* >>> APP MOD 6 <<< stock: LOADER_DATA_STACK (BTCM) */

/*-----------------------------------------------*/
/*  USER PROGRAM                                 */
/*-----------------------------------------------*/
    /* Place the functions to be used in the user program (Programs other than SSBL). */
    .text ALIGN(0x100) :
    {
        _text_start = .;

        *(.text*)

        KEEP(*(.init))
        KEEP(*(.fini))

        /* .ctors */
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)
        _ctor_end = .;

        /* .dtors */
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)
        _dtor_end = .;

        KEEP(*(.eh_frame*))

        /* Place the variable vector table. */
        _rvectors_start = .;
        KEEP(*(.rvectors))
        _rvectors_end = .;

        _text_end = .;
    } > SYSTEM_RAM

    /* Place the exception table (newlib). */
    .ARM.extab :
    {
        __extab_start = .;
        *(.ARM.extab* .gnu.linkonce.armextab.*)
        __extab_end = .;
    } > SYSTEM_RAM

     /* Place the exception index (newlib). */
    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > SYSTEM_RAM

    /* Place the variables to be used in the user program (Programs other than SSBL). */
    /* Since .data is placed immediately after .ARM.exidx, the start address is set to __exidx_end.
     * If you move the associated section to another area, you will need to modify this symbol.
     * For details, refer to the "Linker Script" chapter in the FSP documentation (BSP -> MCU Board Support Package). */
    .data __exidx_end :
    {
        _data_start = .;

        *(vtable)
        *(.data.*)
        *(.data)

        *(.rodata*)
        KEEP(*(.version))
        _erodata = .;

        . = ALIGN(8);
        /* preinit data */
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(8);
        /* init data */
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);

        . = ALIGN(8);
        /* finit data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);

        KEEP(*(.jcr*))

        . = ALIGN(8);

        /* Place the addresses of shared library functions. */
        __got_start = .;
        *(.got)
        *(.got.plt)
        __got_end = .;

        /* All data end */
        _data_end = .;
    } > SYSTEM_RAM

    /* Place the variables in the user program that do not have initial values. */
    /* >>> APP MOD 9 <<< loaded, not (NOLOAD) - same as the v3.1.0 script. */
    .bss :
    {
        . = ALIGN(8);
        __bss_start__ = .;
        _bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end__ = .;
        _ebss = .;
        _end = .;
    } > SYSTEM_RAM

/*-----------------------------------------------*/
/*  HEAP                                         */
/*-----------------------------------------------*/
    /* Place the variables to secure heap area. */
    .heap (NOLOAD) :
    {
        . = ALIGN(8);
        __HeapBase = .;
        KEEP(*(.heap))
        __HeapLimit = .;
    } > SYSTEM_RAM

/*-----------------------------------------------*/
/*  AArch64 STACK                                */
/*-----------------------------------------------*/
    /* Place the stacks used in AArch64. */
    .aarch64_stack (NOLOAD) :
    {
        . = ALIGN(8);
        __AArch64StackBase = .;
        KEEP(*(.aarch64_stack*))
        __AArch64StackLimit = .;
    } > SYSTEM_RAM

/*-----------------------------------------------*/
/*  THREAD STACK                                 */
/*-----------------------------------------------*/
    /* Place the stack mainly used in free-RTOS. */
    .thread_stack (NOLOAD):
    {
        . = ALIGN(8);
        __ThreadStackBase = .;
        KEEP(*(.stack*))
        __ThreadStackLimit = .;
    } > SYSTEM_RAM

/*-----------------------------------------------*/
/*  AArch32 STACK                                */
/*-----------------------------------------------*/
    /* Place the stacks used in AArch32. */
    /* >>> CPU1 MOD 1 <<< ----------------------------------------------------
     * Stock places this in LOADER_DATA_STACK.  With the loader program moved
     * to SYSTEM_RAM that MEMORY region now OVERLAPS SYSTEM_RAM but keeps its
     * OWN allocation pointer, so the stacks would be laid down at 0x10000000 -
     * straight on top of the vector table and the whole program.  GNU ld does
     * not diagnose overlap for NOLOAD sections, so this would have linked
     * cleanly and died on the first push.  Put them in SYSTEM_RAM instead, so
     * they follow everything else.
     * ----------------------------------------------------------------------*/
    .loader_stack (NOLOAD) :
    {
        /* Place the sys_stack here. */
        . = ALIGN(8);
        __SysStackBase = .;
        KEEP(*(.sys_stack))
        __SysStackLimit = .;

        /* Place the svc_stack here. */
        . = ALIGN(8);
        __SvcStackBase = .;
        KEEP(*(.svc_stack))
        __SvcStackLimit = .;

        /* Place the irq_stack here. */
        . = ALIGN(8);
        __IrqStackBase = .;
        KEEP(*(.irq_stack))
        __IrqStackLimit = .;

        /* Place the fiq_stack here. */
        . = ALIGN(8);
        __FiqStackBase = .;
        KEEP(*(.fiq_stack))
        __FiqStackLimit = .;

        /* Place the und_stack here. */
        . = ALIGN(8);
        __UndStackBase = .;
        KEEP(*(.und_stack))
        __UndStackLimit = .;

        /* Place the abt_stack here. */
        . = ALIGN(8);
        __AbtStackBase = .;
        KEEP(*(.abt_stack))
        __AbtStackLimit = .;
    } > SYSTEM_RAM     /* >>> CPU1 MOD 1 <<< stock: LOADER_DATA_STACK */

/*-----------------------------------------------*/
/*  NONCACHE                                     */
/*-----------------------------------------------*/
    /* Place the noncache variables. */
    .noncache NONCACHE_AREA_START : AT(NONCACHE_AREA_INIT_START)
    {
        _noncache_start = .;

        /* Place the noncache variables. */
        . = ALIGN(8);
        _data_noncache_start = .;
        KEEP(*(.data_noncache*))
        _data_noncache_end = .;

        /* Place the noncache variables to be used in DMAC_LINK_MODE. */
        . = ALIGN(8);
        _dmac_link_mode_start = .;
        KEEP(*(.dmac_link_mode*))
        _dmac_link_mode_end = .;

        /* Place the core non-common noncache variables. */
        . = ALIGN(32);
        _ncbuffer_start = .;
        KEEP(*(.noncache_buffer*))
        _ncbuffer_end = .;

        _noncache_end = .;
    } > NONCACHE

    /* Place the core common noncache variables. */
    .shared_noncache_buffer SHARED_NONCACHE_BUFFER_START : AT(SHARED_NONCACHE_BUFFER_INIT_START)
    {
        _sncbuffer_start = .;
        . = ALIGN(32);
        KEEP(*(.shared_noncache_buffer*))
        _sncbuffer_end = .;
    } > SHARED_NONCACHE_BUFFER

    /* Place the noncache variables to be used in LCDC. */
    .lcdc_frame_buffer (NOLOAD) :
    {
        . = ALIGN(512);
        _lcdc_frame_buffer_start = .;
        KEEP(*(.lcdc_frame_buffer*))
        _lcdc_frame_buffer_end = .;
    } > LCDC_FRAME_BUFFER

/*-----------------------------------------------*/
/*  SECTION END ADDRESS                          */
/*-----------------------------------------------*/
    /* Specify the end address for each program. */
    /* >>> CPU1 MOD 2 <<< .loader_stack is now last in SYSTEM_RAM, so it marks
     * the end of the image for both the loader block and the user program. */
    LOADER_PROGRAM_END_ADDRESS_SYMBOL = __AbtStackLimit;
    USER_PROGRAM_END_ADDRESS_SYMBOL   = __AbtStackLimit;

    /* Specify the ATCM end address */
    ATCM_END_ADDRESS       = (0 == USER_PROGRAM_LOCATION_PRV) ? USER_PROGRAM_END_ADDRESS_SYMBOL :
                             (0 == LOADER_PROGRAM_LOCATION_PRV) ? LOADER_PROGRAM_END_ADDRESS_SYMBOL : ORIGIN(ATCM);

    /* Specify the BTCM end address */
    BTCM_END_ADDRESS       = (1 == USER_PROGRAM_LOCATION_PRV) ? USER_PROGRAM_END_ADDRESS_SYMBOL :
                             (1 == LOADER_PROGRAM_LOCATION_PRV) ? LOADER_PROGRAM_END_ADDRESS_SYMBOL : ORIGIN(BTCM);

    /* Specify the SYSTEM_RAM end address */
    SYSTEM_RAM_END_ADDRESS = (2 == USER_PROGRAM_LOCATION_PRV) ? USER_PROGRAM_END_ADDRESS_SYMBOL :
                             (2 == LOADER_PROGRAM_LOCATION_PRV) ? LOADER_PROGRAM_END_ADDRESS_SYMBOL : ORIGIN(SYSTEM_RAM);
}

/*============================================================================*/
/* E N T R Y   P O I N T   A N D   G U A R D S                                */
/*============================================================================*/

/* The SSBL is told to jump to --app_start_addr 10000000, and on CPU1 that
 * address is the RESET VECTOR, not the start of the startup code: the SSBL
 * releases CPU1 from reset, so the core must come up through .intvec ->
 * Reset_Handler.  Only Reset_Handler runs set_IMP_BTCMREGIONR and sets
 * g_bsp_software_reset_occurred; system_init (the entry used by CPU0) does
 * neither.  Pin the ELF entry point to the same address so a JTAG debug
 * session and an SSBL boot follow the identical code path.
 *
 * NOTE: the project's linker setting "Entry Point" (-Wl,-e...) OVERRIDES this
 * directive.  It must say -Wl,-e__Vectors, not the CPU0 value -Wl,-esystem_init.
 * Project Properties -> C/C++ Build -> Settings -> Linker -> Entry Point. */
ENTRY(__Vectors)

ASSERT(_fvector_start == 0x10000000,
       "ERROR: the vector table is not at SYSTEM_RAM 0x10000000 - the SSBL branches there.");

ASSERT(_fvector_end <= _loader_text_start,
       "ERROR: .intvec runs into .loader_text - enlarge the gap before LOADER_TEXT_ADDRESS.");

ASSERT(_text_start >= __noinit_end,
       "ERROR: .text overlaps the loader block.");

/*============================================================================*/
/* D D S C   S Y M B O L                                                      */
/*============================================================================*/

/* Defines the ddsc symbol, which is used in multi-core projects and to calculate the usage of each region.
 * If you change the memory layout, modify each __ddsc_***_END value to be the end address of that region. */

/* ATCM_CR52_0 region */
__ddsc_ATCM_CR52_0_START      = ORIGIN(ATCM);
__ddsc_ATCM_CR52_0_END        = ATCM_END_ADDRESS;

/* BTCM_CR52_0 region */
__ddsc_BTCM_CR52_0_START      = ORIGIN(BTCM);
__ddsc_BTCM_CR52_0_END        = BTCM_END_ADDRESS;

/* ATCM_CR52_1 region */
__ddsc_ATCM_CR52_1_START      = ORIGIN(ATCM);
__ddsc_ATCM_CR52_1_END        = ATCM_END_ADDRESS;

/* BTCM_CR52_1 region */
__ddsc_BTCM_CR52_1_START      = ORIGIN(BTCM);
__ddsc_BTCM_CR52_1_END        = BTCM_END_ADDRESS;

/* SYSTEM_RAM region */
__ddsc_SYSTEM_RAM_START       = ORIGIN(SYSTEM_RAM);
__ddsc_SYSTEM_RAM_END         = SYSTEM_RAM_END_ADDRESS;

/* NONCACHE region */
__ddsc_NONCACHE_START         = NONCACHE_AREA_START - SYSTEM_RAM_MIRROR_OFFSET;
__ddsc_NONCACHE_END           = DEFINED(NONCACHE_START) ? (_noncache_end - SYSTEM_RAM_MIRROR_OFFSET) : __ddsc_NONCACHE_START;

/* Used to determine whether a project is the primary project or not in a multi-core project. */
__ddsc_SECONDARY_START        = (1 == _RZ_ORDINAL) ? 0 : 1;
__ddsc_SECONDARY_END          = 1;