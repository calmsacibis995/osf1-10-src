/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/*
 *  iic.h  -  Header containing all definitions pertainent to Cobra's IIC
 *            controller (the PCD8584 chip)
 *
 *************NOTE: This code is not "SMP safe".  It makes no assumptions
 *                  about current IPL and probably should not be used
 *                  at splextreme.  For example, a single byte write to the
 *                  EEPROM requires about 20 ms (that milliseconds!) of
 *                  recovery time so a memory correctable error log of
 *                  four bytes will take about 80 ms.  Use this code
 *                  without adult supervision is not recommended.
 */

#ifndef IIC_H
#define IIC_H

#include <sys/types.h>
#include <mach/boolean.h>
#include <io/common/devdriver.h>
#include <io/dec/mbox/mbox.h>

/*
 * IIC Register Address Definitions - these are the only 2 externally
 * available registers.  The control register must be loaded with the
 * address of the internal register you want to access.  Then it can
 * be read or written using the data register.
 */
#define IIC_DATA  0x00                  /* IIC Data Register Address       */
#define IIC_CNTL  0x04                  /* IIC Control Status Reg. Addr.   */


/*
 * Register Bit Definitions for the Control Register (S1)
 */
         /* for writes only */
#define IIC_PIN   0x80                  /* Pending Interrupt Not bit       */
#define IIC_ESO   0x40                  /* Enable Serial Output bit        */
#define IIC_ES1   0x20                  /* Enabling Selection bit 1        */
#define IIC_ES2   0x10                  /* Enabling Selection bit 2        */
#define IIC_ENI   0x08			/* ENable (external) Interrupt     */
#define	IIC_STA   0x04			/* send STArt condition bit        */
#define	IIC_STO   0x02			/* send STOp condition bit         */
#define IIC_ACK   0x01			/* ACKnowledge bit                 */
         /* shorthand access to bits in S1 that will be used often */
#define IIC_S0    0x40
#define IIC_S0P   0x00
#define IIC_S2    0x20
#define IIC_S3    0x10
#define IIC_INIT    (IIC_PIN | IIC_S0P)                            /* 0x80 */
#define IIC_NAK     (IIC_PIN | IIC_S0)                             /* 0xC0 */
#define IIC_AK      (IIC_PIN | IIC_S0 | IIC_ACK)                   /* 0xC1 */
#define IIC_STOP    (IIC_PIN | IIC_S0 | IIC_STO | IIC_ACK)         /* 0xC3 */
#define IIC_START   (IIC_PIN | IIC_S0 |IIC_STA | IIC_ACK)          /* 0xC5 */
#define IIC_RESTART (IIC_S0 |IIC_STA | IIC_ACK)                    /* 0x45 */

         /* for reads only  */
struct IIC_STATUS_BITS {
	unsigned int bb:1;		/* Bus Busy (not)		   */
	unsigned int lab:1;		/* Lost ArBitration 		   */
	unsigned int aas:1;		/* Addressed As Slave 	           */
	unsigned int lrb:1;		/* Last Received Bit or 	   */
					/* ADdress 0(general call)as Slave */
	unsigned int ber:1;		/* Bus ERror			   */
	unsigned int sts:1;		/* external STop Signal received   */
	unsigned int res:1;		/* REServed must be 0		   */
	unsigned int pin:1;		/* Pending Interrupt Not	   */
};

union IIC_STATUS_UNION {
	struct IIC_STATUS_BITS bits;
	unsigned char word;
};


/*
 * Clock Register definitions (S2)
 */
         /* SCL Frequencies */
#define IIC_SCL_90  0x00		/* 90 KHZ			   */
#define IIC_SCL_45  0x01		/* 45 KHZ			   */
#define IIC_SCL_11  0x02		/* 11 KHZ			   */
#define IIC_SCL_15  0x03		/* 1.5 KHZ			   */
         /* Internal Clock Frequencies */
#define IIC_CLOCK_3   0x00 		/* 3 MHZ			   */
#define IIC_CLOCK_443 0x10 		/* 4.43 MHZ			   */
#define IIC_CLOCK_6   0x14 		/* 6 MHZ			   */
#define IIC_CLOCK_8   0x18 		/* 8 MHZ			   */
#define IIC_CLOCK_12  0x1c 		/* 12 MHZ			   */
         /* clock condition specific to the Cobra implementation */
#define IIC_CLOCK (IIC_SCL_90 | IIC_CLOCK_12)


/*
 * Interrupt Vector definition (S3) - the IIC controller is loaded with
 * this value, however the IIC interrupt is never used. All code is
 * written in polled mode (just as the console code was done).  The IIC,
 * and especially the EEPROMs on the IIC, are so slow that performance
 * degradation in polled mode will not even be a factor.
 */
#define IIC_VECTOR  0x88


/*
 * macros for IIC register accesses through mailbox definitions
 */
#define RD_IIC_CSR(b,a)     RDCSR(LONG_32,(b),(a))
#define WR_IIC_CSR(b,a,d)   WRTCSR(LONG_32,(b),(a),(d))

#define RD_IIC_DATA(b)        RD_IIC_CSR((b),IIC_DATA)
#define WR_IIC_DATA(b,d)      WR_IIC_CSR((b),IIC_DATA,(d))
#define RD_IIC_CNTL(b)        RD_IIC_CSR((b),IIC_CNTL)
#define WR_IIC_CNTL(b,d)      WR_IIC_CSR((b),IIC_CNTL,(d))

/*
 * IIC bus device addresses - there are others, but these are the
 * only ones used by the kernel code except for the 2 registers on
 * the OCP which are at addresses 0x40 and 0x42 but they are defined
 * differently, below.
 */
#define MEM1ROM  0xA0       /* EEPROM on memory board 1      */
#define MEM2ROM  0xA2       /* EEPROM on memory board 2      */
#define MEM3ROM  0xA4       /* EEPROM on memory board 3      */
#define MEM4ROM  0xA6       /* EEPROM on memory board 4      */
#define CPU1ROM  0xA8       /* EEPROM on CPU board 1         */
#define CPU2ROM  0xAA       /* EEPROM on CPU board 2         */
#define IOROM    0xAC       /* EEPROM on I/O board           */
#define SPAREROM 0xAE       /* spare address                 */
#define IOCNTL   0xB6       /* IIC controller on I/O board   */
#define PSC      0xB8       /* Power supply controller       */


/*
 * Define direction flags. The LSB of all IIC addresses is reserved to be
 * the direction bit as defined here.
 */
#define IIC_WRITE_DIR 0x00
#define IIC_READ_DIR  0x01


/*
 * This structure is used for error logging and recording the bcache
 * correctable error information into the IIC EEPROM on the CPU modules.
 * Its definition is taken from Revision 1.0 of the IIC EEPROM document
 * by Erik Piip.  In this document, a bcache error SDD field is defined
 * to be 4 bytes long. The bit definition is:
 *
 *    3               2 2
 *    1               4 3                                 8 7               0
 *   +-----------------+-----------------------------------+-----------------+
 *   |      count      |              offset               |    syndrome     |
 *   +-----------------+-----------------------------------+-----------------+
 */
struct CPU_BAD_BIT_DESCRIP {
  unsigned int syndrome :  8;
  unsigned int offset   : 16;
  unsigned int count     : 8;
};
union CPU_BAD_BIT_DESC{
  struct CPU_BAD_BIT_DESCRIP bits;
  unsigned int word;
  unsigned char bytes[4];
};


/*
 * This structure is used for error logging and recording the error
 * information into the IIC EEPROM on the memory modules.
 * Its definition is taken from Revision 1.0 of the IIC EEPROM document
 * by Erik Piip.  In this document, a bcache error SDD field is defined
 * to be 4 bytes long. The contents of the fields are evident from their
 * names  for all err_typ except err_typ = 0 (00b).
 * The contents of the fields for this err_typ appear below:
 *   for err_typ = 0
 *   multi   : 1 if the entry was made by diagnostics
 *             0 otherwise
 *   bitmask : set to 0x1
 *   ram_num : when 72 = CMIC1 error or 
 *                  73 = CMIC2 error or
 *                  74 = Address logic error or
 *                  79 = Sync error1
 *                  80 = Sync error2
 *                offset = 0
 *             when 75 = CA parity error1
 *                offset : 0x00001 = cape_0
 *                         0x00004 = cape_2
 *                         0x01000 = mcape1
 *             when 76 = CA parity error2
 *                offset : 0x00002 = cape_1
 *                         0x00008 = cape_3
 *                         0x02000 = mcape2
 *             when 77 = Data parity error1
 *                offset : 0x00010 = dpe_0
 *                         0x00040 = dpe_2
 *                         0x00100 = dpe_4
 *                         0x00400 = dpe_6
 *                         0x04000 = mwdpe1
 *             when 78 = Data parity error2
 *                offset : 0x00020 = dpe_1
 *                         0x00080 = dpe_3
 *                         0x00200 = dpe_5
 *                         0x00800 = dpe_7
 *                         0x08000 = mwdpe2
 *                  
 * Note: multiple bits may be set in the offset field
 *
 * The bit definition (field placement) is:
 *
 *    3 3 2 2       2 2                1 1
 *    1 0 9 8       5 4                8 7                                  0
 *   +---+-+---------+------------------+------------------------------------+
 *   |   | | bitmask |     ram_num      |               offset               |
 *   +---+-+---------+------------------+------------------------------------+
 *     ^  ^
 *     |  |
 *     |  +-------- multi
 *     +------------err_typ
 */
struct MEM_BAD_BIT_DESCRIP {
  unsigned int offset  :18;
  unsigned int ram_num : 7;
  unsigned int bitmask : 4;
  unsigned int multi   : 1;
  unsigned int err_typ : 2;
};
union MEM_BAD_BIT_DESC{
  struct MEM_BAD_BIT_DESCRIP bits;
  unsigned int word;
  unsigned char bytes[4];
};

/*
 * The following structures declare the IIC EEPROM address offset fields
 * into the data structures on all of the COBRA
 * modules.  The members of these structs are initialized in data/cbus_data.c
 * Only the members of the cpu_sdd, io_sdd, and mem_sdd struct members are
 * actually used by the kernel code, but they are all defined for
 * informational purposes and completeness.  The EEPROM address and field
 * definitions implemented here are taken from Revision 1.1 of the IIC
 * EEPROM spec written by Erik Piip.
 *
 * These structures declare the fixed IIC EEPROM address offsets that 
 * are common to all the EEPROMs on all the modules
 */
struct HEADER {
  unsigned char checksum;        /* 0x00 */
  unsigned char class_type;      /* 0x04 */
  unsigned char subclass_ver;    /* 0x06 */
  unsigned char len;             /* 0x08 */
  unsigned char fru_desc_ptr;    /* 0x0C */
  unsigned char symp_log_ptr;    /* 0x10 */
  unsigned char diag_log_ptr;    /* 0x14 */
  unsigned char zero;            /* 0x18*/
};
struct FRU {
  unsigned char checksum;        /* 0x1C */
  unsigned char class_type;      /* 0x20 */
  unsigned char subclass_ver;    /* 0x22 */
  unsigned char len;             /* 0x24 */
  unsigned char serial_num;      /* 0x28 */
  unsigned char partnum1[2];     /* 0x2C, 0x2D */
  unsigned char partnum2[7];     /* 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34 */
  unsigned char sw_rev;          /* 0x35 */
  unsigned char hw_rev[2];       /* 0x36, 0x37 */
};
struct CONFIG_DATA {
  unsigned char speed;           /* 0xFC */
  unsigned char num_roms;        /* 0xFD */
  unsigned char rom_flags;       /* 0xFE */
  unsigned char checksum;        /* 0xFF */
};

/*
 * This structure defines the IIC EEPROM address offsets on the CPU modules.
 * Their format is diagrammed below:
 *
 *   +---------------------+
 *   |                     | 0x00
 *   |       Header        |
 *   |                     |
 *   +---------------------+
 *   |                     | 0x1C
 *   |         FRU         |
 *   |                     |
 *   +---------------------+
 *   |                     | 0x38
 *   |         SDD         |
 *   |                     |
 *   +---------------------+
 *   |                     | 0xA0
 *   |         TDD         |
 *   |                     |
 *   +---------------------+
 *   |        Config       | 0xFC
 *   +---------------------+
 *
 * Of particular interest is the SDD region (since it is the only one
 * used by the kernel.  It is detailed further below:
 *
 *       byte 3     byte 2      byte 1      byte 0   
 *   +-----------+-----------+-----------+-----------+
 *   |                    Checksum                   | 0x38
 *   +-----------+-----------+-----------+-----------+
 *   |  Subclass version     |      Class type       | 0x3C
 *   +-----------+-----------+-----------+-----------+
 *   |                     Length                    | 0x40
 *   +-----------+-----------+-----------+-----------+
 *   |Symlog flgs|Symlog cnt |  Symptom log control  | 0x44
 *   +-----------+-----------+-----------+-----------+
 *   |               Symptom log length              | 0x48
 *   +-----------+-----------+-----------+-----------+
 *   |                                               | 0x4C
 *   |                                               | 0x50
 *   |            28 individual byte-wide            | 0x54
 *   |               counters used for               | 0x58
 *   |              uncorrectable errors             | 0x5c
 *   |                                               | 0x60
 *   |                                               | 0x64
 *   +-----------+-----------+-----------+-----------+
 *   |                                               | 0x68
 *   |                                               | 0x6C
 *   |                                               | 0x70
 *   |                                               | 0x74
 *   |            14 individaul 4-byte wide          | 0x78
 *   |             storage areas used for            | 0x7C
 *   |            correctable bcache errors          | 0x80
 *   |                                               | 0x84
 *   |                                               | 0x88
 *   |                                               | 0x8C
 *   |                                               | 0x90
 *   |                                               | 0x94
 *   |                                               | 0x98
 *   |                                               | 0x9C
 *   +-----------+-----------+-----------+-----------+
 */

#define CPU_CORRERR_FIELDS 14
#define CPU_SYMLL_INIT 32

struct CPUIICEEPROM {
  struct HEADER header;
  struct FRU fru;
  struct CPU_SDD {
    unsigned char checksum;      /* 0x38 */
    unsigned char class_type;    /* 0x3C */
    unsigned char subclass_ver;  /* 0x3E */
    unsigned char len;           /* 0x40 */
    unsigned char sym_log_ctrl;  /* 0x44 */
    unsigned char sym_log_count; /* 0x46 */
    unsigned char sym_log_flags; /* 0x47 */
    unsigned char sym_log_len;   /* 0x48 */
    unsigned char uncorr_err_counter[28]; /*0x4C, 0x4D, 0x4E, 0x4F, 0x50,
					    0x51, 0x52, 0x53, 0x54, 0x55,
					    0x56, 0x57, 0x58, 0x59, 0x5A,
					    0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
					    0x60, 0x61, 0x62, 0x63, 0x64,
					    0x65, 0x66, 0x67 */
    unsigned char bad_bit_desc[CPU_CORRERR_FIELDS]; /*0x68, 0x6C, 0x70, 0x74,
						      0x78, 0x7C, 0x80, 0x84,
						      0x88, 0x8C, 0x90, 0x94,
						      0x98, 0x9C*/
  } cpu_sdd;
  struct CPU_TDD {
    unsigned char checksum;            /* 0xA0 */
    unsigned char class_type;          /* 0xA4 */
    unsigned char subclass_ver;        /* 0xA6 */
    unsigned char len;                 /* 0xA8 */
    unsigned char diag_log_ctrl;       /* 0xAC */
    unsigned char diag_log_count;      /* 0xAE */
    unsigned char diag_log_flags;      /* 0xAF */
    unsigned char diag_log_len;        /* 0xB0 */
    unsigned char blind_packet[6];     /* 0xB4, 0xB8, 0xBC, 0xC0, 0xC4, 0xC8 */
    unsigned char diag_log_entries[12];/* 0xCC, 0xD0, 0xD4, 0xD8, 0xDC, 0xE0,
					  0xE4, 0xE8, 0xEC, 0xF0, 0xF4, 0xF8*/
  } cpu_tdd;
  struct CONFIG_DATA config_data;
};
extern struct CPUIICEEPROM cpu_iic_eeprom;


/*
 * This structure defines the IIC EEPROM address offsets on the I/O module
 * Their format is diagrammed below:
 *
 *   +---------------------+
 *   |                     | 0x00
 *   |       Header        |
 *   |                     |
 *   +---------------------+
 *   |                     | 0x1C
 *   |         FRU         |
 *   |                     |
 *   +---------------------+
 *   |                     | 0x38
 *   |         SDD         |
 *   |                     |
 *   +---------------------+
 *   |                     | 0x90
 *   |         TDD         |
 *   |                     |
 *   +---------------------+
 *   |        Config       | 0xFC
 *   +---------------------+
 *
 * Of particular interest is the SDD region (since it is the only one
 * used by the kernel.  It is detailed further below:
 *
 *       byte 3     byte 2      byte 1      byte 0   
 *   +-----------+-----------+-----------+-----------+
 *   |                    Checksum                   | 0x38
 *   +-----------+-----------+-----------+-----------+
 *   |  Subclass version     |      Class type       | 0x3C
 *   +-----------+-----------+-----------+-----------+
 *   |                     Length                    | 0x40
 *   +-----------+-----------+-----------+-----------+
 *   |Symlog flgs|Symlog cnt |  Symptom log control  | 0x44
 *   +-----------+-----------+-----------+-----------+
 *   |               Symptom log length              | 0x48
 *   +-----------+-----------+-----------+-----------+
 *   |                                               | 0x4C
 *   |                                               | 0x50
 *   |                                               | 0x54
 *   |                                               | 0x58
 *   |                                               | 0x5C
 *   |                                               | 0x60
 *   |            68 individual byte-wide            | 0x64
 *   |               counters used for               | 0x68
 *   |              uncorrectable errors             | 0x6c
 *   |                                               | 0x70
 *   |                                               | 0x74
 *   |                                               | 0x78
 *   |                                               | 0x7C
 *   |                                               | 0x80
 *   |                                               | 0x84
 *   |                                               | 0x88
 *   |                                               | 0x8C
 *   +-----------+-----------+-----------+-----------+
 */
struct IOIICEEPROM {
  struct HEADER header;
  struct FRU fru;
  struct IO_SDD {
    unsigned char checksum;               /* 0x38 */
    unsigned char class_type;             /* 0x3C */
    unsigned char subclass_ver;           /* 0x3E */
    unsigned char len;                    /* 0x40 */
    unsigned char sym_log_ctrl;           /* 0x44 */
    unsigned char sym_log_count;          /* 0x46 */
    unsigned char sym_log_flags;          /* 0x47 */
    unsigned char sym_log_len;            /* 0x48 */
    unsigned char uncorr_err_counter[68]; /* 0x4C, 0x4D, 0x4E, 0x4F, 0x50
					     0x51, 0x52, 0x53, 0x54, 0x55,
					     0x56, 0x57, 0x58, 0x59, 0x5A,
					     0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
					     0x60, 0x61, 0x62, 0x63, 0x64,
					     0x65, 0x66, 0x67, 0x68, 0x69,
					     0x6A, 0x6B, 0x6C, 0x6D, 0x6E,
					     0x6F, 0x70, 0x71, 0x72, 0x73,
					     0x74, 0x75, 0x76, 0x77, 0x78,
					     0x79, 0x7A, 0x7B, 0x7C, 0x7D,
					     0x7E, 0x7F, 0x80, 0x81, 0x82,
					     0x83, 0x84, 0x85, 0x86, 0x87,
					     0x88, 0x89, 0x8A, 0x8B, 0x8C,
					     0x8D, 0x8E, 0x8F */
  } io_sdd;
  struct IO_TDD {
    unsigned char checksum;            /* 0x90 */
    unsigned char class_type;          /* 0x94 */
    unsigned char subclass_ver;        /* 0x96 */
    unsigned char len;                 /* 0x98 */
    unsigned char diag_log_ctrl;       /* 0x9C */
    unsigned char diag_log_count;      /* 0x9E */
    unsigned char diag_log_flags;      /* 0x9F */
    unsigned char diag_log_len;        /* 0xA0 */
    unsigned char blind_packet[6];     /* 0xA4, 0xA8, 0xAC, 0xB0, 0xB4, 0xB8 */
    unsigned char diag_log_entries[16];/* 0xBC, 0xC0, 0xC4, 0xC8, 
					  0xCC, 0xD0, 0xD4, 0xD8,
					  0xDC, 0xE0, 0xE4, 0xE8, 
					  0xEC, 0xF0, 0xF4, 0xF8 */
  } io_tdd;
  struct CONFIG_DATA config_data;
};
extern struct IOIICEEPROM io_iic_eeprom;


/*
 * This structure defines the IIC EEPROM address offsets on the memory modules
 * Their format is diagrammed below:
 *
 *   +---------------------+
 *   |                     | 0x00
 *   |       Header        |
 *   |                     |
 *   +---------------------+
 *   |                     | 0x1C
 *   |         FRU         |
 *   |                     |
 *   +---------------------+
 *   |                     | 0x38
 *   |         SDD         |
 *   |                     |
 *   +---------------------+
 *   |                     | 0xD0
 *   |         TDD         |
 *   |                     |
 *   +---------------------+
 *   |        Config       | 0xFC
 *   +---------------------+
 *
 * Of particular interest is the SDD region (since it is the only one
 * used by the kernel.  It is detailed further below:
 *
 *       byte 3     byte 2      byte 1      byte 0   
 *   +-----------+-----------+-----------+-----------+
 *   |                    Checksum                   | 0x38
 *   +-----------+-----------+-----------+-----------+
 *   |  Subclass version     |      Class type       | 0x3C
 *   +-----------+-----------+-----------+-----------+
 *   |                     Length                    | 0x40
 *   +-----------+-----------+-----------+-----------+
 *   |Symlog flgs|Symlog cnt |  Symptom log control  | 0x44
 *   +-----------+-----------+-----------+-----------+
 *   |               Symptom log length              | 0x48
 *   +-----------+-----------+-----------+-----------+
 *   |                                               | 0x4C
 *   |                                               | 0x50
 *   |                                               | 0x54
 *   |                                               | 0x58
 *   |                                               | 0x5C
 *   |                                               | 0x60
 *   |                                               | 0x64
 *   |                                               | 0x68
 *   |                                               | 0x6C
 *   |                                               | 0x70
 *   |                                               | 0x74
 *   |                                               | 0x78
 *   |                                               | 0x7C
 *   |                                               | 0x80
 *   |                                               | 0x84
 *   |                                               | 0x88
 *   |            33 individaul 4-byte wide          | 0x8C
 *   |             storage areas used for            | 0x90
 *   |            correctable bcache errors          | 0x94
 *   |                                               | 0x98
 *   |                                               | 0x9C
 *   |                                               | 0xA0
 *   |                                               | 0xA4
 *   |                                               | 0xA8
 *   |                                               | 0xAC
 *   |                                               | 0xB0
 *   |                                               | 0xB4
 *   |                                               | 0xB8
 *   |                                               | 0xBC
 *   |                                               | 0xC0
 *   |                                               | 0xC4
 *   |                                               | 0xC8
 *   |                                               | 0xCC
 *   +-----------+-----------+-----------+-----------+
 */

#define MEM_CORRERR_FIELDS 33
#define MEM_SYMLL_INIT 4

struct MEMIICEEPROM {
  struct HEADER header;
  struct FRU fru;
  struct MEM_SDD {
    unsigned char checksum;         /* 0x38 */
    unsigned char class_type;       /* 0x3C */
    unsigned char subclass_ver;     /* 0x3E */
    unsigned char len;              /* 0x40 */
    unsigned char sym_log_ctrl;     /* 0x44 */
    unsigned char sym_log_count;    /* 0x46 */
    unsigned char sym_log_flags;    /* 0x47 */
    unsigned char sym_log_len;      /* 0x48 */
    unsigned char bad_bit_desc[MEM_CORRERR_FIELDS]; /* 0x4C, 0x50, 0x54, 0x58,
						       0x5C, 0x60, 0x64, 0x68,
						       0x6C, 0x70, 0x74, 0x78,
						       0x7C, 0x80, 0x84, 0x88,
						       0x8C, 0x90, 0x94, 0x98,
						       0x9C, 0xA0, 0xA4, 0xA8,
						       0xAC, 0xB0, 0xB4, 0xB8,
						       0xBC, 0xC0, 0xC4, 0xC8,
						       0xCC */
  } mem_sdd;
  struct MEM_TDD {
    unsigned char checksum;            /* 0xD0 */
    unsigned char class_type;          /* 0xD4 */
    unsigned char subclass_ver;        /* 0xD6 */
    unsigned char len;                 /* 0xD8 */
    unsigned char diag_log_ctrl;       /* 0xDC */
    unsigned char diag_log_count;      /* 0xDE */
    unsigned char diag_log_flags;      /* 0xDF */
    unsigned char diag_log_len;        /* 0xE0 */
    unsigned char blind_packet[6];     /* 0xE4, 0xE8, 0xEC, 0xF0, 0xF4, 0xF8 */
  } mem_tdd;
  struct MEM_CONFIG_DATA {
    unsigned char mod_size;        /* 0xFC */
    unsigned char reserved;        /* 0xFD */
    unsigned char checksum;        /* 0xFF */
  } mem_config_data;
};
extern struct MEMIICEEPROM mem_iic_eeprom;


/*
 * These define the error descriptors that the IIC understands
 * and is capable of logging.  Also for all except EV_BCACHE_ERR,
 * C3_BCACHE_ERR, and the memory error descriptors, the value they
 * are defined to be corresponds to the address in the appropriate
 * EEPROM that is reserved for the counter for this error type.
 * These error type definitions and their corresponding addresses
 * are taken from Revision 1.1 of the IIC EEPROM spec written by Eric Piip.
 */
      /* CPU Error Descriptors */
#define  C3_CA_NOACK          cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[0]
#define  C3_WD_NOACK          cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[1]
#define  C3_RD_PAR_E          cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[2]
#define  C3_RD_PAR_O          cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[3]

#define  EV_C_UNCORR_0        cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[4]
#define  EV_C_UNCORR_1        cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[5]
#define  EV_C_UNCORR_2        cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[6]
#define  EV_C_UNCORR_3        cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[7]

#define  EV_TC_PAR            cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[8]
#define  EV_T_PAR             cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[9]
#define  C3_TC_PAR_E          cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[10]
#define  C3_TC_PAR_O          cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[11]

#define  C3_T_PAR_E           cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[12]
#define  C3_T_PAR_O           cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[13]
#define  C3_EV_E              cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[14]
#define  C3_EV_O              cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[15]

#define  C3_CA_PAR_E          cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[16]
#define  C3_CA_PAR_O          cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[17]
#define  C3_C_UNCORR_0        cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[18]
#define  C3_C_UNCORR_1        cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[19]

#define  C3_C_UNCORR_2        cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[20]
#define  C3_C_UNCORR_3        cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[21]
#define  C3_WD_PAR_E          cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[22]
#define  C3_WD_PAR_O          cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[23]

#define  C3_DT_PAR_E          cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[24]
#define  C3_DT_PAR_O          cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[25]
#define  CPU_ERRTYPE26        cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[26]
#define  CPU_ERRTYPE27        cpu_iic_eeprom.cpu_sdd.uncorr_err_counter[27]

#define  EV_BCACHE_ERR        0xFC   /* the values for these error types */
#define  C3_BCACHE_ERR        0xFD   /* are meaningless, but unique from */
                                     /* the others above                 */


      /* I/O Error Descriptors */
#define  IO_CA_NOACK_E        io_iic_eeprom.io_sdd.uncorr_err_counter[0]
#define  IO_CA_NOACK_O        io_iic_eeprom.io_sdd.uncorr_err_counter[1]
#define  IO_WD_NOACK_E        io_iic_eeprom.io_sdd.uncorr_err_counter[2]
#define  IO_WD_NOACK_O        io_iic_eeprom.io_sdd.uncorr_err_counter[3]

#define  IO_WD_PAR_E          io_iic_eeprom.io_sdd.uncorr_err_counter[4]
#define  IO_WD_PAR_O          io_iic_eeprom.io_sdd.uncorr_err_counter[5]
#define  IO_RD_PAR_E          io_iic_eeprom.io_sdd.uncorr_err_counter[6]
#define  IO_RD_PAR_O          io_iic_eeprom.io_sdd.uncorr_err_counter[7]

#define  IO_CA_PAR_E          io_iic_eeprom.io_sdd.uncorr_err_counter[8]
#define  IO_CA_PAR_O          io_iic_eeprom.io_sdd.uncorr_err_counter[9]
#define  IO_LB_DMA_PAR_E      io_iic_eeprom.io_sdd.uncorr_err_counter[10]
#define  IO_LB_DMA_PAR_O      io_iic_eeprom.io_sdd.uncorr_err_counter[11]

#define  IO_FB_DMA_PAR_E      io_iic_eeprom.io_sdd.uncorr_err_counter[12]
#define  IO_FB_DMA_PAR_O      io_iic_eeprom.io_sdd.uncorr_err_counter[13]
#define  IO_SCSTALL_E         io_iic_eeprom.io_sdd.uncorr_err_counter[14]
#define  IO_SCSTALL_O         io_iic_eeprom.io_sdd.uncorr_err_counter[15]

#define  IO_FB_MB_PAR         io_iic_eeprom.io_sdd.uncorr_err_counter[16]
#define  IO_BUSSSYNC          io_iic_eeprom.io_sdd.uncorr_err_counter[17]
#define  IO_TGEC0_A           io_iic_eeprom.io_sdd.uncorr_err_counter[18]
#define  IO_TGEC0_B           io_iic_eeprom.io_sdd.uncorr_err_counter[19]

#define  IO_TGEC1_A           io_iic_eeprom.io_sdd.uncorr_err_counter[20]
#define  IO_TGEC1_B           io_iic_eeprom.io_sdd.uncorr_err_counter[21]
#define  IO_SCRIPT_RAM_A      io_iic_eeprom.io_sdd.uncorr_err_counter[22]
#define  IO_SCRIPT_RAM_B      io_iic_eeprom.io_sdd.uncorr_err_counter[23]

#define  IO_NCR0_A            io_iic_eeprom.io_sdd.uncorr_err_counter[24]
#define  IO_NCR0_B            io_iic_eeprom.io_sdd.uncorr_err_counter[25]
#define  IO_NCR1_A            io_iic_eeprom.io_sdd.uncorr_err_counter[26]
#define  IO_NCR1_B            io_iic_eeprom.io_sdd.uncorr_err_counter[27]

#define  IO_NCR2_A            io_iic_eeprom.io_sdd.uncorr_err_counter[28]
#define  IO_NCR2_B            io_iic_eeprom.io_sdd.uncorr_err_counter[29]
#define  IO_NCR3_A            io_iic_eeprom.io_sdd.uncorr_err_counter[30]
#define  IO_NCR3_B            io_iic_eeprom.io_sdd.uncorr_err_counter[31]

#define  IO_NCR4_A            io_iic_eeprom.io_sdd.uncorr_err_counter[32]
#define  IO_NCR4_B            io_iic_eeprom.io_sdd.uncorr_err_counter[33]

  /* reserved IO EEPROM counter fields that are currently unused */
#define  IO_ERRTYPE34         io_iic_eeprom.io_sdd.uncorr_err_counter[34]
#define  IO_ERRTYPE35         io_iic_eeprom.io_sdd.uncorr_err_counter[35]
#define  IO_ERRTYPE36         io_iic_eeprom.io_sdd.uncorr_err_counter[36]
#define  IO_ERRTYPE37         io_iic_eeprom.io_sdd.uncorr_err_counter[37]
#define  IO_ERRTYPE38         io_iic_eeprom.io_sdd.uncorr_err_counter[38]
#define  IO_ERRTYPE39         io_iic_eeprom.io_sdd.uncorr_err_counter[39]
#define  IO_ERRTYPE40         io_iic_eeprom.io_sdd.uncorr_err_counter[40]
#define  IO_ERRTYPE41         io_iic_eeprom.io_sdd.uncorr_err_counter[41]
#define  IO_ERRTYPE42         io_iic_eeprom.io_sdd.uncorr_err_counter[42]
#define  IO_ERRTYPE43         io_iic_eeprom.io_sdd.uncorr_err_counter[43]
#define  IO_ERRTYPE44         io_iic_eeprom.io_sdd.uncorr_err_counter[44]
#define  IO_ERRTYPE45         io_iic_eeprom.io_sdd.uncorr_err_counter[45]
#define  IO_ERRTYPE46         io_iic_eeprom.io_sdd.uncorr_err_counter[46]
#define  IO_ERRTYPE47         io_iic_eeprom.io_sdd.uncorr_err_counter[47]
#define  IO_ERRTYPE48         io_iic_eeprom.io_sdd.uncorr_err_counter[48]
#define  IO_ERRTYPE49         io_iic_eeprom.io_sdd.uncorr_err_counter[49]
#define  IO_ERRTYPE50         io_iic_eeprom.io_sdd.uncorr_err_counter[50]
#define  IO_ERRTYPE51         io_iic_eeprom.io_sdd.uncorr_err_counter[51]
#define  IO_ERRTYPE52         io_iic_eeprom.io_sdd.uncorr_err_counter[52]
#define  IO_ERRTYPE53         io_iic_eeprom.io_sdd.uncorr_err_counter[53]
#define  IO_ERRTYPE54         io_iic_eeprom.io_sdd.uncorr_err_counter[54]
#define  IO_ERRTYPE55         io_iic_eeprom.io_sdd.uncorr_err_counter[55]
#define  IO_ERRTYPE56         io_iic_eeprom.io_sdd.uncorr_err_counter[56]
#define  IO_ERRTYPE57         io_iic_eeprom.io_sdd.uncorr_err_counter[57]
#define  IO_ERRTYPE58         io_iic_eeprom.io_sdd.uncorr_err_counter[58]
#define  IO_ERRTYPE59         io_iic_eeprom.io_sdd.uncorr_err_counter[59]
#define  IO_ERRTYPE60         io_iic_eeprom.io_sdd.uncorr_err_counter[60]
#define  IO_ERRTYPE61         io_iic_eeprom.io_sdd.uncorr_err_counter[61]
#define  IO_ERRTYPE62         io_iic_eeprom.io_sdd.uncorr_err_counter[62]
#define  IO_ERRTYPE63         io_iic_eeprom.io_sdd.uncorr_err_counter[63]
#define  IO_ERRTYPE64         io_iic_eeprom.io_sdd.uncorr_err_counter[64]
#define  IO_ERRTYPE65         io_iic_eeprom.io_sdd.uncorr_err_counter[65]
#define  IO_ERRTYPE66         io_iic_eeprom.io_sdd.uncorr_err_counter[66]
#define  IO_ERRTYPE67         io_iic_eeprom.io_sdd.uncorr_err_counter[67]



      /* Memory Error Descriptors */
#define  MEM_CRD              0x01   /* the value for these 3 correspond  */
#define  MEM_UNCORR_BIT       0x02   /* to the err_typ field value        */
#define  MEM_HARD_FAIL        0x03   /* in the entry for this error type  */

#define  CMIC1_ERR            72     /* these other values are for errors */
#define  CMIC2_ERR            73     /* which all have their err_typ      */
#define  MEM_ADDR_ERR         74     /* field equal to 0 and the value    */
#define  CA_PAR_ERR_E         75     /* here corresponds to the ram_num   */
#define  CA_PAR_ERR_O         76     /* field value in the entry for the  */
#define  DATA_PAR_ERR_E       77     /* given error type                  */
#define  DATA_PAR_ERR_O       78
#define  SYNC_ERROR_E         79
#define  SYNC_ERROR_O         80


/*
 * Definitions of the correctable memory error syndromes. These
 * are taken from Revision 0.7-6 of the COBRA memory module
 * written by Dave Tatosian.
 */
#define BAD_BIT_0             0x017
#define BAD_BIT_1             0x029
#define BAD_BIT_2             0x01D
#define BAD_BIT_3             0x026
#define BAD_BIT_4             0x01F
#define BAD_BIT_5             0x025
#define BAD_BIT_6             0x031
#define BAD_BIT_7             0x0A2
#define BAD_BIT_8             0x111
#define BAD_BIT_9             0x222
#define BAD_BIT_10            0x411
#define BAD_BIT_11            0x822
#define BAD_BIT_12            0x05D
#define BAD_BIT_13            0x0A6
#define BAD_BIT_14            0x05F
#define BAD_BIT_15            0x0A5
#define BAD_BIT_16            0x054
#define BAD_BIT_17            0x0A8
#define BAD_BIT_18            0x114
#define BAD_BIT_19            0x228
#define BAD_BIT_20            0x117
#define BAD_BIT_21            0x229
#define BAD_BIT_22            0x11D
#define BAD_BIT_23            0x226
#define BAD_BIT_24            0x11F
#define BAD_BIT_25            0x225
#define BAD_BIT_26            0x057
#define BAD_BIT_27            0x0A9
#define BAD_BIT_28            0x414
#define BAD_BIT_29            0x828
#define BAD_BIT_30            0x41F
#define BAD_BIT_31            0x825
#define BAD_BIT_32            0x8B0
#define BAD_BIT_33            0xCD0
#define BAD_BIT_34            0x8B1
#define BAD_BIT_35            0xCD2
#define BAD_BIT_36            0x8B4
#define BAD_BIT_37            0xCD8
#define BAD_BIT_38            0x8B7
#define BAD_BIT_39            0xCD9
#define BAD_BIT_40            0x8BD
#define BAD_BIT_41            0xCD6
#define BAD_BIT_42            0x8BF
#define BAD_BIT_43            0xCD5
#define BAD_BIT_44            0x940
#define BAD_BIT_45            0xE80
#define BAD_BIT_46            0x941
#define BAD_BIT_47            0xE82
#define BAD_BIT_48            0x944
#define BAD_BIT_49            0xE88
#define BAD_BIT_50            0x947
#define BAD_BIT_51            0xE89
#define BAD_BIT_52            0x94D
#define BAD_BIT_53            0xE86
#define BAD_BIT_54            0x94F
#define BAD_BIT_55            0xE85
#define BAD_BIT_56            0x670
#define BAD_BIT_57            0xB90
#define BAD_BIT_58            0x671
#define BAD_BIT_59            0xB92
#define BAD_BIT_60            0x674
#define BAD_BIT_61            0xB98
#define BAD_BIT_62            0x67F
#define BAD_BIT_63            0xB95
#define BAD_BIT_64            0xE30
#define BAD_BIT_65            0x710
#define BAD_BIT_66            0xE31
#define BAD_BIT_67            0x712
#define BAD_BIT_68            0xE34
#define BAD_BIT_69            0x718
#define BAD_BIT_70            0xE37
#define BAD_BIT_71            0x719
#define BAD_BIT_72            0xE3D
#define BAD_BIT_73            0x716
#define BAD_BIT_74            0xE3F
#define BAD_BIT_75            0x715
#define BAD_BIT_76            0xF10
#define BAD_BIT_77            0x520
#define BAD_BIT_78            0xF11
#define BAD_BIT_79            0x522
#define BAD_BIT_80            0xF14
#define BAD_BIT_81            0x528
#define BAD_BIT_82            0xF17
#define BAD_BIT_83            0x529
#define BAD_BIT_84            0xF1D
#define BAD_BIT_85            0x526
#define BAD_BIT_86            0xF1F
#define BAD_BIT_87            0x525
#define BAD_BIT_88            0x4D0
#define BAD_BIT_89            0x860
#define BAD_BIT_90            0x4D1
#define BAD_BIT_91            0x862
#define BAD_BIT_92            0x4D4
#define BAD_BIT_93            0x868
#define BAD_BIT_94            0x4DF
#define BAD_BIT_95            0x865
#define BAD_BIT_96            0x6C0
#define BAD_BIT_97            0xB40
#define BAD_BIT_98            0x6C1
#define BAD_BIT_99            0xB42
#define BAD_BIT_100           0x6C4
#define BAD_BIT_101           0xB48
#define BAD_BIT_102           0x6C7
#define BAD_BIT_103           0xB49
#define BAD_BIT_104           0x6CD
#define BAD_BIT_105           0xB46
#define BAD_BIT_106           0x6CF
#define BAD_BIT_107           0xB45
#define BAD_BIT_108           0x560
#define BAD_BIT_109           0xAB0
#define BAD_BIT_110           0x561
#define BAD_BIT_111           0xAB2
#define BAD_BIT_112           0x564
#define BAD_BIT_113           0xAB8
#define BAD_BIT_114           0x567
#define BAD_BIT_115           0xAB9
#define BAD_BIT_116           0x56D
#define BAD_BIT_117           0xAB6
#define BAD_BIT_118           0x56F
#define BAD_BIT_119           0xAB5
#define BAD_BIT_120           0x5B0
#define BAD_BIT_121           0xAD0
#define BAD_BIT_122           0x5B1
#define BAD_BIT_123           0xAD2
#define BAD_BIT_124           0x5B4
#define BAD_BIT_125           0xAD8
#define BAD_BIT_126           0x5BF
#define BAD_BIT_127           0xAD5
#define BAD_BIT_C0            0x011
#define BAD_BIT_C1            0x022
#define BAD_BIT_C2            0x014
#define BAD_BIT_C3            0x028
#define BAD_BIT_C4            0x010
#define BAD_BIT_C5            0x020
#define BAD_BIT_C6            0x050
#define BAD_BIT_C7            0x0A0
#define BAD_BIT_C8            0x110
#define BAD_BIT_C9            0x220
#define BAD_BIT_C10           0x410
#define BAD_BIT_C11           0x820
#define BAD_BITS_0_1          0x03E
#define BAD_BITS_2_3          0x03B
#define BAD_BITS_4_5          0x03A
#define BAD_BITS_6_7          0x0F3
#define BAD_BITS_8_9          0x333
#define BAD_BITS_10_11        0xC33
#define BAD_BITS_12_13        0x0FB
#define BAD_BITS_14_15        0x0FA
#define BAD_BITS_16_17        0x0FC
#define BAD_BITS_18_19        0x33C
#define BAD_BITS_20_21        0x33E
#define BAD_BITS_22_23        0x33B
#define BAD_BITS_24_25        0x33A
#define BAD_BITS_26_27        0x0FE
#define BAD_BITS_28_29        0xC3C
#define BAD_BITS_30_31        0xC3A
#define BAD_BITS_32_33        0x460
#define BAD_BITS_34_35        0x463
#define BAD_BITS_36_37        0x46C
#define BAD_BITS_38_39        0x46E
#define BAD_BITS_40_41        0x46B
#define BAD_BITS_42_43        0x46A
#define BAD_BITS_44_45        0x7C0
#define BAD_BITS_46_47        0x7C3
#define BAD_BITS_48_49        0x7CC
#define BAD_BITS_50_51        0x7CE
#define BAD_BITS_52_53        0x7CB
#define BAD_BITS_54_55        0x7CA
#define BAD_BITS_56_57        0xDE0
#define BAD_BITS_58_59        0xDE3
#define BAD_BITS_60_61        0xDEC
#define BAD_BITS_62_63        0xDEA
#define BAD_BITS_64_65        0x920
#define BAD_BITS_66_67        0x923
#define BAD_BITS_68_69        0x92C
#define BAD_BITS_70_71        0x92E
#define BAD_BITS_72_73        0x92B
#define BAD_BITS_74_75        0x92A
#define BAD_BITS_76_77        0xA30
#define BAD_BITS_78_79        0xA33
#define BAD_BITS_80_81        0xA3C
#define BAD_BITS_82_83        0xA3E
#define BAD_BITS_84_85        0xA3B
#define BAD_BITS_86_87        0xA3A
#define BAD_BITS_88_89        0xCB0
#define BAD_BITS_90_91        0xCB3
#define BAD_BITS_92_93        0xCBC
#define BAD_BITS_94_95        0xCBA
#define BAD_BITS_96_97        0xD80
#define BAD_BITS_98_99        0xD83
#define BAD_BITS_100_101      0xD8C
#define BAD_BITS_102_103      0xD8E
#define BAD_BITS_104_105      0xD8B
#define BAD_BITS_106_107      0xD8A
#define BAD_BITS_108_109      0xFD0
#define BAD_BITS_110_111      0xFD3
#define BAD_BITS_112_113      0xFDC
#define BAD_BITS_114_115      0xFDE
#define BAD_BITS_116_117      0xFDB
#define BAD_BITS_118_119      0xFDA
#define BAD_BITS_120_121      0xF60
#define BAD_BITS_122_123      0xF63
#define BAD_BITS_124_125      0xF6C
#define BAD_BITS_126_127      0xF6A
#define BAD_BITS_C0_C1        0x033
#define BAD_BITS_C2_C3        0x03C
#define BAD_BITS_C4_C5        0x030
#define BAD_BITS_C6_C7        0x0F0
#define BAD_BITS_C8_C9        0x330
#define BAD_BITS_C10_C11      0xC30


/*
 * definitions for the OCP devices; the first byte (MSB) is the IIC address
 *                                  the second byte (LSB) is the bit mask
*/
/* this bits can be read and written */
enum ocp_led {cpu1_led = 0x4001, cpu1_led_mbz = 0x4002, mem1_led = 0x4004,
	      mem2_led = 0x4008, mem3_led     = 0x4010, mem4_led = 0x4020,
	      fbus_led = 0x4040, io_led       = 0x4080,
	      cpu2_led = 0x4201, cpu2_led_mbz = 0x4202, halt_led = 0x4204,
	      reset_led= 0x4210
	      };
/* read only bits */
enum ocp_rd_dev  {halt_switch = 0x4208, bitrate = 0x42E0};

#endif /* IIC_H */
