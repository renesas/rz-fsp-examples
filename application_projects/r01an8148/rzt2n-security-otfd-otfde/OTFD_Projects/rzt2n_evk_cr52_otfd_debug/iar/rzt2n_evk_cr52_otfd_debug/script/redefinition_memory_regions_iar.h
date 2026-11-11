/* The memory information for each device is done in memory regions file.
 * The starting address and length of memory not defined in memory regions file are defined as 0. */

if (isdefinedsymbol(ATCM_CR52_0_START))
{
    define symbol ATCM_PRV_START          = ATCM_CR52_0_START;
    define symbol ATCM_END_ADDRESS_OFFSET = 1;
}
else if (isdefinedsymbol(ATCM_CR52_1_START))
{
    define symbol ATCM_PRV_START          = ATCM_CR52_1_START;
    define symbol ATCM_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol ATCM_PRV_START          = 0;
    define symbol ATCM_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(ATCM_CR52_0_LENGTH))
{
    define symbol ATCM_PRV_LENGTH = ATCM_CR52_0_LENGTH;
}
else if (isdefinedsymbol(ATCM_CR52_1_LENGTH))
{
    define symbol ATCM_PRV_LENGTH = ATCM_CR52_1_LENGTH;
}
else
{
    define symbol ATCM_PRV_LENGTH = 0;
}

if (isdefinedsymbol(BTCM_CR52_0_START))
{
    define symbol BTCM_PRV_START          = BTCM_CR52_0_START;
    define symbol BTCM_END_ADDRESS_OFFSET = 1;
}
else if (isdefinedsymbol(BTCM_CR52_1_START))
{
    define symbol BTCM_PRV_START          = BTCM_CR52_1_START;
    define symbol BTCM_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol BTCM_PRV_START          = 0;
    define symbol BTCM_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(BTCM_CR52_0_LENGTH))
{
    define symbol BTCM_PRV_LENGTH = BTCM_CR52_0_LENGTH;
}
else if (isdefinedsymbol(BTCM_CR52_1_LENGTH))
{
    define symbol BTCM_PRV_LENGTH = BTCM_CR52_1_LENGTH;
}
else
{
    define symbol BTCM_PRV_LENGTH = 0;
}

if (isdefinedsymbol(SYSTEM_RAM_START))
{
    define symbol SYSTEM_RAM_PRV_START          = SYSTEM_RAM_START;
    define symbol SYSTEM_RAM_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol SYSTEM_RAM_PRV_START          = 0;
    define symbol SYSTEM_RAM_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(SYSTEM_RAM_LENGTH))
{
    define symbol SYSTEM_RAM_PRV_LENGTH = SYSTEM_RAM_LENGTH;
}
else
{
    define symbol SYSTEM_RAM_PRV_LENGTH = 0;
}

if (isdefinedsymbol(SYSTEM_RAM_MIRROR_START))
{
    define symbol SYSTEM_RAM_MIRROR_PRV_START          = SYSTEM_RAM_MIRROR_START;
    define symbol SYSTEM_RAM_MIRROR_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol SYSTEM_RAM_MIRROR_PRV_START          = 0;
    define symbol SYSTEM_RAM_MIRROR_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(SYSTEM_RAM_MIRROR_LENGTH))
{
    define symbol SYSTEM_RAM_MIRROR_PRV_LENGTH = SYSTEM_RAM_MIRROR_LENGTH;
}
else
{
    define symbol SYSTEM_RAM_MIRROR_PRV_LENGTH = 0;
}

if (isdefinedsymbol(xSPI0_CS0_SPACE_MIRROR_START))
{
    define symbol xSPI0_CS0_SPACE_MIRROR_PRV_START          = xSPI0_CS0_SPACE_MIRROR_START;
    define symbol xSPI0_CS0_SPACE_MIRROR_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol xSPI0_CS0_SPACE_MIRROR_PRV_START          = 0;
    define symbol xSPI0_CS0_SPACE_MIRROR_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(xSPI0_CS0_SPACE_MIRROR_LENGTH))
{
    define symbol xSPI0_CS0_SPACE_MIRROR_PRV_LENGTH = xSPI0_CS0_SPACE_MIRROR_LENGTH;
}
else
{
    define symbol xSPI0_CS0_SPACE_MIRROR_PRV_LENGTH = 0;
}

if (isdefinedsymbol(xSPI0_CS1_SPACE_MIRROR_START))
{
    define symbol xSPI0_CS1_SPACE_MIRROR_PRV_START          = xSPI0_CS1_SPACE_MIRROR_START;
    define symbol xSPI0_CS1_SPACE_MIRROR_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol xSPI0_CS1_SPACE_MIRROR_PRV_START          = 0;
    define symbol xSPI0_CS1_SPACE_MIRROR_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(xSPI0_CS1_SPACE_MIRROR_LENGTH))
{
    define symbol xSPI0_CS1_SPACE_MIRROR_PRV_LENGTH = xSPI0_CS1_SPACE_MIRROR_LENGTH;
}
else
{
    define symbol xSPI0_CS1_SPACE_MIRROR_PRV_LENGTH = 0;
}

if (isdefinedsymbol(xSPI1_CS0_SPACE_MIRROR_START))
{
    define symbol xSPI1_CS0_SPACE_MIRROR_PRV_START          = xSPI1_CS0_SPACE_MIRROR_START;
    define symbol xSPI1_CS0_SPACE_MIRROR_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol xSPI1_CS0_SPACE_MIRROR_PRV_START          = 0;
    define symbol xSPI1_CS0_SPACE_MIRROR_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(xSPI1_CS0_SPACE_MIRROR_LENGTH))
{
    define symbol xSPI1_CS0_SPACE_MIRROR_PRV_LENGTH = xSPI1_CS0_SPACE_MIRROR_LENGTH;
}
else
{
    define symbol xSPI1_CS0_SPACE_MIRROR_PRV_LENGTH = 0;
}

if (isdefinedsymbol(xSPI1_CS1_SPACE_MIRROR_START))
{
    define symbol xSPI1_CS1_SPACE_MIRROR_PRV_START          = xSPI1_CS1_SPACE_MIRROR_START;
    define symbol xSPI1_CS1_SPACE_MIRROR_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol xSPI1_CS1_SPACE_MIRROR_PRV_START          = 0;
    define symbol xSPI1_CS1_SPACE_MIRROR_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(xSPI1_CS1_SPACE_MIRROR_LENGTH))
{
    define symbol xSPI1_CS1_SPACE_MIRROR_PRV_LENGTH = xSPI1_CS1_SPACE_MIRROR_LENGTH;
}
else
{
    define symbol xSPI1_CS1_SPACE_MIRROR_PRV_LENGTH = 0;
}

if (isdefinedsymbol(CS0_SPACE_MIRROR_START))
{
    define symbol CS0_SPACE_MIRROR_PRV_START          = CS0_SPACE_MIRROR_START;
    define symbol CS0_SPACE_MIRROR_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol CS0_SPACE_MIRROR_PRV_START          = 0;
    define symbol CS0_SPACE_MIRROR_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(CS0_SPACE_MIRROR_LENGTH))
{
    define symbol CS0_SPACE_MIRROR_PRV_LENGTH = CS0_SPACE_MIRROR_LENGTH;
}
else
{
    define symbol CS0_SPACE_MIRROR_PRV_LENGTH = 0;
}

if (isdefinedsymbol(CS2_SPACE_MIRROR_START))
{
    define symbol CS2_SPACE_MIRROR_PRV_START          = CS2_SPACE_MIRROR_START;
    define symbol CS2_SPACE_MIRROR_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol CS2_SPACE_MIRROR_PRV_START          = 0;
    define symbol CS2_SPACE_MIRROR_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(CS2_SPACE_MIRROR_LENGTH))
{
    define symbol CS2_SPACE_MIRROR_PRV_LENGTH = CS2_SPACE_MIRROR_LENGTH;
}
else
{
    define symbol CS2_SPACE_MIRROR_PRV_LENGTH = 0;
}

if (isdefinedsymbol(CS3_SPACE_MIRROR_START))
{
    define symbol CS3_SPACE_MIRROR_PRV_START          = CS3_SPACE_MIRROR_START;
    define symbol CS3_SPACE_MIRROR_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol CS3_SPACE_MIRROR_PRV_START          = 0;
    define symbol CS3_SPACE_MIRROR_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(CS3_SPACE_MIRROR_LENGTH))
{
    define symbol CS3_SPACE_MIRROR_PRV_LENGTH = CS3_SPACE_MIRROR_LENGTH;
}
else
{
    define symbol CS3_SPACE_MIRROR_PRV_LENGTH = 0;
}

if (isdefinedsymbol(CS5_SPACE_MIRROR_START))
{
    define symbol CS5_SPACE_MIRROR_PRV_START          = CS5_SPACE_MIRROR_START;
    define symbol CS5_SPACE_MIRROR_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol CS5_SPACE_MIRROR_PRV_START          = 0;
    define symbol CS5_SPACE_MIRROR_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(CS5_SPACE_MIRROR_LENGTH))
{
    define symbol CS5_SPACE_MIRROR_PRV_LENGTH = CS5_SPACE_MIRROR_LENGTH;
}
else
{
    define symbol CS5_SPACE_MIRROR_PRV_LENGTH = 0;
}

if (isdefinedsymbol(xSPI0_CS0_SPACE_START))
{
    define symbol xSPI0_CS0_SPACE_PRV_START          = xSPI0_CS0_SPACE_START;
    define symbol xSPI0_CS0_SPACE_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol xSPI0_CS0_SPACE_PRV_START          = 0;
    define symbol xSPI0_CS0_SPACE_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(xSPI0_CS0_SPACE_LENGTH))
{
    define symbol xSPI0_CS0_SPACE_PRV_LENGTH = xSPI0_CS0_SPACE_LENGTH;
}
else
{
    define symbol xSPI0_CS0_SPACE_PRV_LENGTH = 0;
}

if (isdefinedsymbol(xSPI0_CS1_SPACE_START))
{
    define symbol xSPI0_CS1_SPACE_PRV_START          = xSPI0_CS1_SPACE_START;
    define symbol xSPI0_CS1_SPACE_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol xSPI0_CS1_SPACE_PRV_START          = 0;
    define symbol xSPI0_CS1_SPACE_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(xSPI0_CS1_SPACE_LENGTH))
{
    define symbol xSPI0_CS1_SPACE_PRV_LENGTH = xSPI0_CS1_SPACE_LENGTH;
}
else
{
    define symbol xSPI0_CS1_SPACE_PRV_LENGTH = 0;
}

if (isdefinedsymbol(xSPI1_CS0_SPACE_START))
{
    define symbol xSPI1_CS0_SPACE_PRV_START          = xSPI1_CS0_SPACE_START;
    define symbol xSPI1_CS0_SPACE_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol xSPI1_CS0_SPACE_PRV_START          = 0;
    define symbol xSPI1_CS0_SPACE_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(xSPI1_CS0_SPACE_LENGTH))
{
    define symbol xSPI1_CS0_SPACE_PRV_LENGTH = xSPI1_CS0_SPACE_LENGTH;
}
else
{
    define symbol xSPI1_CS0_SPACE_PRV_LENGTH = 0;
}

if (isdefinedsymbol(xSPI1_CS1_SPACE_START))
{
    define symbol xSPI1_CS1_SPACE_PRV_START          = xSPI1_CS1_SPACE_START;
    define symbol xSPI1_CS1_SPACE_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol xSPI1_CS1_SPACE_PRV_START          = 0;
    define symbol xSPI1_CS1_SPACE_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(xSPI1_CS1_SPACE_LENGTH))
{
    define symbol xSPI1_CS1_SPACE_PRV_LENGTH = xSPI1_CS1_SPACE_LENGTH;
}
else
{
    define symbol xSPI1_CS1_SPACE_PRV_LENGTH = 0;
}

if (isdefinedsymbol(CS0_SPACE_START))
{
    define symbol CS0_SPACE_PRV_START          = CS0_SPACE_START;
    define symbol CS0_SPACE_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol CS0_SPACE_PRV_START          = 0;
    define symbol CS0_SPACE_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(CS0_SPACE_LENGTH))
{
    define symbol CS0_SPACE_PRV_LENGTH = CS0_SPACE_LENGTH;
}
else
{
    define symbol CS0_SPACE_PRV_LENGTH = 0;
}

if (isdefinedsymbol(CS2_SPACE_START))
{
    define symbol CS2_SPACE_PRV_START          = CS2_SPACE_START;
    define symbol CS2_SPACE_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol CS2_SPACE_PRV_START          = 0;
    define symbol CS2_SPACE_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(CS2_SPACE_LENGTH))
{
    define symbol CS2_SPACE_PRV_LENGTH = CS2_SPACE_LENGTH;
}
else
{
    define symbol CS2_SPACE_PRV_LENGTH = 0;
}

if (isdefinedsymbol(CS3_SPACE_START))
{
    define symbol CS3_SPACE_PRV_START          = CS3_SPACE_START;
    define symbol CS3_SPACE_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol CS3_SPACE_PRV_START          = 0;
    define symbol CS3_SPACE_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(CS3_SPACE_LENGTH))
{
    define symbol CS3_SPACE_PRV_LENGTH = CS3_SPACE_LENGTH;
}
else
{
    define symbol CS3_SPACE_PRV_LENGTH = 0;
}

if (isdefinedsymbol(CS5_SPACE_START))
{
    define symbol CS5_SPACE_PRV_START          = CS5_SPACE_START;
    define symbol CS5_SPACE_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol CS5_SPACE_PRV_START          = 0;
    define symbol CS5_SPACE_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(CS5_SPACE_LENGTH))
{
    define symbol CS5_SPACE_PRV_LENGTH = CS5_SPACE_LENGTH;
}
else
{
    define symbol CS5_SPACE_PRV_LENGTH = 0;
}

if (isdefinedsymbol(PCIE0_MIRROR_START))
{
    define symbol PCIE0_MIRROR_PRV_START          = PCIE0_MIRROR_START;
    define symbol PCIE0_MIRROR_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol PCIE0_MIRROR_PRV_START          = 0;
    define symbol PCIE0_MIRROR_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(PCIE0_MIRROR_LENGTH))
{
    define symbol PCIE0_MIRROR_PRV_LENGTH = PCIE0_MIRROR_LENGTH;
}
else
{
    define symbol PCIE0_MIRROR_PRV_LENGTH = 0;
}

if (isdefinedsymbol(PCIE1_MIRROR_START))
{
    define symbol PCIE1_MIRROR_PRV_START          = PCIE1_MIRROR_START;
    define symbol PCIE1_MIRROR_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol PCIE1_MIRROR_PRV_START          = 0;
    define symbol PCIE1_MIRROR_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(PCIE1_MIRROR_LENGTH))
{
    define symbol PCIE1_MIRROR_PRV_LENGTH = PCIE1_MIRROR_LENGTH;
}
else
{
    define symbol PCIE1_MIRROR_PRV_LENGTH = 0;
}

if (isdefinedsymbol(DDR_MIRROR0_CR52_0_START))
{
    define symbol DDR_MIRROR0_PRV_START          = DDR_MIRROR0_CR52_0_START;
    define symbol DDR_MIRROR0_END_ADDRESS_OFFSET = 1;
}
else if (isdefinedsymbol(DDR_MIRROR0_CR52_1_START))
{
    define symbol DDR_MIRROR0_PRV_START          = DDR_MIRROR0_CR52_1_START;
    define symbol DDR_MIRROR0_END_ADDRESS_OFFSET = 1;
}
else if (isdefinedsymbol(DDR_MIRROR0_CA55_0_START))
{
    define symbol DDR_MIRROR0_PRV_START          = DDR_MIRROR0_CA55_0_START;
    define symbol DDR_MIRROR0_END_ADDRESS_OFFSET = 1;
}
else if (isdefinedsymbol(DDR_MIRROR0_CA55_1_START))
{
    define symbol DDR_MIRROR0_PRV_START          = DDR_MIRROR0_CA55_1_START;
    define symbol DDR_MIRROR0_END_ADDRESS_OFFSET = 1;
}
else if (isdefinedsymbol(DDR_MIRROR0_CA55_2_START))
{
    define symbol DDR_MIRROR0_PRV_START          = DDR_MIRROR0_CA55_2_START;
    define symbol DDR_MIRROR0_END_ADDRESS_OFFSET = 1;
}
else if (isdefinedsymbol(DDR_MIRROR0_CA55_3_START))
{
    define symbol DDR_MIRROR0_PRV_START          = DDR_MIRROR0_CA55_3_START;
    define symbol DDR_MIRROR0_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol DDR_MIRROR0_PRV_START          = 0;
    define symbol DDR_MIRROR0_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(DDR_MIRROR0_CR52_0_LENGTH))
{
    define symbol DDR_MIRROR0_PRV_LENGTH = DDR_MIRROR0_CR52_0_LENGTH;
}
else if (isdefinedsymbol(DDR_MIRROR0_CR52_1_LENGTH))
{
    define symbol DDR_MIRROR0_PRV_LENGTH = DDR_MIRROR0_CR52_1_LENGTH;
}
else if (isdefinedsymbol(DDR_MIRROR0_CA55_0_LENGTH))
{
    define symbol DDR_MIRROR0_PRV_LENGTH = DDR_MIRROR0_CA55_0_LENGTH;
}
else if (isdefinedsymbol(DDR_MIRROR0_CA55_1_LENGTH))
{
    define symbol DDR_MIRROR0_PRV_LENGTH = DDR_MIRROR0_CA55_1_LENGTH;
}
else if (isdefinedsymbol(DDR_MIRROR0_CA55_2_LENGTH))
{
    define symbol DDR_MIRROR0_PRV_LENGTH = DDR_MIRROR0_CA55_2_LENGTH;
}
else if (isdefinedsymbol(DDR_MIRROR0_CA55_3_LENGTH))
{
    define symbol DDR_MIRROR0_PRV_LENGTH = DDR_MIRROR0_CA55_3_LENGTH;
}
else
{
    define symbol DDR_MIRROR0_PRV_LENGTH = 0;
}

if (isdefinedsymbol(DDR_MIRROR1_CR52_0_START))
{
    define symbol DDR_MIRROR1_PRV_START          = DDR_MIRROR1_CR52_0_START;
    define symbol DDR_MIRROR1_END_ADDRESS_OFFSET = 1;
}
else if (isdefinedsymbol(DDR_MIRROR1_CR52_1_START))
{
    define symbol DDR_MIRROR1_PRV_START          = DDR_MIRROR1_CR52_1_START;
    define symbol DDR_MIRROR1_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol DDR_MIRROR1_PRV_START          = 0;
    define symbol DDR_MIRROR1_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(DDR_MIRROR1_CR52_0_LENGTH))
{
    define symbol DDR_MIRROR1_PRV_LENGTH = DDR_MIRROR1_CR52_0_LENGTH;
}
else if (isdefinedsymbol(DDR_MIRROR1_CR52_1_LENGTH))
{
    define symbol DDR_MIRROR1_PRV_LENGTH = DDR_MIRROR1_CR52_1_LENGTH;
}
else
{
    define symbol DDR_MIRROR1_PRV_LENGTH = 0;
}

if (isdefinedsymbol(DDR_CA55_0_START))
{
    define symbol DDR_PRV_START          = DDR_CA55_0_START;
    define symbol DDR_END_ADDRESS_OFFSET = 1;
}
else if (isdefinedsymbol(DDR_CA55_1_START))
{
    define symbol DDR_PRV_START          = DDR_CA55_1_START;
    define symbol DDR_END_ADDRESS_OFFSET = 1;
}
else if (isdefinedsymbol(DDR_CA55_2_START))
{
    define symbol DDR_PRV_START          = DDR_CA55_2_START;
    define symbol DDR_END_ADDRESS_OFFSET = 1;
}
else if (isdefinedsymbol(DDR_CA55_3_START))
{
    define symbol DDR_PRV_START          = DDR_CA55_3_START;
    define symbol DDR_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol DDR_PRV_START          = 0;
    define symbol DDR_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(DDR_CA55_0_LENGTH))
{
    define symbol DDR_PRV_LENGTH = DDR_CA55_0_LENGTH;
}
else if (isdefinedsymbol(DDR_CA55_1_LENGTH))
{
    define symbol DDR_PRV_LENGTH = DDR_CA55_1_LENGTH;
}
else if (isdefinedsymbol(DDR_CA55_2_LENGTH))
{
    define symbol DDR_PRV_LENGTH = DDR_CA55_2_LENGTH;
}
else if (isdefinedsymbol(DDR_CA55_3_LENGTH))
{
    define symbol DDR_PRV_LENGTH = DDR_CA55_3_LENGTH;
}
else
{
    define symbol DDR_PRV_LENGTH = 0;
}

if (isdefinedsymbol(PCIE0_CA55_0_START))
{
    define symbol PCIE0_PRV_START          = PCIE0_CA55_0_START;
    define symbol PCIE0_END_ADDRESS_OFFSET = 1;
}
else if (isdefinedsymbol(PCIE0_CA55_1_START))
{
    define symbol PCIE0_PRV_START          = PCIE0_CA55_1_START;
    define symbol PCIE0_END_ADDRESS_OFFSET = 1;
}
else if (isdefinedsymbol(PCIE0_CA55_2_START))
{
    define symbol PCIE0_PRV_START          = PCIE0_CA55_2_START;
    define symbol PCIE0_END_ADDRESS_OFFSET = 1;
}
else if (isdefinedsymbol(PCIE0_CA55_3_START))
{
    define symbol PCIE0_PRV_START          = PCIE0_CA55_3_START;
    define symbol PCIE0_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol PCIE0_PRV_START          = 0;
    define symbol PCIE0_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(PCIE0_CA55_0_LENGTH))
{
    define symbol PCIE0_PRV_LENGTH = PCIE0_CA55_0_LENGTH;
}
else if (isdefinedsymbol(PCIE0_CA55_1_LENGTH))
{
    define symbol PCIE0_PRV_LENGTH = PCIE0_CA55_1_LENGTH;
}
else if (isdefinedsymbol(PCIE0_CA55_2_LENGTH))
{
    define symbol PCIE0_PRV_LENGTH = PCIE0_CA55_2_LENGTH;
}
else if (isdefinedsymbol(PCIE0_CA55_3_LENGTH))
{
    define symbol PCIE0_PRV_LENGTH = PCIE0_CA55_3_LENGTH;
}
else
{
    define symbol PCIE0_PRV_LENGTH = 0;
}

if (isdefinedsymbol(PCIE1_CA55_0_START))
{
    define symbol PCIE1_PRV_START          = PCIE1_CA55_0_START;
    define symbol PCIE1_END_ADDRESS_OFFSET = 1;
}
else if (isdefinedsymbol(PCIE1_CA55_1_START))
{
    define symbol PCIE1_PRV_START          = PCIE1_CA55_1_START;
    define symbol PCIE1_END_ADDRESS_OFFSET = 1;
}
else if (isdefinedsymbol(PCIE1_CA55_2_START))
{
    define symbol PCIE1_PRV_START          = PCIE1_CA55_2_START;
    define symbol PCIE1_END_ADDRESS_OFFSET = 1;
}
else if (isdefinedsymbol(PCIE1_CA55_3_START))
{
    define symbol PCIE1_PRV_START          = PCIE1_CA55_3_START;
    define symbol PCIE1_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol PCIE1_PRV_START          = 0;
    define symbol PCIE1_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(PCIE1_CA55_0_LENGTH))
{
    define symbol PCIE1_PRV_LENGTH = PCIE1_CA55_0_LENGTH;
}
else if (isdefinedsymbol(PCIE1_CA55_1_LENGTH))
{
    define symbol PCIE1_PRV_LENGTH = PCIE1_CA55_1_LENGTH;
}
else if (isdefinedsymbol(PCIE1_CA55_2_LENGTH))
{
    define symbol PCIE1_PRV_LENGTH = PCIE1_CA55_2_LENGTH;
}
else if (isdefinedsymbol(PCIE1_CA55_3_LENGTH))
{
    define symbol PCIE1_PRV_LENGTH = PCIE1_CA55_3_LENGTH;
}
else
{
    define symbol PCIE1_PRV_LENGTH = 0;
}

if (isdefinedsymbol(NONCACHE_START))
{
    define symbol NONCACHE_PRV_START          = NONCACHE_START;
    define symbol NONCACHE_END_ADDRESS_OFFSET = 1;
}
else
{
    define symbol NONCACHE_PRV_START          = 0;
    define symbol NONCACHE_END_ADDRESS_OFFSET = 0;
}

if (isdefinedsymbol(NONCACHE_LENGTH))
{
    define symbol NONCACHE_PRV_LENGTH = NONCACHE_LENGTH;
}
else
{
    define symbol NONCACHE_PRV_LENGTH = 0;
}

if (isdefinedsymbol(SECONDARY))
{
    define symbol SECONDARY_PRV = SECONDARY;
}
else
{
    define symbol SECONDARY_PRV = 0;
}
