//#undef LOADER_END
#define FLASH_SIZE getenv("FLASH_ERASE_SIZE")
#define LOADER_END   0x1FFF
#define LOADER_SIZE  0x3FF

#define CALIBRATION_AREA_START 0x4000
#define CALIBRATION_AREA_END 0x4FFF

#ifndef _bootloader
#build(reset=LOADER_END+1, interrupt=LOADER_END+9)
#org 0x0, 0xfff {}
#org 0x1000, 0x1ffe {}

#else
 ////when in the bootloader, keep out of the operational sections 
#org 0x2000, 0x7ffe {}

#endif

