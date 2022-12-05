/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

#define SD_DEFAULT_BLOCK_SIZE 512

extern int sdcard_init(void);
extern int sdcard_read(uint8_t *pdata, uint32_t address, uint32_t num);
extern int sdcard_write(uint8_t *pdata, uint32_t address, uint32_t num);
extern int sdcard_get_status(void);
extern uint32_t sdcard_get_sector_count(void);
extern uint16_t sdcard_get_sector_size(void);
extern uint32_t sdcard_get_block_size(void);

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    DSTATUS stat;
    int result;

    result = sdcard_get_status();

    if(result) {
        stat = RES_NOTRDY;
    } else {
        stat = RES_OK;
    }

    return stat;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    DSTATUS stat;
    int result;

    result = sdcard_init();

    if(result) {
        stat = STA_NOINIT;
    } else {
        stat = RES_OK;
    }

    return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,		/* Physical drive nmuber to identify the drive */
    BYTE *buff,		/* Data buffer to store read data */
    LBA_t sector,	/* Start sector in LBA */
    UINT count		/* Number of sectors to read */
)
{
    DRESULT res;
    int result;

    result = sdcard_read((uint8_t *)buff, sector, count);

    if(result) {
        res = RES_ERROR;
    } else {
        res = RES_OK;
    }

    return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
    BYTE pdrv,			/* Physical drive nmuber to identify the drive */
    const BYTE *buff,	/* Data to be written */
    LBA_t sector,		/* Start sector in LBA */
    UINT count			/* Number of sectors to write */
)
{
    DRESULT res;
    int result;

    result = sdcard_write((uint8_t *)buff, sector, count);

    if(result) {
        res = RES_ERROR;
    } else {
        res = RES_OK;
    }

    return res;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE pdrv,		/* Physical drive nmuber (0..) */
    BYTE cmd,		/* Control code */
    void *buff		/* Buffer to send/receive control data */
)
{
    DRESULT res = RES_ERROR;

    switch (cmd)
    {
    /* Make sure that no pending write process */
    case CTRL_SYNC :
        res = RES_OK;
        break;

    /* Get number of sectors on the disk (DWORD) */
    case GET_SECTOR_COUNT :
        *(DWORD*)buff = sdcard_get_sector_count();
        res = RES_OK;
        break;

    /* Get R/W sector size (WORD) */
    case GET_SECTOR_SIZE :
        *(WORD*)buff = sdcard_get_sector_size();
        res = RES_OK;
        break;

    /* Get erase block size in unit of sector (DWORD) */
    case GET_BLOCK_SIZE :
        *(DWORD*)buff = sdcard_get_block_size();
        res = RES_OK;
        break;

    default:
        res = RES_PARERR;
    }

    return res;
}

DWORD get_fattime (void)
{
  return 0;
}

