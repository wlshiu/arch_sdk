/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file elf32.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/09/11
 * @license
 * @description
 */


#include <string.h>
#include "elf32.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
// #define ENABLE_ELF32_LOG    1
//=============================================================================
//                  Macro Definition
//=============================================================================
#define is_elf64(pElfHdr)       ((pElfHdr)->e_ident[EI_CLASS] == ELFCLASS64)

#if defined(ENABLE_ELF32_LOG) && (ENABLE_ELF32_LOG)
    #include <stdio.h>
    #define msg(str, ...)
    #define err(str, ...)
#else
    #define msg(str, ...)
    #define err(str, ...)
#endif
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
__attribute__ ((unused)) static bool
_is_elf32(Elf32_Ehdr *pEh)
{
    bool    rval = false;
    do {
        /**
         * ELF magic bytes are 0x7f,'E','L','F'
         * Using  octal escape sequence to represent 0x7f
         */
        if( strncmp((char*)pEh->e_ident, "\177ELF", 4) )
        {
            err("ELFMAGIC mismatch!\n"); // not a ELF file
            break;
        }

        rval = (is_elf64(pEh)) ? false : true;
    } while(0);
    return rval;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================

int
elf32_load_to_vma(
    uint8_t             *pElf_raw,
    cb_elf32_data_copy  cb_data_copy)
{
    int     rval = 0;
    do {
        Elf32_Ehdr      *pElf_hdr = (Elf32_Ehdr*)pElf_raw;
        Elf32_Shdr      *pSection_hdr = 0;
        int             cnt = 0;

        pSection_hdr = (Elf32_Shdr*)((uintptr_t)pElf_raw + pElf_hdr->e_shoff);

        msg("\n============= load to VMA:\n");
        msg("[Nr] Type  Addr  Off  Size");

        for(int i = 0; i < pElf_hdr->e_shnum; i++, pSection_hdr++)
        {
            if( pSection_hdr->sh_type != PT_LOAD ||
                pSection_hdr->sh_addr == 0x0 ||
                pSection_hdr->sh_size == 0 )
            {
                continue;
            }

            if( cb_data_copy )
            {
                rval = cb_data_copy((uintptr_t)pSection_hdr->sh_addr,
                                    (uintptr_t)pElf_raw + pSection_hdr->sh_offset,
                                    (uint32_t)pSection_hdr->sh_size);
                if( rval ) break;
            }

            msg("\n[%02d] %8x  %08x  %06x  %06x",
                cnt, pSection_hdr->sh_type, pSection_hdr->sh_addr,
                pSection_hdr->sh_offset, pSection_hdr->sh_size);

            cnt++;
        }

        msg("\nload cnt = %d\n", cnt);
    } while(0);
    return rval;
}
