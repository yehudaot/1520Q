///////////////////////////////////////////////////////////////////////////
////                         loader.c                                  ////
////                                                                   ////
//// This driver will take an Intel 8-bit Hex file over RS232 channels ////
//// and modify the flash program memory with the new code.  A proxy   ////
//// function is required to sit between the real loading function     ////
//// and the main code because the #org preprocessor command could     ////
//// possibly change the order of functions within a defined block.    ////
////                                                                   ////
//// After each good line, the loader sends an ACKLOD character.  The  ////
//// driver uses XON/XOFF flow control.  Also, any buffer on the PC    ////
//// UART must be turned off, or to its lowest setting, otherwise it   ////
//// will miss data.                                                   ////
////                                                                   ////
////                                                                   ////
///////////////////////////////////////////////////////////////////////////

#ifndef LOADER_END
 #define LOADER_END      getenv("PROGRAM_MEMORY")-1

  #if (getenv("PROGRAM_MEMORY") % getenv("FLASH_ERASE_SIZE")) == 0
   #define LOADER_SIZE   0x7FF
  #else
   #define LOADER_SIZE   (getenv("PROGRAM_MEMORY") % (getenv("FLASH_ERASE_SIZE")) - 1)
#endif

#define LOADER_ADDR LOADER_END-LOADER_SIZE

#ifndef BOOTLOADER_AT_START
 #ORG LOADER_ADDR+4, LOADER_END auto=0 default
#endif

#define BUFFER_LEN_LOD 64

int  buffidx;
char buffer[BUFFER_LEN_LOD];

#define ACKLOD 'N'
#define XON    0x11
#define XOFF   0x13

unsigned int atoi_b16(char *s);

void boot_delay(int16 d)
  {
  int16 x;
  while (d--)
    {
    for (x = 0; x < 10; x++);
    }
  }

void boot_putc(int8 c)
  {
  TXREG1 = c;
  boot_delay(1000);
  }

void boot_puts(int8 *c)
  {
  while (*c)
    {
    TXREG1 = *c++;
    boot_delay(1000);
    }
  }

int8 boot_getc(void)
  {
  int8 c;
  while (RC1IF == 0)
    {
    }
  c = RCREG1;
  return c;
  }

void real_load_program(void)
  {
  int1  do_ACKLOD, done=FALSE;
  int8  checksum, line_type;
  int16 l_addr,h_addr=0;
  int32 addr;
#if getenv("FLASH_ERASE_SIZE") != getenv("FLASH_WRITE_SIZE")
  int32 next_addr;
#endif
  int8  dataidx, i, count;
  int8  data[32];

  boot_puts("Boot\r\n");
  while (!done)  // Loop until the entire program is downloaded
    {
    buffidx = 0;  // Read into the buffer until 0x0D ('\r') is received or the buffer is full
    do
      {
      buffer[buffidx] = boot_getc();
      }
    while ((buffer[buffidx++] != 0x0D) && (buffidx <= BUFFER_LEN_LOD));

    boot_putc(XOFF);  // Suspend sender

    do_ACKLOD = TRUE;

// Only process data blocks that start with ':'
    if (buffer[0] == ':')
      {
      count = atoi_b16 (&buffer[1]);  // Get the number of bytes from the buffer

// Get the lower 16 bits of address
      l_addr = make16(atoi_b16(&buffer[3]),atoi_b16(&buffer[5]));

      line_type = atoi_b16 (&buffer[7]);

      addr = make32(h_addr,l_addr);

      checksum = 0;  // Sum the bytes to find the check sum value
      for (i = 1; i < (buffidx - 3); i += 2)
        {
        checksum += atoi_b16 (&buffer[i]);
        }
      checksum = 0xFF - checksum + 1;

      if (checksum != atoi_b16 (&buffer[buffidx-3]))
        {
        do_ACKLOD = FALSE;
        }
      else
        {
// If the line type is 1, then data is done being sent
        if (line_type == 1)
          {
          done = TRUE;
          }
        else if (line_type == 4)
          {
          h_addr = make16(atoi_b16(&buffer[9]), atoi_b16(&buffer[11]));
          }
        else if (line_type == 0)
          {
          if ((addr < LOADER_ADDR || addr > LOADER_END) && addr < getenv("PROGRAM_MEMORY"))
            {
                  // Loops through all of the data and stores it in data
                  // The last 2 bytes are the check sum, hence buffidx-3
            for (i = 9,dataidx=0; i < buffidx-3; i += 2)
              {
              data[dataidx++]=atoi_b16(&buffer[i]);
              }

#if getenv("FLASH_ERASE_SIZE") > getenv("FLASH_WRITE_SIZE")
            if ((addr!=next_addr) &&
                (addr > (next_addr + (getenv("FLASH_ERASE_SIZE") - (next_addr % getenv("FLASH_ERASE_SIZE"))))) &&
                ((addr & (getenv("FLASH_ERASE_SIZE")-1)) != 0)
               )
              {
#if defined(BOOTLOADER_AT_START)
  #if ((getenv("PROGRAM_MEMORY") % getenv("FLASH_ERASE_SIZE")) != 0)
              if (addr > (getenv("PROGRAM_MEMORY") - (getenv("PROGRAM_MEMORY") % getenv("FLASH_ERASE_SIZE"))))
                {
                read_program_memory(getenv("PROGRAM_MEMORY"), buffer,
                                    getenv("FLASH_ERASE_SIZE") - (getenv("PROGRAM_MEMORY") % getenv("FLASH_ERASE_SIZE")));
                erase_program_eeprom(addr);
                write_program_memory(getenv("PROGRAM_MEMORY"), buffer,
                                     getenv("FLASH_ERASE_SIZE") - (getenv("PROGRAM_MEMORY") % getenv("FLASH_ERASE_SIZE")));
                }
              else
  #endif
#endif
                erase_program_eeprom(addr);
              }
            next_addr = addr + count;
                     #endif
                  #endif

#if defined(BOOTLOADER_AT_START)
  #if ((getenv("PROGRAM_MEMORY") % getenv("FLASH_ERASE_SIZE")) != 0)
            if (addr == (getenv("PROGRAM_MEMORY") - (getenv("PROGRAM_MEMORY") % getenv("FLASH_ERASE_SIZE"))))
              {
              read_program_memory(getenv("PROGRAM_MEMORY"), buffer,
                                  getenv("FLASH_ERASE_SIZE") - (getenv("PROGRAM_MEMORY") % getenv("FLASH_ERASE_SIZE")));
              write_program_memory(addr, data, count);
              write_program_memory(getenv("PROGRAM_MEMORY"), buffer,
                                   getenv("FLASH_ERASE_SIZE") - (getenv("PROGRAM_MEMORY") % getenv("FLASH_ERASE_SIZE")));
              }
            else
  #endif
#endif
            write_program_memory(addr, data, count);
            }
          }
        }
      }

    if (do_ACKLOD)
      {
      boot_putc (ACKLOD);
      }

    boot_putc(XON);
    }

  boot_putc (ACKLOD);
  boot_putc(XON);

  reset_cpu();
  }

unsigned int atoi_b16(char *s)
  {  // Convert two hex characters to a int8
  unsigned int result = 0;
  int i;

  for (i=0; i<2; i++,s++)
    {
    if (*s >= 'A')
      result = 16*result + (*s) - 'A' + 10;
    else
      result = 16*result + (*s) - '0';
    }

  return(result);
  }

#ifndef BOOTLOADER_AT_START
 #ORG default
 #ORG LOADER_ADDR, LOADER_ADDR+3
#endif
void load_program(void)
  {
  real_load_program();
  }
