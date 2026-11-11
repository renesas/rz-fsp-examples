echo "Post build script started"

MODE="${4:-DEFAULT}"
case "${MODE}" in

# Legacy post-build flow.
DEFAULT)

  # Determine whether it is primary or not
  cproject_file="../.cproject"
  ordinal_definition=$(grep -o "[_A-Z]*ORDINAL=1" "$cproject_file" | head -n 1)

  boot_cfg_file="../rz_cfg/fsp_cfg/bsp/board_cfg.h"

  esd_boot_definition=$(grep "BSP_CFG_ESD_BOOT" "$boot_cfg_file")
  is_esd_boot=$(echo "$esd_boot_definition" | cut -d ' ' -f 3)

  emmc_boot_definition=$(grep "BSP_CFG_EMMC_BOOT" "$boot_cfg_file")
  is_emmc_boot=$(echo "$emmc_boot_definition" | cut -d ' ' -f 3)

  # If it is primary, execute the binary generation for eSD boot or eMMC boot.
  if [ -n "$ordinal_definition" ]; then

    # Generate files related to eSD boot.
    if [ "$is_esd_boot" = "1" ] || [ "$is_esd_boot" = "(1)" ]; then
      if [ -d ../rz/fsp/src/bsp/mcu/all/cr ]; then
        # Generate an image file for writing to an SD card
        arm-none-eabi-objcopy -O binary ${ProjName}.elf ${ProjName}.bin

        # Generate a .o files to place the loader program in the spare area(1 - 6).
        arm-none-eabi-objcopy -I elf32-littlearm -O binary ${ProjName}.elf -j .loader_text loader.bin
        for i in $(seq 6); do
          arm-none-eabi-objcopy -I binary -O elf32-littlearm -B arm --prefix-symbols=loader${i} --rename-section .data=.loader${i},alloc,data,readonly,load,contents loader.bin loader${i}.o
        done
      elif [ -d ../rz/fsp/src/bsp/mcu/all/ca ]; then
        # Generate an image file for writing to an SD card
        aarch64-none-elf-objcopy -O binary ${ProjName}.elf ${ProjName}.bin

        # Generate a .o files to place the loader program in the spare area(1 - 6).
        aarch64-none-elf-objcopy -I elf64-littleaarch64 -O binary ${ProjName}.elf -j .loader_text loader.bin
        for i in $(seq 6); do
          aarch64-none-elf-objcopy -I binary -O elf64-littleaarch64 -B aarch64 --prefix-symbols=loader${i} --rename-section .data=.loader${i},alloc,data,readonly,load,contents loader.bin loader${i}.o
        done
      fi
    fi

    # Generate files related to eMMC boot.
    if [ "$is_emmc_boot" = "1" ] || [ "$is_emmc_boot" = "(1)" ]; then
      if [ -d ../rz/fsp/src/bsp/mcu/all/cr ]; then
        # Generate a .srec files to place the loader parameter and loader program
        arm-none-eabi-objcopy -I elf32-littlearm -O binary ${ProjName}.elf -j .loader_param -j .loader_text loader.bin
        arm-none-eabi-objcopy -I binary -O srec --srec-forceS3 loader.bin loader.srec --adjust-vma=0x0 --gap-fill=0x00

        # Generate a .srec files to place the user program
        arm-none-eabi-objcopy -I elf32-littlearm -O binary ${ProjName}.elf -R .loader_param -R .loader_text user.bin
        arm-none-eabi-objcopy -I binary -O srec --srec-forceS3 user.bin user.srec --adjust-vma=0x0 --gap-fill=0x00
      elif [ -d ../rz/fsp/src/bsp/mcu/all/ca ]; then
        # Generate a .srec files to place the loader parameter and loader program
        aarch64-none-elf-objcopy -I elf32-littlearm -O binary ${ProjName}.elf -j .loader_param -j .loader_text loader.bin
        aarch64-none-elf-objcopy -I binary -O srec --srec-forceS3 loader.bin loader.srec --adjust-vma=0x0 --gap-fill=0x00

        # Generate a .srec files to place the user program
        aarch64-none-elf-objcopy -I elf32-littlearm -O binary ${ProjName}.elf -R .loader_param -R .loader_text user.bin
        aarch64-none-elf-objcopy -I binary -O srec --srec-forceS3 user.bin user.srec --adjust-vma=0x0 --gap-fill=0x00
      fi
    fi
  fi

  if [ -d ../rz/fsp/src/bsp/mcu/all/cr ]; then
    if [ -d ../rz/fsp/src/bsp/mcu/rzt2m ] || [ -d ../rz/fsp/src/bsp/mcu/rzt2me ]; then
      # Generates binary and object files with all sections except .noncache and .shared_noncache_buffer.
      arm-none-eabi-objcopy -I elf32-littlearm -O binary ${ProjName}.elf -R .noncache -R .shared_noncache_buffer secondary_CR52.bin
      arm-none-eabi-objcopy -I binary -O elf32-littlearm -B arm --rename-section .data=.secondary,alloc,data,readonly,load,contents secondary_CR52.bin secondary_CR52.o

    elif [ -d ../rz/fsp/src/bsp/mcu/rzt2h ] || [ -d ../rz/fsp/src/bsp/mcu/rzt2n ]; then
      header_file="../rz_cfg/fsp_cfg/bsp/bsp_mcu_device_pn_cfg.h"
      core_definition=$(grep "BSP_CFG_CORE_CR52" "$header_file")
      core_num=$(echo "$core_definition" | cut -d ' ' -f 3)

      configurator_file="../.cproject"
      loader_location=$(grep "LOADER_PROGRAM_LOCATION" "$configurator_file")
      loader_location_num=$(echo "$loader_location" | cut -d '=' -f 5 | cut -c 1)
      if [ "$loader_location_num" = "" ]; then
        loader_location_num=1
      fi
      user_location=$(grep "USER_PROGRAM_LOCATION" "$configurator_file")
      user_location_num=$(echo "$user_location" | cut -d '=' -f 5 | cut -c 1)
      if [ "$user_location_num" = "" ]; then
        user_location_num=0
      fi

      if [ "$is_esd_boot" = "1" ] || [ "$is_esd_boot" = "(1)" ] || [ "$is_emmc_boot" = "1" ] || [ "$is_emmc_boot" = "(1)" ]; then
        FIRST_OPERATION="-j .image_info"
        NONCACHE="-j .noncache"
      else
        FIRST_OPERATION="-j NOSECTION"
        NONCACHE="-j NOSECTION"
      fi
      VECTOR="-j .intvec"
      IMAGE_INFO="-j .image_info"
      USER_PROGRAM="-j .text -j .ARM.extab -j .ARM.exidx -j .data"
      LOADER_PROGRAM="-j .loader_text -j .loader_data"
      SECONDARY="-j .secondary"
      NOSECTION="-j NOSECTION"
      if [ "$loader_location_num" = "0" ]; then
        if [ "$user_location_num" = "0" ]; then
          atcm_section="$VECTOR $USER_PROGRAM $LOADER_PROGRAM $FIRST_OPERATION $NONCACHE"
          btcm_section="$NOSECTION"
          systemram_section="$SECONDARY"
        elif [ "$user_location_num" = "1" ]; then
          atcm_section="$VECTOR $LOADER_PROGRAM $FIRST_OPERATION"
          btcm_section="$USER_PROGRAM $NONCACHE"
          systemram_section="$SECONDARY"
        elif [ "$user_location_num" = "2" ]; then
          atcm_section="$VECTOR $LOADER_PROGRAM"
          btcm_section="$NOSECTION"
          systemram_section="$IMAGE_INFO $USER_PROGRAM $SECONDARY $NONCACHE"
        fi
      elif [ "$loader_location_num" = "1" ]; then
        if [ "$user_location_num" = "0" ]; then
          atcm_section="$VECTOR $USER_PROGRAM $FIRST_OPERATION $NONCACHE"
          btcm_section="$LOADER_PROGRAM"
          systemram_section="$SECONDARY"
        elif [ "$user_location_num" = "1" ]; then
          atcm_section="$VECTOR $FIRST_OPERATION"
          btcm_section="$USER_PROGRAM $LOADER_PROGRAM $NONCACHE"
          systemram_section="$SECONDARY"
        elif [ "$user_location_num" = "2" ]; then
          atcm_section="$VECTOR"
          btcm_section="$LOADER_PROGRAM"
          systemram_section="$IMAGE_INFO $USER_PROGRAM $SECONDARY $NONCACHE"
        fi
      elif [ "$loader_location_num" = "2" ]; then
        if [ "$user_location_num" = "0" ]; then
          atcm_section="$VECTOR $USER_PROGRAM $NONCACHE"
          btcm_section="$NOSECTION"
          systemram_section="$IMAGE_INFO $LOADER_PROGRAM $SECONDARY"
        elif [ "$user_location_num" = "1" ]; then
          atcm_section="$VECTOR"
          btcm_section="$USER_PROGRAM $NONCACHE"
          systemram_section="$IMAGE_INFO $LOADER_PROGRAM $SECONDARY"
        elif [ "$user_location_num" = "2" ]; then
          atcm_section="$VECTOR"
          btcm_section="$NOSECTION"
          systemram_section="$IMAGE_INFO $USER_PROGRAM $LOADER_PROGRAM $SECONDARY $NONCACHE"
        fi
      fi

      if [ "$core_num" = "0" ] || [ "$core_num" = "(0)" ]; then
        # Generates binary and object files with sections located in the ATCM and BTCM of CR52_0.
        arm-none-eabi-objcopy -I elf32-littlearm -O binary ${ProjName}.elf $atcm_section secondary_atcm_CR52_0.bin
        arm-none-eabi-objcopy -I binary -O elf32-littlearm -B arm --rename-section .data=.secondary_atcm_cr520,alloc,data,readonly,load,contents secondary_atcm_CR52_0.bin secondary_atcm_CR52_0.o
        arm-none-eabi-objcopy -I elf32-littlearm -O binary ${ProjName}.elf $btcm_section secondary_btcm_CR52_0.bin
        arm-none-eabi-objcopy -I binary -O elf32-littlearm -B arm --rename-section .data=.secondary_btcm_cr520,alloc,data,readonly,load,contents secondary_btcm_CR52_0.bin secondary_btcm_CR52_0.o
        if [ -n "$AARCH64_OBJCOPY" ]; then
          "$AARCH64_OBJCOPY"aarch64-none-elf-objcopy -I binary -O elf64-littleaarch64 -B aarch64 --rename-section .data=.secondary_atcm_cr520,alloc,data,readonly,load,contents secondary_atcm_CR52_0.bin secondary_atcm_aarch64_CR52_0.o
          "$AARCH64_OBJCOPY"aarch64-none-elf-objcopy -I binary -O elf64-littleaarch64 -B aarch64 --rename-section .data=.secondary_btcm_cr520,alloc,data,readonly,load,contents secondary_btcm_CR52_0.bin secondary_btcm_aarch64_CR52_0.o
        fi
      else
        # Generates binary and object files with sections located in the ATCM and BTCM of CR52_1.
        arm-none-eabi-objcopy -I elf32-littlearm -O binary ${ProjName}.elf $atcm_section secondary_atcm_CR52_1.bin
        arm-none-eabi-objcopy -I binary -O elf32-littlearm -B arm --rename-section .data=.secondary_atcm_cr521,alloc,data,readonly,load,contents secondary_atcm_CR52_1.bin secondary_atcm_CR52_1.o
        arm-none-eabi-objcopy -I elf32-littlearm -O binary ${ProjName}.elf $btcm_section secondary_btcm_CR52_1.bin
        arm-none-eabi-objcopy -I binary -O elf32-littlearm -B arm --rename-section .data=.secondary_btcm_cr521,alloc,data,readonly,load,contents secondary_btcm_CR52_1.bin secondary_btcm_CR52_1.o
        if [ -n "$AARCH64_OBJCOPY" ]; then
          "$AARCH64_OBJCOPY"aarch64-none-elf-objcopy -I binary -O elf64-littleaarch64 -B aarch64 --rename-section .data=.secondary_atcm_cr521,alloc,data,readonly,load,contents secondary_atcm_CR52_1.bin secondary_atcm_aarch64_CR52_1.o
          "$AARCH64_OBJCOPY"aarch64-none-elf-objcopy -I binary -O elf64-littleaarch64 -B aarch64 --rename-section .data=.secondary_btcm_cr521,alloc,data,readonly,load,contents secondary_btcm_CR52_1.bin secondary_btcm_aarch64_CR52_1.o
        fi
      fi

      # Generate binary and object files with .secondary only. In the case of TCM placement, only .secondary is placed in the system SRAM, so it is generated separately.
      arm-none-eabi-objcopy -I elf32-littlearm -O binary ${ProjName}.elf $systemram_section secondary_systemram_CR52.bin
      arm-none-eabi-objcopy -I binary -O elf32-littlearm -B arm --rename-section .data=.secondary,alloc,data,readonly,load,contents secondary_systemram_CR52.bin secondary_systemram_CR52.o
      if [ -n "$AARCH64_OBJCOPY" ]; then
        "$AARCH64_OBJCOPY"aarch64-none-elf-objcopy -I binary -O elf64-littleaarch64 -B aarch64 --rename-section .data=.secondary,alloc,data,readonly,load,contents secondary_systemram_CR52.bin secondary_systemram_aarch64_CR52.o
      fi
    fi

    if [ "$is_esd_boot" != "1" ] && [ "$is_esd_boot" != "(1)" ] && [ "$is_emmc_boot" != "1" ] && [ "$is_emmc_boot" != "(1)" ]; then
      # Generate binary and object files with .noncache only. .shared_noncache_buffer is excluded because it is not expected to be defined on the secondary.
      arm-none-eabi-objcopy -I elf32-littlearm -O binary ${ProjName}.elf -j .noncache secondary_noncache_CR52.bin
      arm-none-eabi-objcopy -I binary -O elf32-littlearm -B arm --rename-section .data=.secondary_noncache,alloc,data,readonly,load,contents secondary_noncache_CR52.bin secondary_noncache_CR52.o
      if [ -n "$AARCH64_OBJCOPY" ]; then
        "$AARCH64_OBJCOPY"aarch64-none-elf-objcopy -I binary -O elf64-littleaarch64 -B aarch64 --rename-section .data=.secondary_noncache,alloc,data,readonly,load,contents secondary_noncache_CR52.bin secondary_noncache_aarch64_CR52.o
      fi
    fi
  elif [ -d ../rz/fsp/src/bsp/mcu/all/ca ]; then
    if [ "$is_esd_boot" = "1" ] || [ "$is_esd_boot" = "(1)" ] || [ "$is_emmc_boot" = "1" ] || [ "$is_emmc_boot" = "(1)" ]; then
      # Generates binary and object files with all sections except .noncache and .shared_noncache_buffer.
      aarch64-none-elf-objcopy -I elf64-littleaarch64 -O binary ${ProjName}.elf -R .shared_noncache_buffer secondary_CA55.bin
      aarch64-none-elf-objcopy -I binary -O elf64-littleaarch64 -B aarch64 --rename-section .data=.secondary,alloc,data,readonly,load,contents secondary_CA55.bin secondary_CA55.o
      aarch64-none-elf-objcopy -I binary -O elf32-littlearm -B arm --rename-section .data=.secondary,alloc,data,readonly,load,contents secondary_CA55.bin secondary_aarch32_CA55.o
    else
      # Generates binary and object files with all sections except .noncache and .shared_noncache_buffer.
      aarch64-none-elf-objcopy -I elf64-littleaarch64 -O binary ${ProjName}.elf -R .noncache -R .shared_noncache_buffer secondary_CA55.bin
      aarch64-none-elf-objcopy -I binary -O elf64-littleaarch64 -B aarch64 --rename-section .data=.secondary,alloc,data,readonly,load,contents secondary_CA55.bin secondary_CA55.o
      aarch64-none-elf-objcopy -I binary -O elf32-littlearm -B arm --rename-section .data=.secondary,alloc,data,readonly,load,contents secondary_CA55.bin secondary_aarch32_CA55.o

      # Generate binary and object files with .noncache only. .shared_noncache_buffer is excluded because it is not expected to be defined on the secondary.
      aarch64-none-elf-objcopy -I elf64-littleaarch64 -O binary ${ProjName}.elf -j .noncache secondary_noncache_CA55.bin
      aarch64-none-elf-objcopy -I binary -O elf64-littleaarch64 -B aarch64 --rename-section .data=.secondary_noncache,alloc,data,readonly,load,contents secondary_noncache_CA55.bin secondary_noncache_CA55.o
      aarch64-none-elf-objcopy -I binary -O elf32-littlearm -B arm --rename-section .data=.secondary_noncache,alloc,data,readonly,load,contents secondary_noncache_CA55.bin secondary_noncache_aarch32_CA55.o
    fi
  fi
  ;;

# Address-range-based post-build flow.
ADDRESS_RANGE_MODE)

  # Address range definitions used for object generation.
  # You can generate any object by manually adding regions.
  # Format:
  # StartAddress|EndAddress|SectionName|OutputName
  OBJECT_LIST="
0x00000000|0x00080000|.secondary_atcm_{CORTEX_LOWER}{CORE_NUM}|secondary_atcm_{CORTEX}_{CORE_NUM}
0x00100000|0x00110000|.secondary_btcm_{CORTEX_LOWER}{CORE_NUM}|secondary_btcm_{CORTEX}_{CORE_NUM}
0x10000000|0x10180000|.secondary|secondary
0x10180000|0x10200000|.secondary_noncache|secondary_noncache
"

  PROJECT_DIR=$(cd "$(dirname "$0")/.." && pwd)
  PROJECT_NAME=${PROJECT_DIR##*/}
  TARGET_FILE="$1"
  OUTPUT_DIR=$(dirname "$TARGET_FILE")
  GENERATE_BINARY_TOOL="$2"
  if [ -n "$AARCH64_OBJCOPY" ]; then
    AARCH64_OBJCOPY="${AARCH64_OBJCOPY}/aarch64-none-elf-objcopy.exe"
  elif [ -d "$3" ]; then
    AARCH64_OBJCOPY="$3/aarch64-none-elf-objcopy.exe"
  elif [ -f "$3" ]; then
    AARCH64_OBJCOPY="$3"
  else
    AARCH64_OBJCOPY=""
  fi

  main() {
    # Detect target information from configuration.xml.
    detect_target_info

    # Generate an intermediate SREC file from the ELF image.
    generate_input_srec

    if [ "$ORDINAL" = "1" ]; then
      if [ "$BOOT_MODE" = "eSD_boot" ] || [ "$BOOT_MODE" = "eMMC_boot" ]; then
        # Generate eSD/eMMC boot images from the generated SREC file.
        generate_esd_emmc_binary
      fi
    else
      # Replace template variables in OBJECT_LIST with the detected target-specific values.
      expand_object_list

      # Generate secondary image object files from address ranges.
      process_object_list
    fi
  }

  # Detect target information from configuration.xml.
  detect_target_info() {
    # Check if a configuration.xml file exists in the project.
    cfg_file="${PROJECT_DIR}/configuration.xml"
    if [ ! -f "$cfg_file" ]; then
      echo "Error: File not found: $cfg_file" >&2
      exit 1
    fi

    # Check if a memory_regions file exists in the project.
    memory_regions=$(ls ./memory_regions.* "$PROJECT_DIR"/memory_regions.* 2>/dev/null | head -n1)
    if [ ! -f "$memory_regions" ]; then
      echo "Error: File not found: $memory_regions" >&2
      exit 1
    fi

    # Detect tooling information
    toolchain=$(sed -n 's/.*key="#SELECTED_TOOLCHAIN#"[[:space:]]*value="\([^"]*\)".*/\1/p' "$cfg_file" | head -n 1)
    if [ -z "$GENERATE_BINARY_TOOL" ]; then
      if [ "$toolchain" = "gcc-arm-embedded" ]; then
        GENERATE_BINARY_TOOL=arm-none-eabi-objcopy
      elif [ "$toolchain" = "gcc-arm-a-profile-aarch64" ]; then
        GENERATE_BINARY_TOOL=aarch64-none-elf-objcopy
      elif [ "$toolchain" = "iar.arm.toolchain" ]; then
        GENERATE_BINARY_TOOL=ielftool
      fi
    fi
    case "$GENERATE_BINARY_TOOL" in
    *arm-none-eabi-objcopy*)
      TOOLCHAIN=gcc
      OBJCOPY=${GENERATE_BINARY_TOOL}
      OBJCOPY_AARCH=aarch32
      OBJCOPY_FORMAT=elf32-littlearm
      OBJCOPY_ARM=arm
      ;;
    *aarch64-none-elf-objcopy*)
      TOOLCHAIN=gcc
      OBJCOPY=${GENERATE_BINARY_TOOL}
      OBJCOPY_AARCH=aarch64
      OBJCOPY_FORMAT=elf64-littleaarch64
      OBJCOPY_ARM=aarch64
      ;;
    *ielftool*)
      TOOLCHAIN=iar
      OBJCOPY=""
      OBJCOPY_AARCH=""
      OBJCOPY_FORMAT=""
      OBJCOPY_ARM=""
      ;;
    *)
      echo "Error: Unsupported GENERATE_BINARY_TOOL [$GENERATE_BINARY_TOOL]" >&2
      exit 1
      ;;
    esac

    # Detect core information
    core=$(sed -n 's/.*key="Core"[[:space:]]*value="\([^"]*\)".*/\1/p' "$cfg_file" | head -n 1)
    case "$core" in
    CR52_*)
      CORTEX=CR52
      CORTEX_LOWER=cr52
      CORE_NUM=${core#CR52_}
      ;;
    CA55_*)
      CORTEX=CA55
      CORTEX_LOWER=ca55
      CORE_NUM=${core#CA55_}
      ;;
    *)
      echo "Unsupported core: $core" >&2
      exit 1
      ;;
    esac

    # Detect boot mode information
    BOOT_MODE=$(
      sed -n 's/.*key="#ConfigurationFragments#"[[:space:]]*value="\([^"]*\)".*/\1/p' "$cfg_file" |
        awk -F'##' '{print $NF}' |
        head -n 1
    )

    # Detect ORDINAL information (A value of 1 indicates primary project, and a value of 2 indicates secondary project.)
    secondary=$(sed -n 's/.*SECONDARY[[:space:]]*=[[:space:]]*\([^;]*\).*/\1/p' "$memory_regions")
    if [ "$secondary" = "0x00000000" ]; then
        ORDINAL=1
    elif [ "$secondary" = "0x00000001" ]; then
        ORDINAL=2
    else
        echo "Error: Unexpected secondary value [$secondary]"
        exit 1
    fi
  }

  # Generate an intermediate SREC file from the target ELF file.
  generate_input_srec() {
    if [ "$TOOLCHAIN" = "gcc" ]; then
      "${GENERATE_BINARY_TOOL}" -O srec "${TARGET_FILE}" "${OUTPUT_DIR}/${PROJECT_NAME}.srec"
    elif [ "$TOOLCHAIN" = "iar" ]; then
      "${GENERATE_BINARY_TOOL}" --srec "${TARGET_FILE}" "${OUTPUT_DIR}/${PROJECT_NAME}.srec"
    fi
  }

  # Generate eSD/eMMC boot images from the generated SREC file.
  generate_esd_emmc_binary() {
    # Check if a map file exists in the project.
    for map_file in \
      "${OUTPUT_DIR}/${PROJECT_NAME}.map" \
      "${OUTPUT_DIR}/../List/${PROJECT_NAME}.map"; do
      [ -f "$map_file" ] && break
    done
    [ -f "$map_file" ] || {
      echo "Map file not found." >&2
      exit 1
    }

    esd_emmc_start_addr=0x88000000
    esd_emmc_end_addr=0x94000000

    # Generate files related to eSD boot.
    if [ "$BOOT_MODE" = "eSD_boot" ]; then
      # Detect loader addresses from map files
      if [ "$TOOLCHAIN" = "gcc" ]; then
        loader_start_addr=$(awk '/^[[:space:]]*0x/ && /_flash_begin_address = \./ { gsub("'"'"'", "", $1); print $1; exit }' "$map_file")
        loader_end_addr=$(awk '/^[[:space:]]*0x/ && /_flash_begin_address \+ LOADER_PROGRAM_SIZE/ { gsub("'"'"'", "", $1); print $1; exit }' "$map_file")
      elif [ "$TOOLCHAIN" = "iar" ]; then
        loader_start_addr=$(awk '/^[[:space:]]*LDR_PRG_RBLOCK\$\$Base[[:space:]]/ { gsub("'"'"'", "", $2); print $2; exit }' "$map_file")
        loader_end_addr=$(awk '/^[[:space:]]*LDR_PRG_RBLOCK\$\$Limit[[:space:]]/ { gsub("'"'"'", "", $2); print $2; exit }' "$map_file")
      fi
      # Generate an image file for writing to an SD card
      srec_cat \
        "${OUTPUT_DIR}/${PROJECT_NAME}.srec" \
        -offset -"${esd_emmc_start_addr}" \
        -o "${OUTPUT_DIR}/${PROJECT_NAME}.bin" -binary || return 1

      # Generate a .o files to place the loader program in the spare area(1 - 6).
      srec_cat \
        "${OUTPUT_DIR}/${PROJECT_NAME}.srec" \
        -crop "${loader_start_addr}" "${loader_end_addr}" \
        -offset -"${loader_start_addr}" \
        -o "${OUTPUT_DIR}/loader.bin" -binary || return 1

      if [ "$TOOLCHAIN" = "gcc" ]; then
        i=1
        while [ "$i" -le 6 ]; do
          "${GENERATE_BINARY_TOOL}" \
            -I binary -O "${OBJCOPY_FORMAT}" -B "${OBJCOPY_ARM}" \
            --prefix-symbols=loader${i} \
            --rename-section .data=.loader${i},alloc,data,readonly,load,contents \
            "${OUTPUT_DIR}/loader.bin" \
            "${OUTPUT_DIR}/loader${i}.o" || return 1
          i=$((i + 1))
        done
      fi
    # Generate files related to eMMC boot.
    elif [ "$BOOT_MODE" = "eMMC_boot" ]; then
      # Detect loader addresses from map files
      if [ "$TOOLCHAIN" = "gcc" ]; then
        mbr_param_base=$(awk '/^[[:space:]]*\.mbr_param[[:space:]]/ { print $2; exit }' "$map_file")
      elif [ "$TOOLCHAIN" = "iar" ]; then
        mbr_param_base=$(awk '/^[[:space:]]*\.mbr_param\$\$Base[[:space:]]/ { gsub("'"'"'", "", $2); print $2; exit }' "$map_file")
      fi
      # Generate a .srec files to place the loader parameter and loader program
      srec_cat \
        "${OUTPUT_DIR}/${PROJECT_NAME}.srec" \
        -crop "${esd_emmc_start_addr}" "${mbr_param_base}" \
        -offset -"${esd_emmc_start_addr}" \
        -o "${OUTPUT_DIR}/loader.srec" -motorola \
        -address-length=4 \
        -Output_Block_Size 16 \
        -execution-start-address=0 || return 1

      # Generate a .srec files to place the user program
      srec_cat \
        "${OUTPUT_DIR}/${PROJECT_NAME}.srec" \
        -crop "${mbr_param_base}" "${esd_emmc_end_addr}" \
        -offset -"${mbr_param_base}" \
        -o "${OUTPUT_DIR}/user.srec" -motorola \
        -address-length=4 \
        -Output_Block_Size 16 \
        -execution-start-address=0 || return 1
    fi
  }

  # Replace template variables in OBJECT_LIST with the detected target-specific values.
  expand_object_list() {
    OBJECT_LIST=$(printf '%s\n' "$OBJECT_LIST" |
      sed \
        -e "s/{CORTEX_LOWER}/${CORTEX_LOWER}/g" \
        -e "s/{CORTEX}/${CORTEX}/g" \
        -e "s/{CORE_NUM}/${CORE_NUM}/g")
  }

  # For each row in OBJECT_LIST, execute generate_object.
  process_object_list() {
    while IFS='|' read -r start_addr end_addr section output_name; do
      if [ -n "$start_addr" ]; then
        generate_object \
          "$start_addr" \
          "$end_addr" \
          "$section" \
          "$output_name" &
      fi
    done << EOF
$OBJECT_LIST
EOF
    wait
  }

  # Generate secondary image object files from address ranges.
  generate_object() {
    start_addr="$1"
    end_addr="$2"
    section="$3"
    output="$4"

    # Extract the specified address range from the SREC file.
    tmp_srec="${OUTPUT_DIR}/${output}.tmp.srec"
    srec_cat \
      "${OUTPUT_DIR}/${PROJECT_NAME}.srec" \
      -crop "${start_addr}" "${end_addr}" \
      -o "${tmp_srec}" -motorola \
      -execution-start-address 0 || return 1

    # Determine the first valid address in the extracted image.
    min_addr=$(srec_info "${tmp_srec}" 2>&1 | awk '/Data:/ {print $2; exit}')
    if [ "${min_addr}" != "none" ]; then

      # Relocate the image to address zero and generate a BIN file.
      srec_cat \
        "${tmp_srec}" \
        -offset "-0x${min_addr}" \
        -o "${OUTPUT_DIR}/${output}.bin" -binary || return 1

      if [ "$TOOLCHAIN" = "gcc" ]; then
        # Convert the generated BIN file into an object file.
        "${OBJCOPY}" \
          -I binary -O "${OBJCOPY_FORMAT}" -B "${OBJCOPY_ARM}" \
          --rename-section .data=${section},alloc,data,readonly,load,contents \
          "${OUTPUT_DIR}/${output}.bin" \
          "${OUTPUT_DIR}/${output}.o" || return 1

        if [ "${CORTEX}" = "CR52" ]; then
          if [ -n "$AARCH64_OBJCOPY" ]; then
            # Generate an additional AArch64 object file when required.
            "${AARCH64_OBJCOPY}" \
              -I binary -O elf64-littleaarch64 -B aarch64 \
              --rename-section .data=${section},alloc,data,readonly,load,contents \
              "${OUTPUT_DIR}/${output}.bin" \
              "${OUTPUT_DIR}/${output}_aarch64.o" || return 1
          fi
        elif [ "${CORTEX}" = "CA55" ]; then
          # Generate an additional AArch32 object file.
          "${OBJCOPY}" \
            -I binary -O elf32-littlearm -B arm \
            --rename-section .data=${section},alloc,data,readonly,load,contents \
            "${OUTPUT_DIR}/${output}.bin" \
            "${OUTPUT_DIR}/${output}_aarch32.o" || return 1
        fi
      fi
    fi

    # Delete temporary files
    rm -f "${tmp_srec}"
  }

  # Execute main()
  main "$@"
  ;;

*)
  echo "Unknown mode: ${MODE}"
  ;;

esac

echo "Post build script complete"
