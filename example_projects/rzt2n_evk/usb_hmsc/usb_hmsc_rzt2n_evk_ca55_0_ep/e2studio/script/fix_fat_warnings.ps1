<#
    fix_fat_warnings.ps1
    --------------------
    Re-applies the warning fixes to Lab-Project-FreeRTOS-FAT.

    The RZ FSP Smart Configurator may restore the pristine middleware sources
    under rz\ when project content is generated or when the FSP pack is
    upgraded.  Running this script puts the fixes back.  It is idempotent:
    a patch that is already in place is skipped, so it is safe to run it
    before every build (Project > Properties > C/C++ Build > Settings >
    Build Steps > Pre-build steps).

    Usage:   powershell -NoProfile -ExecutionPolicy Bypass -File "<this file>"
    Options: -ProjectRoot <path>   (default: the parent of this script folder)

    Always exits with code 0 so that it can never break a build.
#>
param( [string] $ProjectRoot = '' )

$ErrorActionPreference = 'Continue'

if ( $ProjectRoot -eq '' )
{
    $ProjectRoot = Split-Path -Parent $PSScriptRoot
}

Write-Host "fix_fat_warnings: project root = $ProjectRoot"

$encoding = New-Object System.Text.UTF8Encoding( $false )
$applied  = 0
$alreadyOk = 0
$problems = 0

$patches = @(
    @{
        Rel    = 'rz/freertos/Lab-Project-FreeRTOS-FAT/include/ff_error.h'
        Name   = 'FF_isERR sign-conversion'
        Marker = '( ( ( unsigned ) ( x ) ) & FF_ERRFLAG )'
        Old    = '#define FF_isERR( x )            ( ( ( x ) & FF_ERRFLAG ) != 0 )'
        New    = @(
              '/* FF_ERRFLAG (0x80000000) has type unsigned int, while the argument is a',
              ' * signed FF_Error_t.  The explicit cast avoids -Wsign-conversion at every',
              ' * call site without changing the resulting value. */',
              '#define FF_isERR( x )            ( ( ( ( unsigned ) ( x ) ) & FF_ERRFLAG ) != 0 )') -join "`n"
        Count  = 1
    },
    @{
        Rel    = 'rz/freertos/Lab-Project-FreeRTOS-FAT/include/FreeRTOSFATConfigDefaults.h'
        Name   = 'FF_PRINTF no-op'
        Marker = 'ffconfigHAS_FF_PRINTF'
        Old    = @(
              '#if !defined( FF_PRINTF )',
              '    #define FF_PRINTF    FF_PRINTF') -join "`n"
        New    = @(
              '/* The generated FreeRTOSFATConfig.h defines FF_PRINTF as an *empty* macro,',
              ' * which turns every call site into a bare comma expression (-Wunused-value).',
              ' * Discard it and install a variadic no-op instead: the arguments stay used',
              ' * and no code is generated.  Define ffconfigHAS_FF_PRINTF as 1 to keep your',
              ' * own FF_PRINTF. */',
              '#if !defined( ffconfigHAS_FF_PRINTF ) || ( ffconfigHAS_FF_PRINTF == 0 )',
              '    #undef FF_PRINTF',
              '    #define FF_PRINTF    FF_PRINTF') -join "`n"
        Count  = 1
    },
    @{
        Rel    = 'rz/freertos/Lab-Project-FreeRTOS-FAT/ff_file.c'
        Name   = 'FF_Move ucMode mask'
        Marker = 'ucMode &= ( uint8_t ) ~('
        Old    = 'pSrcFile->ucMode &= ~( FF_MODE_WRITE | FF_MODE_APPEND | FF_MODE_CREATE );'
        New    = 'pSrcFile->ucMode &= ( uint8_t ) ~( FF_MODE_WRITE | FF_MODE_APPEND | FF_MODE_CREATE );'
        Count  = 1
    },
    @{
        Rel    = 'rz/freertos/Lab-Project-FreeRTOS-FAT/ff_file.c'
        Name   = 'FF_ExtendFile chain length'
        Marker = 'ulChainLength += ( uint32_t ) xIndex;'
        Old    = 'pxFile->ulChainLength += xIndex;'
        New    = @(
              '/* xIndex counts the clusters that were added, so it is never',
              '             * negative and always fits in a uint32_t. */',
              '            pxFile->ulChainLength += ( uint32_t ) xIndex;') -join "`n"
        Count  = 1
    },
    @{
        Rel    = 'rz/freertos/Lab-Project-FreeRTOS-FAT/ff_file.c'
        Name   = 'FF_SetPerm attrib mask'
        Marker = 'ucAttrib &= ( uint8_t ) ~FF_FAT_ATTR_USER;'
        Old    = 'xOriginalEntry.ucAttrib &= ~FF_FAT_ATTR_USER;'
        New    = 'xOriginalEntry.ucAttrib &= ( uint8_t ) ~FF_FAT_ATTR_USER;'
        Count  = 1
    },
    @{
        Rel    = 'rz/freertos/Lab-Project-FreeRTOS-FAT/ff_format.c'
        Name   = 'FF_Partition summed sizes'
        Marker = 'ulSummedSizes += ( uint32_t ) pParams'
        Old    = 'ulSummedSizes += pParams->xSizes[ xPartitionNumber ];'
        New    = 'ulSummedSizes += ( uint32_t ) pParams->xSizes[ xPartitionNumber ];'
        Count  = 1
    },
    @{
        Rel    = 'rz/freertos/Lab-Project-FreeRTOS-FAT/ff_fat.c'
        Name   = '#warning one FAT table'
        Marker = 'used to be a #warning directive'
        Old    = @(
              '    #if ffconfigBUF_STORE_COUNT != 2',
              '    #warning Only maintaining one FAT table',
              '    #endif') -join "`n"
        New    = @(
              '    #if ffconfigBUF_STORE_COUNT != 2',
              '        /* Note: with ffconfigBUF_STORE_COUNT != 2 only one FAT table is',
              '         * maintained.  This used to be a #warning directive, which -Wcpp',
              '         * reports on every single compilation. */',
              '    #endif') -join "`n"
        Count  = 1
    },
    @{
        Rel    = 'rz/freertos/Lab-Project-FreeRTOS-FAT/ff_fat.c'
        Name   = 'prvGetFAT12Entry return cast'
        Marker = '-Wsign-conversion warning without changing the value'
        Old    = '        return ( int32_t ) ulFATEntry;'
        New    = @(
              '        /* The function returns uint32_t, so the int32_t cast only produced a',
              '         * -Wsign-conversion warning without changing the value. */',
              '        return ulFATEntry;') -join "`n"
        Count  = 1
    },
    @{
        Rel    = 'rz/freertos/Lab-Project-FreeRTOS-FAT/ff_fat.c'
        Name   = 'FF_FindFreeCluster FAT offset'
        Marker = 'ulFATOffset += ( uint32_t ) xEntrySize;'
        Old    = 'ulFATOffset += xEntrySize;'
        New    = 'ulFATOffset += ( uint32_t ) xEntrySize;'
        Count  = 1
    },
    @{
        Rel    = 'rz/freertos/Lab-Project-FreeRTOS-FAT/ff_dir.c'
        Name   = 'strcasecmp prototype'
        Marker = 'const char * pcString2 );'
        Old    = @(
              '    int strcasecmp( const char * pcString1,',
              '                    const char * pcString2 )',
              '    {') -join "`n"
        New    = @(
              '    /* Prototype: strings.h is not pulled in here, and without a previous',
              '     * declaration a non-static definition triggers -Wmissing-declarations. */',
              '    int strcasecmp( const char * pcString1,',
              '                    const char * pcString2 );',
              '',
              '    int strcasecmp( const char * pcString1,',
              '                    const char * pcString2 )',
              '    {') -join "`n"
        Count  = 1
    },
    @{
        Rel    = 'rz/freertos/Lab-Project-FreeRTOS-FAT/ff_dir.c'
        Name   = 'usCurrentItem LFN count (x2)'
        Marker = 'usCurrentItem += ( uint16_t ) ( xLFNCount - 1 );'
        Old    = 'pxDirEntry->usCurrentItem += ( xLFNCount - 1 );'
        New    = 'pxDirEntry->usCurrentItem += ( uint16_t ) ( xLFNCount - 1 );'
        Count  = 0
    },
    @{
        Rel    = 'rz/freertos/Lab-Project-FreeRTOS-FAT/ff_ioman.c'
        Name   = 'ulFirstCluster set-but-not-used'
        Marker = '( void ) ulFirstCluster;'
        Old    = @(
              '            ulFirstCluster = FF_getLong( pxBuffer->pucBuffer, 0U );',
              '            xError = FF_ReleaseBuffer( pxIOManager, pxBuffer );',
              '        }',
              '    }',
              '') -join "`n"
        New    = @(
              '            ulFirstCluster = FF_getLong( pxBuffer->pucBuffer, 0U );',
              '            xError = FF_ReleaseBuffer( pxIOManager, pxBuffer );',
              '        }',
              '    }',
              '',
              '    /* ulFirstCluster is only read when ffconfigFAT_CHECK is enabled; this',
              '     * keeps it used and avoids -Wunused-but-set-variable. */',
              '    ( void ) ulFirstCluster;',
              '') -join "`n"
        Count  = 1
    }
)

foreach ( $patch in $patches )
{
    $path = Join-Path $ProjectRoot $patch.Rel

    if ( -not ( Test-Path -LiteralPath $path ) )
    {
        Write-Host ( "  [MISSING FILE] {0}" -f $patch.Name )
        $problems++
        continue
    }

    $original = [System.IO.File]::ReadAllText( $path )
    $hadCrLf  = $original.Contains( "`r`n" )
    $text     = $original.Replace( "`r`n", "`n" )

    if ( $text.Contains( $patch.Marker ) )
    {
        Write-Host ( "  [already ok  ] {0}" -f $patch.Name )
        $alreadyOk++
        continue
    }

    $found = 0
    $index = $text.IndexOf( $patch.Old )
    while ( $index -ge 0 )
    {
        $found++
        $index = $text.IndexOf( $patch.Old, $index + $patch.Old.Length )
    }

    if ( $found -eq 0 )
    {
        Write-Host ( "  [NOT FOUND   ] {0}  <-- check this one by hand" -f $patch.Name )
        $problems++
        continue
    }

    if ( ( $patch.Count -ne 0 ) -and ( $found -ne $patch.Count ) )
    {
        Write-Host ( "  [AMBIGUOUS {0} ] {1}  <-- check this one by hand" -f $found, $patch.Name )
        $problems++
        continue
    }

    $text = $text.Replace( $patch.Old, $patch.New )

    if ( $hadCrLf )
    {
        $text = $text.Replace( "`n", "`r`n" )
    }

    [System.IO.File]::WriteAllText( $path, $text, $encoding )
    Write-Host ( "  [patched  x{0} ] {1}" -f $found, $patch.Name )
    $applied++
}

Write-Host ( "fix_fat_warnings: applied={0} already-ok={1} problems={2}" -f $applied, $alreadyOk, $problems )

if ( $problems -gt 0 )
{
    Write-Host "fix_fat_warnings: some patches did not apply - the library sources may have changed."
}

exit 0
