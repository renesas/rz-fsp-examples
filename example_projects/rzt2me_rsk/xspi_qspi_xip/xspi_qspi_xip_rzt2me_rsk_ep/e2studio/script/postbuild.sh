echo "Post build script started"

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

  elif [ -d ../rz/fsp/src/bsp/mcu/rzt2h ]; then
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

echo "Post build script complete"
