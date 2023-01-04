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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: amd79c30.c,v $ $Revision: 1.1.4.11 $ (DEC) $Date: 1993/11/04 16:59:08 $";
#endif

/*
 * Old HISTORY
 * Revision 1.1.2.14  92/11/18  10:36:15  Narayan_Mohanram
 * 	"Changes for HDLC and bug fixes"
 * 
 * Revision 1.1.5.2  92/11/11  14:45:25  Narayan_Mohanram
 * 	Fixes for intermittent problems, latency performance, several new ioctls
 * 
 * Revision 1.1.2.13  92/10/14  16:46:26  Narayan_Mohanram
 * 	Changed number of minor devices to 8. Also cleaned up better behind voice
 * 	close --narayan
 * 	[92/10/14  16:41:03  Narayan_Mohanram]
 * 
 * Revision 1.1.2.12  92/10/07  17:06:00  Narayan_Mohanram
 * 	Turn off the MCR3 connections on close. Also forgot to turn of B2VOCE
 * 	bit properly-narayan
 * 	[92/10/07  16:58:40  Narayan_Mohanram]
 * 
 * Revision 1.1.2.11  92/09/03  16:12:08  Narayan_Mohanram
 * 	Allowed the specification of chan on 56KB connections -narayan
 * 	[92/09/03  16:06:54  Narayan_Mohanram]
 * 
 * Revision 1.1.2.10  92/08/28  10:59:46  Ron_Bhanukitsiri
 * 	Modified driver for Alpha BL8
 * 	[92/08/28  10:58:57  Ron_Bhanukitsiri]
 * 
 * Revision 1.1.2.9  92/08/19  16:02:10  Narayan_Mohanram
 * 	Added alpha changes. Also fixed some dma recevice bug on mips -narayan
 * 	[92/08/19  15:55:35  Narayan_Mohanram]
 * 
 * Revision 1.1.2.8  92/08/03  13:59:30  Narayan_Mohanram
 * 	More ODE screwups
 * 	[92/08/03  13:58:12  Narayan_Mohanram]
 * 
 * Revision 1.1.3.2  92/06/22  10:50:34  Narayan_Mohanram
 * 	Added support for 56KB connections --narayan
 * 
 * Revision 1.1.2.3  92/06/08  08:44:22  Narayan_Mohanram
 * 	Fixed bba to work for BL3 (this was abandoned). Now it is similar to lofi
 * 		-narayan-
 * 	[92/06/05  13:33:16  Narayan_Mohanram]
 * 
 * Revision 1.1.2.2  92/04/19  17:01:48  Ron_Bhanukitsiri
 * 	"BL2 - Bellcore Certification"
 * 	[92/04/19  17:00:40  Ron_Bhanukitsiri]
 */
 
 
/************************************************************************
 * Modification History
 *
 *    7/17/90 
 *       prototype bba driver hacked by Rich Hyde
 *    
 *
 ************************************************************************/

#include <sys/param.h>
#include <sys/types.h>
#include <sys/conf.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/sysconfig.h>
#include <io/common/devdriver.h>        /* auto-config headers */
#include <hal/cpuconf.h>
#if defined(__alpha)
#include <io/dec/tc/ioasic.h>
#include <io/dec/tc/tc.h>
#elif defined(mips)
#include <hal/kn02ca.h>
#else
#include <xxxxxxxx>
#endif /* alpha */
#define DEBUG_BBA
#include <io/dec/tc/amd79c30_dlld_drv.h>
#include <io/dec/tc/amd79c30_reg.h>
#include <io/dec/tc/amd79c30.h>
#include <io/dec/tc/codecioc.h>
#if defined(__alpha)
#define stqp(lp,val)	{*(u_long *)(lp) = (u_long)(val);}
#elif defined(mips)
#define mb()
#else
xxxx
#endif /* mips and alpha */

u_char BBA_b1_outdata [2048], BBA_b2_outdata [2048];
u_char *BBA_b1_next, *BBA_b2_next;
int BBA_b1_len, BBA_b2_len;
u_char *bba_dma_buffers;
u_char bba_dma_space [DMA_BOUNDARY * 5];
int bba_nointr;
int bba_hdlc_flag = 0x7e;
int bba_current_sample;		/* User controllable output count */
ulong bba_record;		/* Data in from CODEC (recording) */
ulong bba_play;			/* Data out to CODEC (playback) */

struct BBA_softc BBA_softc;
struct device *bba_devinfo [1];
struct controller *bba_ctlr;
caddr_t bba_csr [1];
struct bba_intr_info bba_intr_info;

struct bba_gain_coeff
{
	unsigned short	coeff;
	int	gain;		/* in dB */
};

struct bba_gain_coeff bba_gx_coeff [] = {
#define BBA_GX_DEFAULT 0
	{ 0x0808, 0 },			/* LSB followed by MSB and Gain*/
	{ 0x4cb2, 5 },
	{ 0x3dac, 10 },
	{ 0x2ae5, 15 },
	{ 0x2533, 20 },
	{ 0x2222, 25 },			/* 5 */
	{ 0x2122, 30 },
	{ 0x1fd3, 35 },
	{ 0x12a2, 40 },
	{ 0x121b, 45 },
	{ 0x113b, 50 },			/* 10 */
	{ 0x0bc3, 55 },
	{ 0x10f2, 60 },
	{ 0x03ba, 65 },
	{ 0x02ca, 70 },
	{ 0x021d, 75 },			/* 15 */
	{ 0x015a, 80 },
	{ 0x0122, 85 },
	{ 0x0112, 90 },
	{ 0x00ec, 95 },
	{ 0x0032, 100 },		/* 20 */
	{ 0x0021, 105 },
	{ 0x0013, 110 },
	{ 0x0011, 115 },
	{ 0x000e, 120 },		/* 24 */
	{ 0x9008, 0x8000 }		/* Infinity */
};
struct bba_gain_coeff bba_gr_coeff [] = {
	{ 0x91f9, -120 },	/* 0 */
	{ 0x91c5, -115 },
	{ 0x91b6, -110 },
	{ 0x9212, -105 },
	{ 0x91a4, -100 },
	{ 0x9222, -95 },	/* 5 */
	{ 0x9232, -90 },
	{ 0x92fb, -85 },
	{ 0x92aa, -80 },
	{ 0x9327, -75 },
	{ 0x93b3, -70 },	/* 10 */
	{ 0x94b3, -65 },
	{ 0x9f91, -60 },
	{ 0x9cea, -55 },
	{ 0x9bf9, -50 },
	{ 0x9aac, -45 },	/* 15 */
	{ 0x9a4a, -40 },
	{ 0xa222, -35 },
	{ 0xa2a2, -30 },
	{ 0xa68d, -25 },
	{ 0xaaa3, -20 },	/* 20 */
	{ 0xb242, -15 },
	{ 0xbb52, -10 },
	{ 0xcbb2, -5 },
#define BBA_GR_DEFAULT 24
	{ 0x0808, 0 },		/* 24 */
	{ 0x9008, 0x8000 }	/* Infinity */
};
struct bba_gain_coeff bba_stg_coeff [] = {
#define BBA_STG_DEFAULT 0
	{ 0x8b7c, -180 },	/* 0 */
	{ 0x8b44, -175 },
	{ 0x8b35, -170 },
	{ 0x8b2a, -165 },
	{ 0x8b24, -160 },
	{ 0x8b22, -155 },	/* 5 */
	{ 0x9123, -150 },
	{ 0x912e, -145 },
	{ 0x912a, -140 },
	{ 0x9132, -135 },
	{ 0x913b, -130 },	/* 10 */
	{ 0x914b, -125 },
	{ 0x91f9, -120 },
	{ 0x91c5, -115 },
	{ 0x91b6, -110 },
	{ 0x9212, -105 },	/* 15 */
	{ 0x91a4, -100 },
	{ 0x9222, -95 },
	{ 0x9232, -90 },
	{ 0x92fb, -85 },
	{ 0x92aa, -80 },	/* 20 */
	{ 0x9327, -75 },
	{ 0x93b3, -70 },
	{ 0x94b3, -65 },
	{ 0x9f91, -60 },
	{ 0x9cea, -55 },	/* 25 */
	{ 0x9bf9, -50 },
	{ 0x9aac, -45 },
	{ 0x9a4a, -40 },
	{ 0xa222, -30 },
	{ 0xa2a2, -30 },	/* 30 */
	{ 0xa68d, -25 },
	{ 0xaaa3, -20 },
	{ 0xb242, -15 },
	{ 0xbb52, -10 },
	{ 0xcbb2, -5 },		/* 35 */
	{ 0x0808, 0 },
	{ 0x9008, 0x8000 }		/* Infinity */
};
struct bba_gain_coeff bba_ger_coeff [] = {
	{ 0xaaaa, -100 },	/* 0 */
	{ 0x9bbb, -95 },
	{ 0x79ac, -90 },
	{ 0x099a, -85 },
	{ 0x4199, -80 },
	{ 0x3199, -75 },	/* 5 */
	{ 0x9cde, -70 },
	{ 0x9def, -65 },
	{ 0x749c, -60 },
	{ 0x549d, -55 },
	{ 0x6aae, -50 },	/* 10 */
	{ 0xabcd, -45 },
	{ 0xabdf, -40 },
	{ 0x7429, -35 },
	{ 0x64ab, -30 },
	{ 0x6aff, -25 },	/* 15 */
	{ 0x2abd, -20 },
	{ 0xbeef, -15 },
	{ 0x5cce, -10 },
	{ 0x75cd, -5 },
#define BBA_GER_DEFAULT 20
	{ 0x0099, 0 },		/* 20 */
	{ 0x554c, 5 },
	{ 0x43dd, 10 },
	{ 0x33dd, 15 },
	{ 0x52ef, 20 },
	{ 0x771b, 25 },		/* 25 */
	{ 0x5542, 30 },
	{ 0x41dd, 35 },
	{ 0x31dd, 40 },
	{ 0x441f, 45 },
	{ 0x431f, 50 },		/* 30 */
	{ 0x331f, 55 },
	{ 0x40dd, 60 },
	{ 0x11dd, 65 },
	{ 0x440f, 70 },
	{ 0x411f, 75 },		/* 35 */
	{ 0x311f, 80 },
	{ 0x5520, 85 },
	{ 0x10dd, 90 },
	{ 0x4211, 95 },
	{ 0x410f, 100 },	/* 40 */
	{ 0x111f, 105 },
	{ 0x600b, 110 },
	{ 0x00dd, 115 },
	{ 0x4210, 120 },
	{ 0x400f, 125 },	/* 45 */
	{ 0x110f, 130 },
	{ 0x2210, 134 },
	{ 0x7200, 140 },
	{ 0x4200, 145 },
	{ 0x2110, 150 },	/* 50 */
	{ 0x100f, 155 },
	{ 0x2200, 159 },
	{ 0x1110, 166 },
	{ 0x000b, 169 },
	{ 0x2100, 175 },	/* 55 */
	{ 0x000f, 180 },
	{ 0x0008, 0x8000 }	/* Infinity */
};
struct bba_gain_coeff bba_ga_coeff [] = {
#define BBA_GA_DEFAULT 0
	{ 0x0000, 0 },
	{ 0x0010, 6 },
	{ 0x0020, 12 },
	{ 0x0030, 18 },
	{ 0x0040, 24 },
	{ 0x0000, 0x8000 }	/* Infinity - not really used */
};
unsigned char bba_dtmf_coeff [] = {
	0x79, 0xab, /* 0 */
	0x5a, 0x9b, /* 1 */
	0x5a, 0xab, /* 2 */
	0x5a, 0xbf,
	0x63, 0x9b,
	0x63, 0xab, /* 5 */
	0x63, 0xbf,
	0x6e, 0x9b,
	0x6e, 0xab, /* 8 */
	0x6e, 0xbf, /* 9 */
	0x79, 0x9b, /* * */
	0x79, 0xbf, /* # */
};

#define BBA_GR_TOTAL ((sizeof (bba_gr_coeff) / sizeof(struct bba_gain_coeff)) -2)
#define BBA_GX_TOTAL ((sizeof (bba_gx_coeff) / sizeof(struct bba_gain_coeff)) -2)
#define BBA_GER_TOTAL ((sizeof (bba_ger_coeff) / sizeof(struct bba_gain_coeff)) -2)
#define BBA_STG_TOTAL ((sizeof (bba_stg_coeff) / sizeof(struct bba_gain_coeff)) -2)
#define BBA_GA_TOTAL ((sizeof (bba_ga_coeff) / sizeof(struct bba_gain_coeff)) -2)

/* Used by bba_Gains and bba_Get_Gain_Param */
static struct
{
	struct bba_gain_coeff *gp;
	int	total_entries;
	int	map_reg;
	int	cmd;
	int	step;
	int	last_setting;
} *rp, rs[] = {
{ bba_gr_coeff, BBA_GR_TOTAL, MAP_GR, BBA_SET_VOICE_OUT_GAIN, 5, BBA_GR_DEFAULT },
{ bba_ger_coeff, BBA_GER_TOTAL, MAP_GER, BBA_SET_H_I_GAIN, 5, BBA_GER_DEFAULT },
{ bba_ga_coeff, BBA_GA_TOTAL, 0, BBA_SET_PREAMP_GAIN, 60, BBA_GA_DEFAULT },
{ bba_gx_coeff, BBA_GX_TOTAL, MAP_GX, BBA_SET_IN_GAIN, 5, BBA_GX_TOTAL },
{ bba_stg_coeff, BBA_STG_TOTAL, MAP_STG, BBA_SET_FEEDBACK_GAIN, 5, BBA_STG_DEFAULT },
{ 0 }
};

struct bba_audio_dev_cap bba_audio_dev_cap_table[] =
{
	{ 8000, 8, BBA_DMASIZE, 1, BBA_DMASIZE, 1, 1,
		  BBA_DT_MU_LAW|BBA_DT_A_LAW, BBA_DT_MU_LAW|BBA_DT_A_LAW,
		  0, "AMD 79C30" } /* We currently only support 8000 */
};

/**/
#ifdef __alpha
extern unsigned int scc_ioasic_base;         /* Variable base address of IOASIC*/
extern int cpu;
static unsigned char *bba_ssr_p, *bba_sir_p, *bba_tc_reg_p;

unsigned int
bba_read_ssr()
{
	mb();
	return (*((unsigned int *)PHYS_TO_KSEG(bba_ssr_p)));
}
void
bba_write_ssr(unsigned int x)
{
	*((unsigned int *)PHYS_TO_KSEG(bba_ssr_p)) = x;
	mb();
}
unsigned int
bba_read_sir()
{
	mb();
	return (*((unsigned int *)PHYS_TO_KSEG(bba_sir_p)));
}
void
bba_write_sir(unsigned int x)
{
	*((unsigned int *)PHYS_TO_KSEG(bba_sir_p)) = x;
	mb();
}
#endif /* __alpha */
/*
 * Definition of the driver for the auto-configuration program.
 */
bba_configure (sysconfig_op_t op,
       str_config_t *indata,
       size_t indatalen,
       str_config_t *outdata,
       size_t outdatalen)
{
	dev_t devno;
	struct streamadm sa;

	BBA_DEBUG (BBAD_CONF, ("bba: Config.."));
	if (op != SYSCONFIG_CONFIGURE)
		return EINVAL;

	if (indata != NULL && indatalen == sizeof (str_config_t)
	    && indata->sc_version == OSF_STREAMS_CONFIG_10)
		devno = indata->sc_devnum;
	else
		devno = NODEV;

	sa.sa_version = OSF_STREAMS_10;
	sa.sa_flags = STR_IS_DEVICE;
	sa.sa_ttys = sa.sa_sync_info = NULL;
	sa.sa_sync_level = SQLVL_QUEUE;
	strcpy (sa.sa_name, "bba");

	if ((devno = strmod_add (devno, &bbatab, &sa)) == NODEV)
		return ENODEV;

	if (outdata != NULL && outdatalen == sizeof (str_config_t)) {
		outdata->sc_version = OSF_STREAMS_CONFIG_10;
		outdata->sc_devnum = makedev (major (devno), minor (devno));
		outdata->sc_sa_flags = sa.sa_flags;
		strcpy (outdata->sc_sa_name, sa.sa_name);
	}
	BBA_DEBUG (BBAD_CONF, ("bba: device %x (%d,%d) Configured OK\r\n",
			       outdata->sc_devnum, major (devno), minor (devno)));
	return 0;

}

bbaprobe (volatile char *nvx,
	  struct controller *ctlr)
{
	register int unit = ctlr->ctlr_num;

	BBA_DEBUG (BBAD_CONF, ("bba: unit %d ", unit));
	if (unit != 0)
		return 0;
#if defined(__alpha)

	if (cpu == DEC_3000_300)
	{
#define SLOT_5_ADDR 0x1b0000000
#define IOASIC_ADDR SLOT_5_ADDR	/* slot 7 is really the core I/O ASIC */
#define KN16AA_ISDN_ADDR	(IOASIC_ADDR + 0x480000)
#define KN16AA_SSR	0x1a0040100
#define KN16AA_SIR	0x1a0040110
		bba_ssr_p = (unsigned char *)KN16AA_SSR;
		bba_sir_p = (unsigned char *)KN16AA_SIR;
		bba_tc_reg_p = (unsigned char *)(SLOT_5_ADDR + 0x80000);
		BBA_softc.bs_bbamap = (u_long *)PHYS_TO_KSEG (KN16AA_ISDN_ADDR);
	}
	else
	{
#undef IOASIC_ADDR
#define SLOT_7_ADDR 0x1f0000000
#define IOASIC_ADDR SLOT_7_ADDR	/* slot 7 is really the core I/O ASIC */
#define TC_ISDN_ADDR	(IOASIC_ADDR + 0x480000)
#define KN15AA_SSR	0x1e0040100
#define KN15AA_SIR	0x1e0040110
		bba_ssr_p = (unsigned char *)KN15AA_SSR;
		bba_sir_p = (unsigned char *)KN15AA_SIR;
		bba_tc_reg_p = (unsigned char *)(SLOT_7_ADDR + 0x80000);
		BBA_softc.bs_bbamap = (u_long *)PHYS_TO_KSEG (TC_ISDN_ADDR);
#undef IOASIC_ADDR
	}
#elif defined(mips)
	BBA_softc.bs_bbamap = (volatile struct codec_reg *)PHYS_TO_K1(CODEC_REG_ADDR);
#else
	xxx
#endif
	BBA_DEBUG (BBAD_CONF, ("bba: address %lx\n", BBA_softc.bs_bbamap));
	bba_ctlr = ctlr;
	return 1;
}

bbaattach (struct controller *ctlr)
{
	unsigned char data = 4;
	/*
	 * Read the IR register so that the INT pin goes low.
	 * Set the chip in lo-power mode and disable it.
	 */
#if defined(mips)
	*((volatile u_int *)PHYS_TO_K1(MACH_SSR_ADDR)) 
		&= ~(SSR_XMT_EN | SSR_RCV_EN | SSR_CODEC_RESET);
	*(volatile u_int *)PHYS_TO_K1 (MACH_SSR_ADDR) |= (SSR_CODEC_RESET);
#elif defined(__alpha)
#if 0
	pmap_map_io(IOASIC_ADDR + 0x4000, 0x80000, bba_ioasicp,
		    VM_PROT_READ | VM_PROT_WRITE, TB_SYNC_ALL);
#endif
	mb();
	bba_write_ssr (bba_read_ssr() & 
		~(SSR_XMT_EN|SSR_RCV_EN|SSR_CODEC_RESET));
	bba_write_ssr (bba_read_ssr () | SSR_CODEC_RESET);
#endif /* __alpha */
	bba_codecindirect (INIT_1, 1, &data, BBA_CODEC_WRITE);
	BBA_DEBUG (BBAD_CONF,("bba:set to idle/low-power mode\n"));
	return 0;
}

void
bba_alloc_dma (void)
{	
	register unsigned char *dmap;
	register u_int x;
	vm_offset_t phys;
	kern_return_t svatophys ();

#ifdef notdef
	/*
	 *  Allocate memory for dma buffers This better be on a 2^n boundary
	 */
	if ((dmap = (u_char *)kalloc (DMA_BOUNDARY * 5)) == (u_char *)NULL) {
#ifdef BBA_DO_PRINTFS
		printf ("bba_alloc_dma: Cant alloc %d\n", DMA_BOUNDARY * 5);
#endif
		return;
	}
#else
	dmap = &bba_dma_space [0];
#endif
	bba_dma_buffers = dmap;
	if ((u_long)dmap & ((u_long)(DMA_BOUNDARY -1l))) {
		dmap += DMA_BOUNDARY;
		dmap = (u_char *)((u_long)dmap & ~((long)(DMA_BOUNDARY -1l)));
	}
	for (x = 0; x < 2; x++) {
		BBA_softc.bs_xbuf [x] = dmap;
		dmap += DMA_BOUNDARY;
		BBA_softc.bs_rbuf [x] = dmap;
		dmap += DMA_BOUNDARY;
		/*
		 * Point at DMASIZE from the end of the page (the idea being
		 * that at the end of the page we get a DMA interrupt).
		 */
		BBA_softc.bs_rbuf [x] += DMA_BOUNDARY - (BBA_DMASIZE*4);
		BBA_softc.bs_xbuf [x] += DMA_BOUNDARY - (BBA_DMASIZE*4);
#if defined(mips)
		BBA_softc.bs_rbuf [x] = (u_char *)K0_TO_K1 (BBA_softc.bs_rbuf [x]);
		/**
		 ** If we ever get DMA troubles on the transmit, we can
		 ** uncoment out the following...
		BBA_softc.bs_xbuf [x] = (u_char *)K0_TO_K1 (BBA_softc.bs_xbuf [x]);
		***/
#endif
		/*
		 * Calculate the value to put in the dma register
		 */
		if (svatophys ((vm_offset_t)BBA_softc.bs_rbuf [x], &phys)
			== KERN_INVALID_ADDRESS)
		{
#ifdef BBA_DO_PRINTFS
			printf ("bba_alloc_dma: Can't get phys addr for %lx\n",
				BBA_softc.bs_rbuf [x]);
#endif
			return;
		}
		BBA_softc.bs_rcv_dma_addr [x] = phys << 3;
		if (svatophys ((vm_offset_t)BBA_softc.bs_xbuf [x], &phys)
			== KERN_INVALID_ADDRESS)
		{
#ifdef BBA_DO_PRINTFS
			printf ("bba_alloc_dma:Can't get phys addr for %lx\n",
				BBA_softc.bs_xbuf [x]);
#endif
			return;
		}
		BBA_softc.bs_xmt_dma_addr [x] = phys << 3;
	}
	BBA_DEBUG (BBAD_CONF, ("bba:%d dma PHYS(RX %lx TX %lx) VIRT(%lx %lx)\n",
			       DMA_BOUNDARY * 5, BBA_softc.bs_rcv_dma_addr [0],
			       BBA_softc.bs_xmt_dma_addr [0],
			       BBA_softc.bs_rbuf [0], BBA_softc.bs_xbuf [0]));
}

void
bba_enable_dma (void)
{
	int x;

	/*
	 * Set up DMA pointers
	 * Set up the current pointer to buffer 0, the next to buffer 1 and
	 * set toggle so that buffer 0 will be filled on the next interrupt
	 */
	x = splimp();

	if (BBA_softc.bs_status & BBAS_DMA_ENABLED)
	{
		splx(x);
		return;
	}

	BBA_softc.bs_status |= BBAS_DMA_ENABLED;
	splx(x);

	if (BBA_softc.bs_rcv_dma_addr [0] == 0 || 
	    BBA_softc.bs_rcv_dma_addr [1] == 0 ||
	    BBA_softc.bs_xmt_dma_addr [0] == 0 ||
	    BBA_softc.bs_xmt_dma_addr [1] == 0)
	{
#ifdef BBA_DO_PRINTFS
		printf ("bba:cannot enable DMA (some null ptrs)\n");
#endif
		return;
	}
	/*** DEBUG (Fill with silence)****/
	for (x = 0; x < (DMA_BOUNDARY * 5); x++) {
		bba_dma_buffers [x] = 0xFF;
	}
	BBA_softc.bs_rind = BBA_softc.bs_xind = 0;
	/*
	 * Set up dma pointers (both current and buffer)
	 * Enable the dma. If we set the codec-reset bit to 0, then
	 * the chip is reset..
	 */
#if defined(mips)
	*(volatile u_int *)PHYS_TO_K1 (DMA_CUR_RCV) = 
		BBA_softc.bs_rcv_dma_addr [0];
	*(volatile u_int *)PHYS_TO_K1 (DMA_NEXT_RCV) =
		BBA_softc.bs_rcv_dma_addr [1];
	*(volatile unsigned long *)PHYS_TO_K1 (DMA_CUR_XMT) =
		BBA_softc.bs_xmt_dma_addr [0];
	*(volatile u_int *)PHYS_TO_K1 (DMA_NEXT_XMT) =
		BBA_softc.bs_xmt_dma_addr [1];
	*(volatile u_int *)PHYS_TO_K1 (MACH_SSR_ADDR) |= 
		(SSR_XMT_EN | SSR_RCV_EN | SSR_CODEC_RESET);
	BBA_DEBUG (BBAD_ENABLE, ("bba:Cur-Rcv %lx Next Rcv %lx\n",
		(volatile u_int *)PHYS_TO_K1 (DMA_CUR_RCV),
		(volatile u_int *)PHYS_TO_K1 (DMA_NEXT_RCV)));
#elif defined(__alpha)
	mb();
	*(u_int *)DMA_CUR_RCV = (u_int) BBA_softc.bs_rcv_dma_addr [0];
	*(u_int *)DMA_NEXT_RCV = (u_int) BBA_softc.bs_rcv_dma_addr [1];
	*(u_int *)DMA_CUR_XMT = (u_int) BBA_softc.bs_xmt_dma_addr [0];
	*(u_int *)DMA_NEXT_RCV = (u_int) BBA_softc.bs_xmt_dma_addr [1]; mb ();
	bba_write_ssr (bba_read_ssr () 
		| (SSR_XMT_EN | SSR_RCV_EN | SSR_CODEC_RESET));
#else
	xxxx
#endif
	BBA_DEBUG (BBAD_ENABLE, ("bba: DMA enabled\n"));
}

void
bba_disable_dma (void)
{
#if defined (mips)
	*(volatile u_int *)PHYS_TO_K1 (MACH_SSR_ADDR) &= 
		~(SSR_XMT_EN | SSR_RCV_EN);
#elif defined(__alpha)
	mb();
	bba_write_ssr (bba_read_ssr () & ~(SSR_XMT_EN | SSR_RCV_EN));
#else
	xxx
#endif
	BBA_DEBUG (BBAD_ENABLE, ("bba:DMA disabled\n"));
	BBA_softc.bs_status &= ~BBAS_DMA_ENABLED;
}

void
bba_enable_intr (void)
{
	unsigned char data [1];
	/*
	 * First Disable the codec and reset
	 * Clear out any residual interrupts, then enable the interrupts
	 */
#if defined(mips)
	*(volatile u_int *)PHYS_TO_K1(MACH_SSR_ADDR) &= 
		~(SSR_XMT_EN | SSR_RCV_EN);
	*(volatile u_int *)PHYS_TO_K1 (MACH_SIR_ADDR) &= ~BBA_INTR;
	/** *(volatile unsigned long *)PHYS_TO_K1(KN02CA_SIRM_ADDR) |= BBA_INTR;
	***/
#elif defined(__alpha)
	mb();
	bba_write_ssr (bba_read_ssr () & ~(SSR_XMT_EN | SSR_RCV_EN));
	bba_write_sir (~ISDN_INTR);
#else
	xxx
#endif
	tc_enable_option (bba_ctlr);
	bba_nointr = 0;
	BBA_softc.bs_status |= BBAS_ENABLED;
	/*
	 * Initialize the codec to voice and data
	 */
	bba_codecindirect (INIT_1, 1, &data [0], BBA_CODEC_READ);
	data [0] &= ~3;
	data [0] |= INIT_PM_ACTIVE_VOICE_DATA;
	bba_codecindirect (INIT_1, 1, &data [0], BBA_CODEC_WRITE);
	BBA_DEBUG (BBAD_ENABLE, ("bba:interrupts enabled\n"));
}

void
bba_disable_intr (void)
{
#if defined(mips)
	*(volatile u_int *)PHYS_TO_K1 (MACH_SSR_ADDR) &= 
		~(SSR_XMT_EN | SSR_RCV_EN);
	*(volatile u_int *)PHYS_TO_K1 (MACH_SIR_ADDR) &= ~BBA_INTR;
#elif defined(__alpha)
	mb();
	bba_write_ssr (bba_read_ssr () & ~(SSR_XMT_EN | SSR_RCV_EN));
#else
	xxx
#endif
	tc_disable_option (bba_ctlr);
	BBA_DEBUG (BBAD_ENABLE, ("bba: interrupts disabled\n"));
	BBA_softc.bs_status &= ~BBAS_ENABLED;
}

bba_map_disable (void)
{

	if (!(BBA_softc.bs_status & BBAS_MAPENABLED))
		return -1;
	BBA_softc.bs_status &= ~BBAS_MAPENABLED;
	BBA_DEBUG (BBAD_ENABLE, ("bba: MAP disabled\n"));
	return 0;
}

/*
 * Enable the Map part of the codec 
 */
bba_map_enable (void)
{
	unsigned char data [4];

	if (BBA_softc.bs_status & BBAS_MAPENABLED)
		(void) bba_map_disable ();
	/*
	 * map section
	 *  u-law  (MMR1)
	 *  Ear1/Ear2 and AINA (MMR2)
	 *  Mute-Off (Enable AINA) Ear/LS enabled 0db gain
	 *  All these are done by the various bits being 0.
	 */
	data [0] = 0x00; /* No-Digital Loopback */
	bba_codecindirect (MAP_MMR1, 1, data, BBA_CODEC_WRITE); 
	data [0] = 0;
	bba_codecindirect (MAP_MMR2, 1, data, BBA_CODEC_WRITE);
	data [0] = MAP_MMR3_BITS_MUTE; /* 0 db gain, Ear and Ls enabled */
	bba_codecindirect (MAP_MMR3, 1, data, BBA_CODEC_WRITE); 
	bba_codecindirect (MAP_GX, 2, (unsigned char *)&bba_gx_coeff [BBA_GX_DEFAULT].coeff,
			   BBA_CODEC_WRITE); 
	BBA_softc.bs_gx = BBA_GX_DEFAULT;
	bba_codecindirect (MAP_STG, 2, (unsigned char *)&bba_stg_coeff [BBA_STG_DEFAULT].coeff,
			   BBA_CODEC_WRITE);
	bba_codecindirect (MAP_GR, 2, (unsigned char *)&bba_gr_coeff [BBA_GR_DEFAULT].coeff,
			   BBA_CODEC_WRITE);
	BBA_softc.bs_gr = BBA_GR_DEFAULT;
	bba_codecindirect (MAP_GER, 2, (unsigned char *)&bba_ger_coeff [BBA_GER_DEFAULT].coeff,
			   BBA_CODEC_WRITE); 
	BBA_softc.bs_ger = BBA_GER_DEFAULT;
	data [0] = data [1] = 0x10;
	bba_codecindirect (MAP_SEQ_ATGR1_to_2, 2, data, BBA_CODEC_WRITE); 
	/*
	 * Load GX, GR, GER, Side-Tone coeff's from register
	 */
	data [0] = 0x4e;
	bba_codecindirect (MAP_MMR1, 1, data, BBA_CODEC_WRITE);
	BBA_softc.bs_status |= BBAS_MAPENABLED;
	BBA_DEBUG (BBAD_ENABLE, ("bba: MAP enabled\n"));
	return 0;
}	

void
bba_recover (void)
{
	/* TODO: Is this really necessary given the IOASIC? */
#define SPIN400ns 10

	volatile int spin;
	/*
	 * This code path should enforce a recovery time at
	 * the codec of > 400 ns.  This is 10 instructions on a
	 * 25 MHz clock so it is probably easiest to just spin.
	 * (Each C instruction has about 3 or 4 machine instructions)
	 */
	for (spin = 0; spin < SPIN400ns; spin++)
		;
}

/*
 * Read CODEC register
 */
/* ARGSUSED */
bba_codec_read (int dunit, int regno)
{
	register int value;
#if defined(mips)
	register volatile struct codec_reg *regp;

	regp = BBA_softc.bs_bbamap;
	bba_recover();
	value = regp [regno].reg_data;
#elif defined(__alpha)
	register volatile u_long *regp;

	regp = (volatile u_long *)BBA_softc.bs_bbamap;
	value = (regp [regno] >> 8) & 0xff;
#else
	xxx
#endif
	BBA_DEBUG (BBAD_RDDSC, ("bba_codec_read: regno %x value %x\n",
				regno, value));
	return value;
}

/*
 * Write CODEC register
 */
/* ARGSUSED */
void
bba_codec_write (int dunit, int regno, unsigned char value)
{
#if defined(mips)
	register volatile struct codec_reg *regp;

	regp = BBA_softc.bs_bbamap;
	bba_recover ();
	regp [regno].reg_data = value;
#elif defined(__alpha)
	register volatile u_long *regp = (volatile u_long *)BBA_softc.bs_bbamap;

	stqp (&regp [regno], (u_long)(1l << 32)| ((value & 0xff) << 8));
	mb ();
#else
	xxxx
#endif
	BBA_DEBUG (BBAD_WRDSC, ("bba_codec_write: regno %x value %x\n",
				regno, value));
}

/*
 * Read or Write indirect codec register
 */
void
bba_codecindirect (int regno,
		   int length,
		   unsigned char *data,
		   int dir)	/* direction of transfer (CODEC_READ/CODEC_WRITE) */
{
	register int s = splimp ();
#if defined(mips)
	register volatile struct codec_reg *regptr;

	regptr = BBA_softc.bs_bbamap;
	regptr [CR_OFFS].reg_data = regno;
	if (dir == BBA_CODEC_WRITE) {
		while (length--) {
			bba_recover ();
			regptr [DR_OFFS].reg_data = *data++;
		}
	} else {
		while (length--) {
			bba_recover ();
			*data++ = regptr [DR_OFFS].reg_data ;
		}
	}
#elif defined(__alpha)
	register volatile u_long *regptr;

	/*
	 * Remember that all the registers are mapped on longword
	 * boundaries in the dense address space. But in the
	 * sparse address space they are mapped into the
	 * quad word boundary.(So we need to multiply reg-off by 2)
	 * (Remember that regptr points to long-word)
	 */
	regptr = (u_long *)BBA_softc.bs_bbamap;
	BBA_DEBUG (BBAD_WRDSC|BBAD_RDDSC,
		("bba_codecindirect:regno %x len %d val %lx @location %lx\n",
			regno, length, (1l<<32) |((regno & 0xff) << 8),
			regptr + CR_OFFS));
	stqp ((u_long)(regptr + CR_OFFS), (1l<<32)| ((regno & 0xff) << 8));
	mb ();
	if (dir == BBA_CODEC_WRITE) {
		while (length--) {
			BBA_DEBUG (BBAD_WRDSC|BBAD_RDDSC, ("bba_codecindirect:writing %d to %lx\n", *data, regptr + DR_OFFS));

			stqp ((u_long)(regptr + DR_OFFS), 
				(1l<<32) | ((*data++) << 8));
			mb ();
		}
	} else while (length--) {
		*data++ = regptr [DR_OFFS] >> 8;
		mb ();
	}
#else
	xxx
#endif
	splx (s);
}

/* ARGSUSED */
bbaopen (queue_t *q,		/* pointer to read queue */
	 dev_t dev,		/* major/minor device number */
	 int flag,
	 int sflag)
{
	register int mdev;

	/*
	 * Initialize the DLLD code.
	 */
	if (!(BBA_softc.bs_status & BBAS_DLLDINIT)) {
		bba_alloc_dma ();
		if ((BBA_softc.bs_dunit = dlld_unix_init (&bba_drv_funcs)) < 0) {
#ifdef BBA_DO_PRINTFS
			printf ("Bba:Could not init DLLD\n");
#endif
			u.u_error = EIO;
			return OPENFAIL;
		}
		bba_map_enable ();
		BBA_softc.bs_status |= BBAS_DLLDINIT;
		BBA_DEBUG (BBAD_CONF, ("bba: DLLD%d enabled\n", BBA_softc.bs_dunit));
	} 
	if (sflag == CLONEOPEN) {
		/*
		 * Clone-Opens mean that a B-Channell is being open'd
		 */
		for (mdev = BBA_BCHAN; mdev < BBA_MAXMINOR; mdev++)
			if (BBA_softc.bs_open [mdev].bo_rdq == (queue_t *)NULL)
				goto found;
		u.u_error = ENXIO;
		return OPENFAIL;
	} else {
                mdev = minor (dev);
	        BBA_DEBUG (BBAD_OPEN, ("bba: open:q %lx mdev %d\n", q, mdev));
	        if (mdev > BBA_MAXMINOR) {
		   u.u_error = EAGAIN;
		   return OPENFAIL;
	        } 
                if (q->q_ptr != NULL) {
	   	   /*
		    * This minor is already open. Don't allow duplicate
		    * opens of Dchanell or Audio.
		    */
		   if (mdev == BBA_DCHAN || mdev == BBA_AUDIO) {
			u.u_error = EBUSY;
			return OPENFAIL;
		   } 
		   /*
		    * Duplicate opens on maintainance and B chanell are allowed
		    */
		   BBA_DEBUG (BBAD_OPEN, ("bba: open: reopen minor %d\n", mdev));
		   return mdev;
	        } else if (((mdev == BBA_DCHAN) || (mdev == BBA_AUDIO)) && 
		            (BBA_softc.bs_open [mdev].bo_rdq)) {
                   /*
		    * Bogus fix when linked q's are closed, the os does
		    * not fill in q->qptr on reopen
		    */
		   u.u_error = EBUSY;
		   return OPENFAIL;
	        }
        }
	found:
	BBA_softc.bs_open [mdev].bo_rdq = q;
	q->q_ptr = WR (q)->q_ptr = (char *)&BBA_softc.bs_open [mdev];
	BBA_softc.bs_open [mdev].bo_proc = u.u_procp; /* For latter mapping */
	BBA_softc.bs_open [mdev].bo_chan = -1; /* Can't happen */
	BBA_softc.bs_open [mdev].bo_rmblk = (mblk_t *)NULL;
	BBA_softc.bs_open [mdev].bo_flags = 0; /*Initial State */
	BBA_softc.bs_nopens++;
	BBA_DEBUG (BBAD_OPEN, ("bba: opened minor %x opens %d status %x\n",
		mdev, BBA_softc.bs_nopens, BBA_softc.bs_status));
	if ((BBA_softc.bs_status & BBAS_ENABLED) == 0)
		bba_enable_intr ();
	return mdev;
}

/*
 * Close the bba device. Dump any pending data buffers. Stop the
 * DMA if this is the last open.
 */
bbaclose (queue_t *q)
{
	register struct BBA_open *bo;
	register int s;
	int spkr [2];

	s = splimp ();
	bo = (struct BBA_open *)q->q_ptr;
	ASSERT (bo != (struct BBA_open *)NULL);
	BBA_DEBUG (BBAD_OPEN, ("bbaclose: dev %d status %x flags %x\n",
			       bo - &BBA_softc.bs_open [0], BBA_softc.bs_status,
				bo->bo_flags));
	flushq (q, FLUSHALL);
	flushq (OTHERQ(q), FLUSHALL);
	if (bo->bo_rmblk) {
		freemsg (bo->bo_rmblk);
		bo->bo_rmblk = (mblk_t *)NULL;
	}
	spkr [0] = 0;
/*	if (bo->bo_flags & BOS_HANDSET)*/
		bba_Mic_Enable (bo, &spkr [0]);
/*	if (bo->bo_flags & BOS_EAR)*/
		bba_Ear_Enable (bo, &spkr [0]);
/*	if (bo->bo_flags & BOS_DIAL_TONE)*/
		bba_Dial_Tone (bo, &spkr [0]);
/*	if (bo->bo_flags & BOS_SPEAKER1) {*/
		spkr [1] = 1;
		bba_Speaker_Enable (bo, &spkr [0]);
/*	}*/
/*	if (BBA_softc.bs_mapin == bo)*/
		bba_hookup_map (bo, &spkr [0]);
/*	if (BBA_softc.bs_mapout == bo)*/
		bba_attach_speaker (bo, &spkr [0]);
/*	if (bo->bo_flags & (BOS_VOICE|BOS_ENABLED))*/
		bba_disable_voice (bo);
/*	if ((BBA_softc.bs_status & */
/*		(BBAS_MAPOUT|BBAS_B1|BBAS_B1VOICE|BBAS_B2|BBAS_B2VOICE)) == 0)*/
		bba_disable_dma ();
#ifdef ISDNIOC_SNOOP
/*	if (bo == BBA_softc.bs_snoop)*/
		BBA_softc.bs_snoop = (struct BBA_open *)NULL;
#endif /* ISDNIOC_SNOOP */
/*	if (--BBA_softc.bs_nopens == 0 && (BBA_softc.bs_status & BBAS_ENABLED))*/
		bba_disable_intr ();
	bo->bo_flags = 0;
	bo->bo_rdq = (queue_t *)NULL; 
	splx (s);
	return 0;
}

bba_Speaker_Enable (register struct BBA_open *bo,
		    int *spkr)	/* spkr [0] = enable/disable, spkr[1] = which spkr */
{
	unsigned char data;

	BBA_DEBUG (BBAD_SPEAKER, ("bba:Speaker-Enable: %d %s\n",
				  spkr [1], spkr [0] ? "on" : "off"));
	if (spkr [1] != 1)
		return EINVAL;
	if (spkr [0]) {
		/*
		 * Enable Spkeaker..
		 */
		if (BBA_softc.bs_status & BBAS_SPEAKER1)
			return EBUSY;
		/*
		 * Enable both ear-piece and LS. (Mute-off)
		 */
		bba_codecindirect (MAP_MMR3, 1, &data, 0);
		data |=  0x02;
		bba_codecindirect (MAP_MMR3, 1, &data, 1);
		BBA_softc.bs_status |= BBAS_SPEAKER1;
		bo->bo_flags |= BOS_SPEAKER1;
	} else {
		/*
		 * spkr[0] == 0 -> Disable Speaker..
		 */
		bba_codecindirect (MAP_MMR3, 1, &data, 0);
		data &= ~0x02;
		bba_codecindirect (MAP_MMR3, 1, &data, 1);
		BBA_softc.bs_status &= ~BBAS_SPEAKER1;
		bo->bo_flags &= ~BOS_SPEAKER1;
	}
#ifdef BBA_DO_PRINTFS
	printf("bba_speaker_enable\n");
#endif
	return 0;
}

bba_Ear_Enable (register struct BBA_open *bo,
		int *enable)
{
	unsigned char data;

	BBA_DEBUG (BBAD_SPEAKER, ("bba:Ear-Piece %s\n", enable [0] 
				  ? "Enable" : "Disable"));
	if (*enable) {
		/*
		 * Enable Ear-Piece
		 */
		if (BBA_softc.bs_status & BBAS_EAR)
			return EBUSY;
		/*
		 * Enable both ear-piece and LS. (Mute-off)
		 */
		if (BBA_softc.bs_status & BBAS_SPEAKER1) {
			bba_codecindirect (MAP_MMR3, 1, &data, BBA_CODEC_READ);
			data |= 0x02;
			bba_codecindirect (MAP_MMR3, 1, &data, BBA_CODEC_WRITE);
		}
		BBA_softc.bs_status |= BBAS_EAR;
		bo->bo_flags |= BOS_EAR;
	} else {
		/*
		 * Disable Ear..
		 */
		BBA_softc.bs_status &= ~BBAS_EAR;
		bo->bo_flags &= ~BOS_EAR;
	}

#ifdef BBA_DO_PRINTFS
	printf("bba_ear_enable\n");
#endif
	return 0;
}

bba_Mic_Enable (register struct BBA_open *bo,
		int *enable)
{
	unsigned char data;

	BBA_DEBUG (BBAD_SPEAKER, ("bba:Handset %s\n", enable [0] 
				  ? "Enable" : "Disable"));
	if (*enable) {
		/*
		 * Enable handset
		 */
		if (BBA_softc.bs_status & BBAS_HANDSET)
			return EBUSY;
		/*
		 * Mute-Off..
		 */
		bba_codecindirect (MAP_MMR3, 1, &data, BBA_CODEC_READ);
		data &= ~0x8;
		bba_codecindirect (MAP_MMR3, 1, &data, BBA_CODEC_WRITE);
		BBA_softc.bs_status |= BBAS_HANDSET;
		bo->bo_flags |= BOS_HANDSET;
	} else {
		/*
		 * Disable handset (Turn On Mute)
		 */
		bba_codecindirect (MAP_MMR3, 1, &data, BBA_CODEC_READ);
		data |= 0x8;
		bba_codecindirect (MAP_MMR3, 1, &data, BBA_CODEC_WRITE);
		BBA_softc.bs_status &= ~BBAS_HANDSET;
		bo->bo_flags &= ~BOS_HANDSET;
	}
#ifdef BBA_DO_PRINTFS
	printf("bba_mic_enable\n");
#endif
	return 0;
}

bba_hookup_map (struct BBA_open *bo,
		int *on)
{
	unsigned char data;

	if (*on) {
		if (BBA_softc.bs_mapin != (struct BBA_open *)NULL) {
#ifdef BBA_DO_PRINTFS
	 		printf ("bba_hookup_map: Map already taken\n");
#endif
			return EBUSY;
		}
		BBA_softc.bs_mapin = bo;
		/*
		 * Connect BD to BA..
		 */
		data = MUX_PORT_BA << 4 | MUX_PORT_BD;
		bba_codecindirect (MUX_MCR3, 1, &data, BBA_CODEC_WRITE);
		bba_enable_dma ();
	} else {
		if (BBA_softc.bs_mapin == (struct BBA_open *)NULL) {
#ifdef BBA_DO_PRINTFS
			printf ("bba_hookup_map: Map not taken\n");
#endif
			return EINVAL;
		}
		data = 0;
		bba_codecindirect (MUX_MCR3, 1, &data, BBA_CODEC_WRITE);
		BBA_softc.bs_mapin = (struct BBA_open *)NULL;
	}
#ifdef BBA_DO_PRINTFS
	printf("bba_hookup_map\n");
#endif
	return 0;
}

bba_attach_speaker (struct BBA_open *bo,
		    int *enable)
{
	unsigned char data;

	if (*enable) {
		BBA_softc.bs_status |= BBAS_MAPOUT;
		BBA_softc.bs_mapout = bo;
		/*
		 * Connect BD to BA..
		 */
		data = MUX_PORT_BA << 4 | MUX_PORT_BD;
		bba_codecindirect (MUX_MCR3, 1, &data, BBA_CODEC_WRITE);
		bba_enable_dma ();
	} else {
		data = 0;
		bba_codecindirect (MUX_MCR3, 1, &data, BBA_CODEC_WRITE);
		BBA_softc.bs_status &= ~BBAS_MAPOUT;
		BBA_softc.bs_mapout = (struct BBA_open *)NULL;
	}
#ifdef BBA_DO_PRINTFS
	printf("bba_attach_speaker\n");
#endif
	return 0;
}

bba_Volume_Change (register struct BBA_open *bo,
		   int *increase)
{
	BBA_DEBUG (BBAD_SPEAKER, ("bba:Vol-Change: %s\n",
				  *increase ? "Up" : "Down"));
	if (*increase) {
		if (BBA_softc.bs_ger == BBA_GER_TOTAL) {
#ifdef out_for_now
			if (BBA_softc.bs_gr < BBA_GR_TOTAL)
				bba_codecindirect (MAP_GR, 2,
					    &bba_gr_coeff[++BBA_softc.bs_gr*2].coeff, 1);
#endif /* out_for_now */
		} else if (BBA_softc.bs_ger < BBA_GER_TOTAL)
			bba_codecindirect (MAP_GER, 2, (unsigned char *)&bba_ger_coeff
					    [++BBA_softc.bs_ger*2].coeff, 1);
	} else {
#ifdef out_for_now
		if (BBA_softc.bs_gr == 0) {
			if (BBA_softc.bs_ger > 0)
				bba_codecindirect (MAP_GER, 2,
				    &bba_ger_coeff [--BBA_softc.bs_ger*2].coeff, 1);
		} else if (BBA_softc.bs_gr > 0)
			bba_codecindirect (MAP_GR, 2, &bba_gr_coeff
					    [--BBA_softc.bs_gr*2].coeff, 1);
#endif /* out_for_now */
		if (BBA_softc.bs_ger > 0) 
			bba_codecindirect (MAP_GER, 2, (unsigned char *)&bba_ger_coeff
					    [--BBA_softc.bs_ger*2].coeff, 1);
	}
#ifdef BBA_DO_PRINTFS
	printf("bba_volume_change\n");
#endif
	return 0;
}

void
bba_Tone (register struct BBA_open *bo, 
	  int freq1,
	  int freq2,
	  int on,
	  int which)
{
	unsigned char data [2];

	if (on) {
		data [0] = freq1; data [1] = freq2;
		bba_codecindirect (MAP_SEQ_FTGR1_to_2, 2, data, 1);
		data [0] = data [1] = 0x20;
		bba_codecindirect (MAP_SEQ_ATGR1_to_2, 2, data, 1);
		bba_codecindirect (MAP_MMR2, 1, data, 0);
		data [0] |= which;	/* Enable DTMF/ToneRinger */
		bba_codecindirect (MAP_MMR2, 1, data, 1);
		bo->bo_flags |= BOS_DIAL_TONE;
		BBA_DEBUG (BBAD_SPEAKER, ("bba:Tone (%d %d) on\n",
			freq1, freq1));
	} else {
		bba_codecindirect (MAP_MMR2, 1, data, 0);
		data [0] &= ~(MAP_MMR2_BITS_TRING|MAP_MMR2_BITS_DTMF);
			/* Disable DTMF */
		bba_codecindirect (MAP_MMR2, 1, data, 1);
		bo->bo_flags &= ~BOS_DIAL_TONE;
		BBA_DEBUG (BBAD_SPEAKER, ("bba:Tone (%d %d) off\n",
			freq1, freq1));
	}
}

/*
 * I think that audible ring is 440+480 Hz so I need to use dtmf
 */
bba_Tone_Ringer (register struct BBA_open *bo,
		 int *on)
{
	BBA_DEBUG (BBAD_SPEAKER, ("bba:Ring-Tone %s\n",
				  *on ? "on" : "off"));
	bba_Tone (bo, 0x37, 0x41, *on, MAP_MMR2_BITS_DTMF);
#ifdef BBA_DO_PRINTFS
	printf("bba_tone_ringer\n");
#endif
	return 0;
}

bba_Dial_Tone (register struct BBA_open *bo,
	       int *on)
{
	static int bba_Freq1 = 0x2c, bba_Freq2 = 0x37;

	BBA_DEBUG (BBAD_SPEAKER, ("bba:Dial-Tone %s\n",
				  *on ? "on" : "off"));
	bba_Tone (bo, bba_Freq1, bba_Freq2, *on, MAP_MMR2_BITS_DTMF);
#ifdef BBA_DO_PRINTFS
	printf("bba_dial_tone\n");
#endif
	return 0;
}

bba_DTMF (register struct BBA_open *bo,
	  int *args)
{
	BBA_DEBUG (BBAD_SPEAKER, ("bba:DTMF %d %s\n", args [0],
				  args [1] ? "on" : "off"));
	if (args [1]) {
		if (args [0] == '*')
			args [0] = 10;
		else if (args [0] == '#')
			args [0] = 11;
		else if ((args [0] < 0) || (args [0] > 11))
			/*
			 * Invalid DTMF key.
			 */
			return EINVAL;
		bba_Tone (bo, bba_dtmf_coeff [args [0] * 2], bba_dtmf_coeff
				[args [0] * 2 + 1], 1, MAP_MMR2_BITS_DTMF);
	} else
		bba_Tone (bo, 0, 0, 0, MAP_MMR2_BITS_DTMF);
#ifdef BBA_DO_PRINTFS
	printf("bba_dtmf\n");
#endif
	return 0;
}

void
bba_ack(register struct BBA_open *bo, register struct iocblk *iocp, mblk_t *mp)
{
#ifdef BBA_DO_PRINTFS
	printf("bba_ack\n");
#endif
	if (mp->b_cont)
		freemsg(mp->b_cont);

	mp->b_cont = NULL;

	iocp->ioc_count = 0;
	iocp->ioc_error = 0;
        iocp->ioc_rval = 0;
	mp->b_datap->db_type = M_IOCACK;
	
	putq(bo->bo_rdq, mp);
	qenable(bo->bo_rdq);
}
	
/* Used to allow application programs direct access to codec registers.
   USE OF THIS ROUTINE IS NOT ADVISED!
   */
bba_Codec_Indirect (register struct BBA_open *bo, register struct iocblk *iocp, mblk_t *mp,
		    struct bba_indirect *id)
{
	register int *ip;

	BBA_DEBUG (BBAD_CODINDIR, ("bba:Codec_Indirect: reg 0x%x, len %d, dir %d\n",
				   id->bi_regno, id->bi_len, id->bi_direction));

	/* Verify that the register specified is valid */

	for (ip = bba_valid_codec_regs; *ip; ip++)
		if (*ip == id->bi_regno)
			break;

	if (!*ip)
	{
		BBA_DEBUG (BBAD_CODINDIR, ("bba:Codec_Indirect: bad register value\n"));
		return EINVAL;
	}

	if (id->bi_direction != BBA_CODEC_READ &&
	    id->bi_direction != BBA_CODEC_WRITE)
	{
		BBA_DEBUG (BBAD_CODINDIR, ("bba:Codec_Indirect: bad direction\n"));
		return EINVAL;
	}

	bba_codecindirect (id->bi_regno, id->bi_len,
			   (unsigned char *)id + sizeof(*id), id->bi_direction);

	if (id->bi_direction == BBA_CODEC_READ)
	{
		register mblk_t *bp;
		int out_size = sizeof(struct bba_indirect) + id->bi_len;
		char *out_data = (char *)id;

		if (!canput(bo->bo_rdq))
			return EAGAIN;

		if (mp->b_cont)
			freemsg(mp->b_cont);
	
		if ((mp->b_cont = bp = allocb(out_size, BPRI_MED)) == NULL)
			return EAGAIN;

		MBLK_TYPE(bp) = M_DATA;

		bcopy(out_data, bp->b_wptr, out_size);
		bp->b_wptr += out_size;

		iocp->ioc_count = out_size;
		mp->b_datap->db_type = M_IOCACK;
	
		putq(bo->bo_rdq, mp);
		qenable(bo->bo_rdq);
#if 0
		if ((bp = allocb(sizeof(struct bba_indirect) + id->bi_len, BPRI_LO)) == NULL)
			return EAGAIN;

		MBLK_TYPE(bp) = M_COPYOUT;
		bcopy(id, bp->b_wptr, sizeof(struct bba_indirect) + id->bi_len);
		bp->b_wptr += sizeof(struct bba_indirect) + id->bi_len;

		putq(bo->bo_rdq, bp);
		qenable(bo->bo_rdq);
#endif
	} else
		bba_ack(bo, iocp, mp);
	return 0;
}

/* Set the various gains */

bba_Gains (register struct BBA_open *bo, register struct iocblk *iocp, mblk_t *mp,
	   int *args, int cmd)
{
	/* Handle
	   BBA_SET_VOICE_OUT_GAIN
	   BBA_SET_H_I_GAIN
	   BBA_SET_PREAMP_GAIN
	   BBA_SET_IN_GAIN
	   BBA_SET_FEEDBACK_GAIN
	   */
	register int idx, fa, sa;
	register mblk_t *bp;
	unsigned char c;
	int	rv;		/* return value */

	for (rp = rs; rp->cmd; rp++)
		if (rp->cmd == cmd)
			break;
	if (!rp->cmd)
		return EINVAL;

	if (!canput(bo->bo_rdq))
		return EAGAIN;

	fa = args[0];		/* First argument - type */
	sa = args[1];		/* Second argument - gain amount */

	if (fa == BBA_GAIN_PERCENTAGE)
	{
		if (sa > 100)
			sa = 100;
		else if (sa < 0)
			sa = 0;
		idx = sa / (100 / rp->total_entries);
		if (idx > rp->total_entries)
			idx = rp->total_entries;
		rv = idx * 100 / rp->total_entries;
	}
	else if (fa == BBA_GAIN_DB)
	{
		/* TODO: This should be a binary search */
		register struct bba_gain_coeff *gc;

		for (idx = 0, gc = rp->gp;
		     idx < rp->total_entries && gc->gain < sa;
		     gc++, idx++)
			;
		rv = rp->gp[idx].gain;
	}
	else if (fa == BBA_MUTE)
	{
		idx = rp->total_entries + 1;
		rv = 0;
	}
	else if (fa == BBA_STEP_VOLUME)
	{
		if (rp->last_setting == -1)
			return EINVAL;
		if (rp->last_setting + sa >= rp->total_entries)
			idx = rp->total_entries;
		else if (rp->last_setting + sa < 0)
			idx = 0;
		else
			idx = rp->last_setting + sa;
		rv = rp->gp[idx].gain;
	}
	else
	{
#if 0
		printf("bba_Gains: type of %d invalid\n", fa);
#endif
		return EINVAL;
	}
	
	rp->last_setting = idx;

	if (cmd == BBA_SET_PREAMP_GAIN)	/* Special case */
	{
		int x = splimp();
		if (fa == BBA_MUTE)
		{
			bba_codecindirect (MAP_MMR3, 1, &c, BBA_CODEC_READ);
			c |= MAP_MMR3_BITS_MUTE;
			c &= 0x7f; /* High bit must be written 0 (pg 3-32) */
			bba_codecindirect (MAP_MMR3, 1, &c, BBA_CODEC_WRITE);
		}
		else
		{
			bba_codecindirect (MAP_MMR3, 1, &c, BBA_CODEC_READ);
			/* Turn off current gain and mute (BBA_GA_MASK includes high bit) */
			c &= ~(MAP_MMR3_BITS_GA | MAP_MMR3_BITS_MUTE);
			c |= rp->gp[idx].coeff;
			bba_codecindirect (MAP_MMR3, 1, &c, BBA_CODEC_WRITE);
		}
		splx(x);
	}
	else
		bba_codecindirect(rp->map_reg, 2,
				  (unsigned char *)&rp->gp[idx].coeff, BBA_CODEC_WRITE);
	args[1] = rv;

{
	int out_size = sizeof(int) * 2;
	char *out_data = (char *)args;

	if (!canput(bo->bo_rdq))
		return EAGAIN;

	if (mp->b_cont)
		freemsg(mp->b_cont);
	
	if ((mp->b_cont = bp = allocb(out_size, BPRI_MED)) == NULL)
		return EAGAIN;

	MBLK_TYPE(bp) = M_DATA;

	bcopy(out_data, bp->b_wptr, out_size);
	bp->b_wptr += out_size;

	iocp->ioc_count = out_size;
	mp->b_datap->db_type = M_IOCACK;
	
	putq(bo->bo_rdq, mp);
	qenable(bo->bo_rdq);
}	
#if 0
	if ((bp = allocb(sizeof(int) * 2, BPRI_LO)) == NULL)
		return EAGAIN;

	MBLK_TYPE(bp) = M_COPYOUT;
	bcopy(args, bp->b_wptr, sizeof(int) * 2);
	bp->b_wptr += sizeof(int) * 2;

	putq(bo->bo_rdq, bp);
	qenable(bo->bo_rdq);
#endif
	return 0;
}

bba_Get_Gain_Param(register struct BBA_open *bo, register struct iocblk *iocp, mblk_t *mp,
		   register struct bba_get_gain_param *gp)
{
	register mblk_t *bp;

	if (!canput(bo->bo_rdq))
		return EAGAIN;

	for (rp = rs; rp->cmd; rp++)
		if (rp->cmd == gp->bba_call)
			break;
	if (!rp->cmd)
		return EINVAL;

	gp->minval = rp->gp[0].gain;
	gp->maxval = rp->gp[rp->total_entries].gain;
	gp->step = rp->step;
	gp->map_reg = rp->map_reg;
{
	int out_size = sizeof(*gp);
	char *out_data = (char *)gp;

	if (!canput(bo->bo_rdq))
		return EAGAIN;

	if (mp->b_cont)
		freemsg(mp->b_cont);
	
	if ((mp->b_cont = bp = allocb(out_size, BPRI_MED)) == NULL)
		return EAGAIN;

	MBLK_TYPE(bp) = M_DATA;

	bcopy(out_data, bp->b_wptr, out_size);
	bp->b_wptr += out_size;

	iocp->ioc_count = out_size;
	mp->b_datap->db_type = M_IOCACK;
	
	putq(bo->bo_rdq, mp);
	qenable(bo->bo_rdq);
}	
#if 0
	if ((bp = allocb(sizeof(*gp), BPRI_LO)) == NULL)
		return EAGAIN;

	MBLK_TYPE(bp) = M_COPYOUT;
	bcopy(gp, bp->b_wptr, sizeof(*gp));
	bp->b_wptr += sizeof(*gp);

	putq(bo->bo_rdq, bp);
	qenable(bo->bo_rdq);
#endif
	return 0;
}

bba_Set_Sample_Rate_Type(register struct BBA_open *bo, register struct iocblk *iocp, mblk_t *mp, int *sr)
{
	register mblk_t *bp;
	unsigned char c;
	int fq = sr[0], typ = sr[1], x;

	if (fq != 8000 ||
	    ((typ & BBA_DT_MU_LAW) && (typ & BBA_DT_A_LAW)) ||
	    (typ & ~(BBA_DT_MU_LAW | BBA_DT_A_LAW)))
		return EINVAL;

	if (!canput(bo->bo_rdq))
		return EAGAIN;

	x = splimp();
	bba_codecindirect (MAP_MMR1, 1, &c, BBA_CODEC_READ);
	if (typ & BBA_DT_MU_LAW || !typ)
		c &= 0xfe;
	else
		if (typ & BBA_DT_A_LAW)
			c |= 0x01;
	bba_codecindirect (MAP_MMR1, 1, &c, BBA_CODEC_WRITE);
	splx(x);

	if (c & 0x01)
		sr[1] = BBA_DT_A_LAW;
	else
		sr[1] = BBA_DT_MU_LAW;

	/* We currently only support 8000 */
{
	int out_size = sizeof(int) * 2;
	char *out_data = (char *)sr;

	if (!canput(bo->bo_rdq))
		return EAGAIN;

	if (mp->b_cont)
		freemsg(mp->b_cont);
	
	if ((mp->b_cont = bp = allocb(out_size, BPRI_MED)) == NULL)
		return EAGAIN;

	MBLK_TYPE(bp) = M_DATA;

	bcopy(out_data, bp->b_wptr, out_size);
	bp->b_wptr += out_size;

	iocp->ioc_count = out_size;
	mp->b_datap->db_type = M_IOCACK;
	
	putq(bo->bo_rdq, mp);
	qenable(bo->bo_rdq);
}
#if 0
	if ((bp = allocb(sizeof(int) * 2, BPRI_LO)) == NULL)
		return EAGAIN;

	MBLK_TYPE(bp) = M_COPYOUT;
	bcopy((char *)sr, bp->b_wptr, sizeof(int) * 2);
	bp->b_wptr += sizeof(int) * 2;

	putq(bo->bo_rdq, bp);
	qenable(bo->bo_rdq);
#endif
	return 0;
}

bba_Get_Sample_Rate_Type(register struct BBA_open *bo, register struct iocblk *iocp, mblk_t *mp, int *sr)
{
	register mblk_t *bp;
	unsigned char c;
	int	x;
	int	r[2] = {8000, 0};

	/* We currently only support 8000 */

	if (!canput(bo->bo_rdq))
		return EAGAIN;

	x = splimp();
	bba_codecindirect (MAP_MMR1, 1, &c, BBA_CODEC_READ);
	splx(x);

	if (c & 0x01)
		r[1] = BBA_DT_A_LAW;
	else
		r[1] = BBA_DT_MU_LAW;
{
	int out_size = sizeof(r);
	char *out_data = (char *)r;

	if (!canput(bo->bo_rdq))
		return EAGAIN;

	if (mp->b_cont)
		freemsg(mp->b_cont);
	
	if ((mp->b_cont = bp = allocb(out_size, BPRI_MED)) == NULL)
		return EAGAIN;

	MBLK_TYPE(bp) = M_DATA;

	bcopy(out_data, bp->b_wptr, out_size);
	bp->b_wptr += out_size;

	iocp->ioc_count = out_size;
	mp->b_datap->db_type = M_IOCACK;
	
	putq(bo->bo_rdq, mp);
	qenable(bo->bo_rdq);
}
#if 0
	if ((bp = allocb(sizeof(r), BPRI_LO)) == NULL)
		return EAGAIN;

	MBLK_TYPE(bp) = M_COPYOUT;
	bcopy((char *)r, bp->b_wptr, sizeof(r));
	bp->b_wptr += sizeof(r);

	putq(bo->bo_rdq, bp);
	qenable(bo->bo_rdq);
#endif
	return 0;
}

bba_Get_Audio_Dev_Cap_Table(register struct BBA_open *bo, register struct iocblk *iocp, mblk_t *mp,
			struct bba_audio_dev_cap *ignored)
{
	register mblk_t *bp;
	int out_size = sizeof(bba_audio_dev_cap_table);
	char *out_data = (char *)bba_audio_dev_cap_table;

	if (!canput(bo->bo_rdq))
		return EAGAIN;

	if (mp->b_cont)
		freemsg(mp->b_cont);
	
	if ((mp->b_cont = bp = allocb(out_size, BPRI_MED)) == NULL)
		return EAGAIN;

	MBLK_TYPE(bp) = M_DATA;

	bcopy(out_data, bp->b_wptr, out_size);
	bp->b_wptr += out_size;

	iocp->ioc_count = out_size;
	mp->b_datap->db_type = M_IOCACK;
	
	putq(bo->bo_rdq, mp);
	qenable(bo->bo_rdq);
	return 0;
}

bba_Get_Audio_Dev_Cap_Size(register struct BBA_open *bo, register struct iocblk *iocp, mblk_t *mp,
			   int *ignored)
{
	register mblk_t *bp;
	int	i;
	int out_size = sizeof(i);
	char *out_data = (char *)&i;

	if (!canput(bo->bo_rdq))
		return EAGAIN;

	if (mp->b_cont)
		freemsg(mp->b_cont);
	
	if ((mp->b_cont = bp = allocb(out_size, BPRI_MED)) == NULL)
		return EAGAIN;

	MBLK_TYPE(bp) = M_DATA;

	i = sizeof(bba_audio_dev_cap_table);

	bcopy(out_data, bp->b_wptr, out_size);
	bp->b_wptr += out_size;

	iocp->ioc_count = out_size;
	mp->b_datap->db_type = M_IOCACK;
	
	putq(bo->bo_rdq, mp);
	qenable(bo->bo_rdq);
#if 0
	if (!canput(bo->bo_rdq))
		return EAGAIN;

	if ((bp = allocb(sizeof(i), BPRI_LO)) == NULL)
		return EAGAIN;

	MBLK_TYPE(bp) = M_COPYOUT;
	bcopy((char *)&i, bp->b_wptr, sizeof(i));
	bp->b_wptr += sizeof(i);

	putq(bo->bo_rdq, bp);
	qenable(bo->bo_rdq);
#endif
	return 0;
}

bba_Set_Sample_Number(register struct BBA_open *bo,
			     int *set_to)
{
	register mblk_t *bp;

	bba_current_sample = *set_to;

	return 0;
}

bba_Get_Sample_Number(register struct BBA_open *bo, register struct iocblk *iocp, mblk_t *mp,
		      int *value)
{
	register mblk_t *bp;
	struct copyreq *cqp;
	int out_size = sizeof(bba_current_sample);
	char *out_data = (char *)&bba_current_sample;

	if (!canput(bo->bo_rdq))
		return EAGAIN;

	if (mp->b_cont)
		freemsg(mp->b_cont);
	
	if ((mp->b_cont = bp = allocb(out_size, BPRI_MED)) == NULL)
		return EAGAIN;

	MBLK_TYPE(bp) = M_DATA;

#ifdef BBA_DO_PRINTFS
	printf("current_sample = %d\n", bba_current_sample);
#endif

	bcopy(out_data, bp->b_wptr, out_size);
	bp->b_wptr += out_size;

	iocp->ioc_count = out_size;
	mp->b_datap->db_type = M_IOCACK;
	
	putq(bo->bo_rdq, mp);
	qenable(bo->bo_rdq);
	return 0;
}

bba_Get_Sample_Counts(register struct BBA_open *bo, register struct iocblk *iocp, mblk_t *mp,
		      int *value)
{
	register mblk_t *bp;
	struct copyreq *cqp;
	struct bba_sample_counts bba_sample_counts;
	int out_size = sizeof(bba_sample_counts);
	char *out_data = (char *)&bba_sample_counts;
#ifdef time			/* The streams module re-defines time!! */
#undef time
#define time_redefed
#endif
	extern struct timeval time;

	if (!canput(bo->bo_rdq))
		return EAGAIN;

	if (mp->b_cont)
		freemsg(mp->b_cont);
	
	if ((mp->b_cont = bp = allocb(out_size, BPRI_MED)) == NULL)
		return EAGAIN;

	bba_sample_counts.record = bba_record;
	bba_sample_counts.play = bba_play;
	bcopy((char *)&time, (char *)&bba_sample_counts.timestamp,
	      sizeof(struct timeval));

#ifdef time_redefed
#define time		streams_time()	
#endif

	MBLK_TYPE(bp) = M_DATA;

#ifdef BBA_DO_PRINTFS
	printf("record count = %d\nplayback count = %d\n", 
	       bba_record, bba_playback);
#endif

	bcopy(out_data, bp->b_wptr, out_size);
	bp->b_wptr += out_size;

	iocp->ioc_count = out_size;
	mp->b_datap->db_type = M_IOCACK;
	
	putq(bo->bo_rdq, mp);
	qenable(bo->bo_rdq);
	return 0;
}

bba_Get_Current_Gain(register struct BBA_open *bo, register struct iocblk *iocp, mblk_t *mp,
		   register struct bba_get_current_gain *gp)
{
	register struct bba_gain_coeff *gc;
	register mblk_t *bp;
	register int count;
	unsigned short coeff;
	unsigned char c;

	if (!canput(bo->bo_rdq))
		return EAGAIN;

	for (rp = rs; rp->cmd; rp++)
		if (rp->cmd == gp->bba_call)
			break;
	if (!rp->cmd)
		return EINVAL;

	if (rp->cmd == BBA_SET_PREAMP_GAIN)	/* Special case */
	{
		bba_codecindirect (MAP_MMR3, 1, &c, BBA_CODEC_READ);
		coeff = c & 0xf0;
#ifdef MIC_PREAMP_FIXED
		/* Can't read this from the chip!!! */
		if (c & MAP_MMR3_BITS_MUTE)
			gp->muted = 1;
		else
#endif
			gp->muted = 0;
	}
	else
		bba_codecindirect (rp->map_reg, sizeof(coeff),
				   (unsigned char *)&coeff,
				   BBA_CODEC_READ);

	for (gc = rp->gp, count = 0;
	     count < rp->total_entries + 2 && coeff != gc->coeff;
	     gc++, count++)
#ifdef BBA_DO_PRINTFS
		printf("Checking 0x%x against 0x%x\n",
		       coeff, gc->coeff);
#else
	;
#endif
	if (count >= rp->total_entries + 2)
	{
		/* This shouldn't happen!! */
#ifdef BBA_DO_PRINTFS
		printf("Coeff 0x%x not found for 0x%x\n",
		       coeff, rp->map_reg);
#endif
		return EFAULT;
	}
	if (rp->cmd != BBA_SET_PREAMP_GAIN)
	{
		if (count == rp->total_entries + 1)
			gp->muted = 1;
		else
			gp->muted = 0;
	}

	gp->gain_db = gc->gain;
	gp->gain_pct = count * 100 / rp->total_entries;
	
	gp->minval = rp->gp[0].gain;
	gp->maxval = rp->gp[rp->total_entries].gain;
	gp->step = rp->step;
	gp->map_reg = rp->map_reg;
{
	int out_size = sizeof(*gp);
	char *out_data = (char *)gp;

	if (!canput(bo->bo_rdq))
		return EAGAIN;

	if (mp->b_cont)
		freemsg(mp->b_cont);
	
	if ((mp->b_cont = bp = allocb(out_size, BPRI_MED)) == NULL)
		return EAGAIN;

	MBLK_TYPE(bp) = M_DATA;

	bcopy(out_data, bp->b_wptr, out_size);
	bp->b_wptr += out_size;

	iocp->ioc_count = out_size;
	mp->b_datap->db_type = M_IOCACK;
	
	putq(bo->bo_rdq, mp);
	qenable(bo->bo_rdq);
}	
	return 0;
}

/*
 * Deal with all ioctl's here..
 */
void
bbaioctl (queue_t *q, mblk_t *mp)
{
	struct iocblk *iocp;
	register struct BBA_open *bo;
	int unit, mdev, len, off, *ip, s;
	mblk_t *bp;
	queue_t *reply;

	iocp = MBLK_TO_DT (mp, struct iocblk *);

	iocp->ioc_error = 0;
	bo = (struct BBA_open *)q->q_ptr;
	ASSERT (bo != (struct BBA_open *)NULL);
	mdev = bo - &BBA_softc.bs_open [0];
	reply = bo->bo_rdq;

	if (mp->b_datap->db_type == M_IOCDATA)
	{
		mp->b_datap->db_type = M_IOCACK;
		mp->b_wptr = mp->b_rptr + sizeof(struct iocblk);
		iocp->ioc_error = 0;
		iocp->ioc_count = 0;
		iocp->ioc_rval = 0;
		putq (reply, mp);
		return;
	}
	
	/*
	 * Only One IOCTL at one time..
	 */
	s = splbba ();
	BBA_DEBUG (BBAD_IOC, ("bba: ioctl: type %x bo %lx reply %lx\n",
			      iocp->ioc_cmd, bo, reply));
	switch (iocp->ioc_cmd) {

#ifdef notyet_ready		/* It appears as though speed at 8k isn't
				   a major issue, even through streams */
	case BBAIOCMAPBBA:
		/*
		 * Map the BBA memory into user-space. This is so
		 * that users such as the Audio server may have
		 * access to the sound buffers directly.
		 */
		if ((map = BBA_map_region (lsp, bo, (char *)BBA_softc.bs_bbamap, 
				    1024 * 1024, 0600))  == (char *)NULL)
		{
			No_Mem:
			iocp->ioc_error = ENOMEM;
			goto Nack;
		}

		if (mp->b_cont != (mblk_t *)NULL)
			bp = mp->b_cont;
		else if ((bp = allocb (sizeof map, BPRI_HI))==(mblk_t *)NULL) {
			BBA_map_free (lsp, map);
			goto No_Mem;
		}
		*(char **)((bp)->b_rptr) = map;
		bp->b_wptr = bp->b_rptr + sizeof map;
		iocp->ioc_count = sizeof map;
		break;
#endif /* notyet */

	case BBAIOC_CODEC_INDIRECT: /* Read/Write codec registers */
		if (mp->b_cont == (mblk_t *)NULL)
		    goto Inval;
		if (iocp->ioc_error = bba_Codec_Indirect (bo, iocp, mp,
			      MBLK_TO_DT (mp->b_cont, struct bba_indirect *)))
		    goto Nack;
		break;
	
	case BBA_SET_VOICE_OUT_GAIN:
	case BBA_SET_H_I_GAIN:
	case BBA_SET_PREAMP_GAIN:
	case BBA_SET_IN_GAIN:
	case BBA_SET_FEEDBACK_GAIN:
		if (mp->b_cont == (mblk_t *)NULL)
		    goto Inval;
		if (iocp->ioc_error = bba_Gains (bo, iocp, mp,
			      MBLK_TO_DT (mp->b_cont, int *), iocp->ioc_cmd))
		    goto Nack;
		break;

	case BBA_GET_GAIN_PARAMETERS: /* What are the legitimate gain parameters? */
		if (mp->b_cont == (mblk_t *)NULL)
		    goto Inval;
		if (iocp->ioc_error = bba_Get_Gain_Param (bo, iocp, mp,
			      MBLK_TO_DT (mp->b_cont, struct bba_get_gain_param *)))
		    goto Nack;
		break;

	case BBA_SET_SAMPLE_RATE_TYPE:
		if (mp->b_cont == (mblk_t *)NULL)
		    goto Inval;
		if (iocp->ioc_error = bba_Set_Sample_Rate_Type (bo, iocp, mp,
			      MBLK_TO_DT (mp->b_cont, int *)))
		    goto Nack;
		break;

	case BBA_GET_SAMPLE_RATE_TYPE:
		if (mp->b_cont == (mblk_t *)NULL)
		    goto Inval;
		if (iocp->ioc_error = bba_Get_Sample_Rate_Type (bo, iocp, mp,
			      MBLK_TO_DT (mp->b_cont, int *)))
		    goto Nack;
		break;

	case BBA_GET_AUDIO_DEV_CAP_TABLE:
		if (mp->b_cont == (mblk_t *)NULL)
		    goto Inval;
		if (iocp->ioc_error = bba_Get_Audio_Dev_Cap_Table (bo, iocp, mp,
			      MBLK_TO_DT (mp->b_cont, struct bba_audio_dev_cap *)))
		    goto Nack;
		break;

	case BBA_GET_AUDIO_DEV_CAP_SIZE:
		if (mp->b_cont == (mblk_t *)NULL)
		    goto Inval;
		if (iocp->ioc_error = bba_Get_Audio_Dev_Cap_Size (bo, iocp, mp,
			      MBLK_TO_DT (mp->b_cont, int *)))
		    goto Nack;
		break;

	case BBA_SET_SAMPLE_NUMBER:
		if (mp->b_cont == (mblk_t *)NULL)
		    goto Inval;
		if (iocp->ioc_error = bba_Set_Sample_Number (bo,
			      MBLK_TO_DT (mp->b_cont, int *)))
		    goto Nack;
		bba_ack(bo, iocp, mp);
		break;

	case BBA_GET_SAMPLE_NUMBER:
		if (mp->b_cont == (mblk_t *)NULL)
		    goto Inval;
		if (iocp->ioc_error = bba_Get_Sample_Number (bo, iocp, mp,
			      MBLK_TO_DT (mp->b_cont, int *)))
		    goto Nack;
		break;

	case BBA_GET_SAMPLE_COUNTS:
		if (mp->b_cont == (mblk_t *)NULL)
		    goto Inval;
		if (iocp->ioc_error = bba_Get_Sample_Counts (bo, iocp, mp,
			      MBLK_TO_DT (mp->b_cont, int *)))
		    goto Nack;
		break;

	case BBA_GET_CURRENT_GAIN: /* What are the current & legitimate gain parameters? */
		if (mp->b_cont == (mblk_t *)NULL)
		    goto Inval;
		if (iocp->ioc_error = bba_Get_Current_Gain (bo, iocp, mp,
			      MBLK_TO_DT (mp->b_cont, struct bba_get_current_gain *)))
		    goto Nack;
		break;

	case ISDNIOC_56KB:
	{
		register struct BBA_open *bc;

		/*printf ("Setting up 56KB conn\n");*/
		if (mp->b_cont == (mblk_t *)NULL)
			goto Inval;
		/*
		printf ("56KB conn chan %d %s\n",
			(MBLK_TO_DT (mp->b_cont, int *)) [1],
			(MBLK_TO_DT (mp->b_cont, int *)) [0] ? "ON" : "OFF");*/
		if ((MBLK_TO_DT (mp->b_cont, int *)) [1] == 1)
			bc = BBA_softc.bs_b1;
		else
			bc = BBA_softc.bs_b2;
		if (bc == (struct BBA_open *)NULL) {
#ifdef BBA_DO_PRINTFS
			printf ("bba_ioc:56KB No connection found\n");
#endif
			iocp->ioc_error = EINVAL;
			goto Nack;
		}
		if (*MBLK_TO_DT (mp->b_cont, int *) == 0)
			bc->bo_flags &= ~BOS_56KB;
		else
			bc->bo_flags |= BOS_56KB;
		break;
	}

#ifdef ISDNIOC_HDLC
	case ISDNIOC_HDLC:
	{
		register struct BBA_open *bc;

		if (mp->b_cont == (mblk_t *)NULL)
			goto Inval;
#ifdef BBA_DO_PRINTFS
		printf ("bbaioc:Hdlc conn chan %d %s\n",
			(MBLK_TO_DT (mp->b_cont, int *)) [1],
			(MBLK_TO_DT (mp->b_cont, int *)) [0] ? "ON" : "OFF");
#endif
		if ((MBLK_TO_DT (mp->b_cont, int *)) [1] == 1)
			bc = BBA_softc.bs_b1;
		else
			bc = BBA_softc.bs_b2;
		if (bc == (struct BBA_open *)NULL) {
#ifdef BBA_DO_PRINTFS
			printf ("bba_ioc:No-HDLC No connection found\n");
#endif
			iocp->ioc_error = EINVAL;
			goto Nack;
		}
		if (*MBLK_TO_DT (mp->b_cont, int *))
			bc->bo_flags &= ~BOS_NOHDLC;
		else
			bc->bo_flags |= BOS_NOHDLC;
		break;
	}
#endif /* ISDNIOC_HDLC */

	case ISDNIOC_SPEAKER_ENABLE:
		if (mp->b_cont == (mblk_t *)NULL)
			goto Inval;
		if (iocp->ioc_error = bba_Speaker_Enable (bo,
				MBLK_TO_DT (mp->b_cont, int *)))
			goto Nack;
		bba_ack(bo, iocp, mp);
		break;

	case ISDNIOC_EAR_ENABLE:
		if (mp->b_cont == (mblk_t *)NULL)
			goto Inval;
		if (iocp->ioc_error = bba_Ear_Enable (bo,
				MBLK_TO_DT (mp->b_cont, int *)))
			goto Nack;
		bba_ack(bo, iocp, mp);
		break;
			
	case ISDNIOC_MIC_ENABLE:
		if (mp->b_cont == (mblk_t *)NULL)
			goto Inval;
		if (iocp->ioc_error = bba_Mic_Enable (bo, 
		      MBLK_TO_DT (mp->b_cont, int *)))
			goto Nack;
		bba_ack(bo, iocp, mp);
		break;

	case ISDNIOC_MIC_INPUT:
		/*
		 * Connect Mic to Codec-0
		 */
		if (!(bo->bo_flags & BOS_HANDSET) || MBLK_ISNULL (mp->b_cont))
			goto Inval;
		if (iocp->ioc_error = bba_hookup_map (bo,
				MBLK_TO_DT (mp->b_cont, int *)))
			goto Nack;
		bba_ack(bo, iocp, mp);
		break;


	case ISDNIOC_SPEAKER_OUTPUT:
		/*
		 * Send data to the speaker..
		 */
		if (!(bo->bo_flags & BOS_SPEAKER1) || MBLK_ISNULL (mp->b_cont))
			goto Inval;
		if (iocp->ioc_error = bba_attach_speaker (bo,
				MBLK_TO_DT (mp->b_cont, int *)))
			goto Nack;
		bba_ack(bo, iocp, mp);
		break;

	case ISDNIOC_VOLUME_CHANGE:
		if (MBLK_ISNULL (mp->b_cont))
			goto Inval;
		if (iocp->ioc_error = bba_Volume_Change (bo,
				MBLK_TO_DT (mp->b_cont, int *)))
			goto Nack;
		bba_ack(bo, iocp, mp);
		break;

	case ISDNIOC_DIAL_TONE:
		if (mp->b_cont == (mblk_t *)NULL)
			goto Inval;
		if (iocp->ioc_error = bba_Dial_Tone (bo,
				MBLK_TO_DT (mp->b_cont, int *)))
			goto Nack;
		break;

	case ISDNIOC_RING_TONE:
		if (mp->b_cont == (mblk_t *)NULL)
			goto Inval;
		if (iocp->ioc_error = bba_Tone_Ringer (bo,
				MBLK_TO_DT (mp->b_cont, int *)))
			goto Nack;
		bba_ack(bo, iocp, mp);
		break;

	case ISDNIOC_DTMF:
		if (mp->b_cont == (mblk_t *)NULL)
			goto Inval;
		if (iocp->ioc_error = bba_DTMF (bo,
				MBLK_TO_DT (mp->b_cont, int *)))
			goto Nack;
		break;
#ifdef ISDNIOC_SNOOP
	case ISDNIOC_SNOOP:
		if (mp->b_cont == (mblk_t *)NULL)
			goto Inval;
#ifdef BBA_DO_PRINTFS
		printf ("bba: Snoop %s\n", *(MBLK_TO_DT (mp->b_cont, int **))
			? "Enabled" : "Disabled");
#endif
		if (*(MBLK_TO_DT (mp->b_cont, int **)))
			BBA_softc.bs_snoop = bo;
		else
			BBA_softc.bs_snoop = (struct BBA_open *)NULL;
		break;
#endif

	default:
		/*
		 * dlld_ioctl returns zero to tell us to signal success
		 * to the ioctl request..
		 */
		if (dlld_ioctl (mp, BBA_softc.bs_dunit, q) == 0)
			break;
		Inval:
		iocp->ioc_error = EINVAL;
		Nack:
		splx (s);
		mp->b_datap->db_type = M_IOCNAK;
		putq (reply, mp);
		return;
	}
	splx (s);
}

/*
 * Put procedure from the user. Process IOCTL requests right away.
 * All M_PROTO and M_DATA are delayed and processed in the service procedure.
 */
static
bbawput (queue_t *q, mblk_t *mp)
{
	switch (MBLK_TYPE (mp)) {

	case M_FLUSH:
		if(*mp->b_rptr & FLUSHW)
			flushq (q, FLUSHALL);
		if(*mp->b_rptr & FLUSHR) {
			*mp->b_rptr &= ~FLUSHW;
			qreply (q, mp);
		}
		break;

	case M_DATA: 
#ifdef notdef
		if ((q->q_first != (mblk_t *)NULL) ||
			(bba_bchan_send (q, mp)))
		{
			putq (q, mp);
		}
		break;
#endif /* notdef */
		/* Fall-thru so that this can be processed by bbawsrv */

	case M_PROTO:  
		putq (q, mp);
		qenable (q);
		break;

	case M_IOCDATA:
	case M_IOCTL:
		bbaioctl (q, mp);
		break;

	default:
		BBA_DEBUG (BBAD_USRERR, ("bba: wput: MSG-type %x dumping\n",
			MBLK_TYPE (mp)));
		(void) freemsg (mp);
		break;
	}
}

/*
 * Service any M_PROTO's that were Q'd up. A non-zero return value from
 * the dlld_mproto indicates blocking.
 */
static
bbawsrv (queue_t *q)
{
	register mblk_t *bp;
	register s;
	
	s = splbba ();
	/*
	 * Data that is being directly written to the speaker
	 * should be held in the queue. The interrupts then
	 * take care of sending the data to the speaker
	 */
	if (q->q_ptr == (char *)BBA_softc.bs_mapout 
		&& BBA_softc.bs_status & BBAS_MAPOUT)
		noenable (q);
	else while ((bp = getq(q)) != (mblk_t *)NULL) {
		if (MBLK_TYPE (bp) == M_PROTO) {
			if (dlld_mproto (bp, BBA_softc.bs_dunit)) {
				/*
				 * This packet cannot be processed yet
				 * the dlld routines will do a q enable when
				 * ready to deal with this.
				 */
				putbq (q, bp);
				noenable (q);
				break;
			}
		} else if (bba_bchan_send (q, bp)) {
			/*
			 * The send routine was probably busy so we just
			 * put it back in the Q and return. This will
			 * be reenabled in the Transmit done routine
			 */
			putbq (q, bp);
			noenable (q);
			break;
		}
	}
	splx (s);
}

/*
 * Receive done. An entire HDLC frame was assembled. Send it on up.
 */
bba_frame_complete (caddr_t arg)
{
	register struct BBA_open *bo;
	register mblk_t *bp;

	bo = (struct BBA_open *)arg;
	ASSERT (bo != (struct Bba_open *)NULL);
	bp = bo->bo_rmblk;
	ASSERT (bp);
	BBA_DEBUG (BBAD_FRAMECOMP,
		   ("bba: frame_complet: chan %d minor %d bits %d bp %lx q %lx",
		    bo->bo_chan, bo - &BBA_softc.bs_open [0],
		    bo->bo_hdlcin.len, bp, bo->bo_rdq));
	bp->b_wptr = bp->b_rptr + (bo->bo_hdlcin.len >> 3);
	MBLK_TYPE (bp) = M_DATA;
	putq (bo->bo_rdq, bp);
	qenable (bo->bo_rdq);
	if ((bo->bo_rmblk = allocb (HDLC_MAX_BUF,BPRI_MED)) == (mblk_t *)NULL){
#ifdef BBA_DO_PRINTFS
		printf ("bba_frame_complete:can't alloc 2K\n");
#endif
		return 0;
	}
	bo->bo_hdlcin.base = bo->bo_rmblk->b_rptr;
	BBA_INCREMENT (bii_frame_comp);
	return 1;
}

/*
 * Write as 1  quantum worth of output to the codec-buffers.
 * Always write to 2 quantums after the current location
 */
void
bba_Voice_Write (queue_t *qp, register u_char *dst)
{
	register struct BBA_open *bo;
	register int wloc, nbytes, bsize, out_word;
	register u_char *src;
	mblk_t *bp, *nbp;

	bo = (struct BBA_open *)qp->q_ptr;
	ASSERT (bo);
	nbytes = out_word = bsize = 0;
	/*
	 * Extract the last saved buffer from the last partial write.
	 */
	BBA_INCREMENT (bii_voice_writes);
	bp = bo->bo_rmblk;
	next_bp:
	BBA_DEBUG (BBAD_SEND, ("bba_Voice_Write:bsize %d nbytes %d bp %lx next bp %lx\n",
			       bsize, nbytes, bp, qp->q_first));
	/*
	 * If the last buffer has been exhausted,
	 * We need the next buffer from the original Q.
	 */
	if ((bp == (mblk_t *)NULL) && ((bp = getq (qp)) == (mblk_t *)NULL)) {
		/*
		 * There is no more buffers from the user
		 * We fill the rest of the quantum with silence.
		 */
		while (nbytes < BBA_DMASIZE) {
			*dst = 0xff;
			dst += 4;
			nbytes++;
		}
		BBA_DEBUG (BBAD_SEND, ("bba_Voice_Write:Filled Silence\n"));
		BBA_INCREMENT (bii_silence);
		goto done;
	}
	/*
	 * Start copying to the dma buffer
	 */
	bsize = bp->b_wptr - bp->b_rptr;
	src = bp->b_rptr;
	while ((--bsize >= 0) && (nbytes < BBA_DMASIZE)) {
		bba_current_sample++;
		bba_play++;
		nbytes++;
		*dst = *src++;
		dst += 4;
		
	}
	bsize++;
	bp_empty:
	if (nbytes != BBA_DMASIZE) {
		nbp = bp;
		bp = bp->b_cont;
		freeb (nbp);
		goto next_bp;
		
	}
	bp->b_rptr = bp->b_wptr - bsize;
	done:
	bo->bo_rmblk = bp;
}

void
bba_txintr (void)
{
	register struct BBA_open *bo;
	register u_char *dst;
	register int x, old_len, tlen, flag;
	register queue_t *qp;
	
	BBA_DEBUG (BBAD_TXINTR, ("bba_txintr: status %x b1 %lx b2 %lx\n",
				 BBA_softc.bs_status, BBA_softc.bs_b1,
				 BBA_softc.bs_b2));
	if (BBA_softc.bs_status & BBAS_MAPOUT) {
		/*
		 * Send out data to the MAP..
		 */
		ASSERT (BBA_softc.bs_mapout);
		bba_Voice_Write (OTHERQ (BBA_softc.bs_mapout->bo_rdq),
				 BBA_softc.bs_xbuf [BBA_softc.bs_xind] + 2);
	}

	if ((BBA_softc.bs_status & (BBAS_B1|BBAS_B1VOICE)) != BBAS_B1)
		goto Tx_B2;
	if ((bo = BBA_softc.bs_b1)  == (struct BBA_open *)NULL)	{
		BBA_DEBUG (BBAD_INTRERR, ("bba_txintr: TX:Missing B1\n"));
		goto Tx_B2;
	}
	dst = BBA_softc.bs_xbuf [BBA_softc.bs_xind];
	flag = bba_hdlc_flag;
	BBA_INCREMENT (bii_b1tx);
		
	if (bo->bo_flags & BOS_SENDING) {
		old_len = BBA_b1_len;
		tlen = MIN (old_len, BBA_DMASIZE);
		for (x = tlen; x > 0; x--) {
			*dst = *BBA_b1_next++;
			dst += 4;
		}
		if ((BBA_b1_len -= tlen) == 0) {
			for (; old_len < BBA_DMASIZE; old_len++) {
				*dst = flag;
				dst += 4;
			}
			bo->bo_flags &= ~BOS_SENDING;
			qp = OTHERQ (bo->bo_rdq);
			if (qp->q_first)
				qenable (qp);
			/**BBA_DEBUG (BBAD_SEND, ("bba: Done Sending B1 len %d\n",
					       bo->bo_hdlcout.len));**/
		}
	} else {
		BBA_INCREMENT (bii_b1flags_sent);
		for (x = 0; x < BBA_DMASIZE; x++) {
			*dst = flag;
			dst += 4;
		}
#ifdef have_q_stopped_bug
		/*
		 * There seems to be some timing problem where
		 * the q seems to have something on it but not
		 * enabled..
		 */
		qp = OTHERQ (bo->bo_rdq);
		if (qp->q_first)
			qenable (qp);
#endif /* have_q_stopped_bug */
	}
	Tx_B2:
	if ((BBA_softc.bs_status & (BBAS_B2|BBAS_B2VOICE)) != BBAS_B2)
		return;
	if ((bo = BBA_softc.bs_b2)  == (struct BBA_open *)NULL)	{
		BBA_DEBUG (BBAD_INTRERR, ("bba_txintr:Missing B2\n"));
		return;
	}
	BBA_INCREMENT (bii_b2tx);
	dst = BBA_softc.bs_xbuf [BBA_softc.bs_xind] + 1;
	flag = bo->bo_hdlcout.flag;
	if (bo->bo_flags & BOS_SENDING) {
		old_len = BBA_b2_len;
		tlen = MIN (old_len, BBA_DMASIZE);
		for (x = tlen; x > 0; x--) {
			*dst = *BBA_b2_next++;
			dst += 4;
		}
		if ((BBA_b2_len -= tlen) == 0) {
			for (; old_len < BBA_DMASIZE; old_len++) {
				*dst = flag;
				dst += 4;
			}
			bo->bo_flags &= ~BOS_SENDING;
			qp = OTHERQ (bo->bo_rdq);
			if (qp->q_first)
				qenable (qp);
			BBA_DEBUG (BBAD_SEND, ("bba: Done Sending B1 len %d\n",
					       bo->bo_hdlcout.len));
		}
	} else {
		for (x = 0; x < BBA_DMASIZE; x++) {
			*dst = flag;
			dst += 4;
		}
#ifdef have_q_stopped_bug
		qp = OTHERQ (bo->bo_rdq);
		if (qp->q_first)
			qenable (qp);
#endif /* have_q_stopped_bug */
	}
}

#ifdef ISDNIOC_SNOOP
/*
 * Copy data from the chan to the snooping user program
 */
void
bba_snoop (src)
	register u_char *src;
{
	register mblk_t *xp;
	register u_char *dst;
	register int len;

	if (xp = allocb (BBA_DMASIZE, BPRI_LO)) {
		dst = (u_char *)xp->b_rptr;
		for (len = 0; len < BBA_DMASIZE; len++)
			dst [len] = src [len << 2];
		xp->b_wptr = xp->b_rptr+BBA_DMASIZE;
		MBLK_TYPE (xp) = M_DATA;
		putq (BBA_softc.bs_snoop->bo_rdq, xp);
	}
}
#endif /* ISDNIOC_SNOOP */

/*
 * Send data to the user directly
 */
void
bba_recv_raw_data (register struct BBA_open *bo,
		   register u_char *src)
{
	register mblk_t *xp;
	register u_char *dst;
	register int len;

	if (xp = allocb (BBA_DMASIZE, BPRI_LO)) {
		dst = (u_char *)xp->b_rptr;
		for (len = 0; len < BBA_DMASIZE; len++)
			dst [len] = src [len << 2];
		xp->b_wptr = xp->b_rptr+BBA_DMASIZE;
		MBLK_TYPE (xp) = M_DATA;
		putq (bo->bo_rdq, xp);
	}
}

void
bba_rxintr (void)
{
	register struct BBA_open *bo;
	register void (*rfunc) ();
	extern void hdlc_recv_bba_56KB (), hdlc_recv_bba ();

	/*
	 * Every 4th byte contains data from the chan.
	 * Off-0 = BD (on 79c30 see page 21)
	 * Off-1 = BE connected to B2
	 * Off-2 = BF connected to B1
	 * Off-3 = Wasted
	 *
	 * On non-voice oriented connections we pass the
	 * data directly to the HDLC interface. This
	 * Interface will move it to the right mblk that
	 * was allocated for it. The problem with this is
	 * that all this is being done at interrupt time.
	 * What would be ideal would be to have software
	 * interrupts dealing with incomming data..
	 */
	BBA_DEBUG (BBAD_RXINTR, ("bba:rxintr: status %x b1 %lx b2 %lx\n",
				 BBA_softc.bs_status, BBA_softc.bs_b1,
				 BBA_softc.bs_b2));
	if (BBA_softc.bs_status & BBAS_B1VOICE) {
		/*
		 * Voice connected to B1 don't route this thru HDLC
		 */
	} else if (BBA_softc.bs_status & BBAS_B1) {
		/*
		 * This is a data connection.
		 */
		if ((bo = BBA_softc.bs_b1)  == (struct BBA_open *)NULL)	{
			BBA_DEBUG (BBAD_INTRERR, ("bba_rxintr: Missing B1\n"));
			goto Next_Chan;
		}
		BBA_INCREMENT (bii_b1rx);
		if (bo->bo_flags & BOS_NOHDLC) {
			bba_recv_raw_data (bo, BBA_softc.bs_rbuf 
				      [BBA_softc.bs_rind]);
			goto Next_Chan;
		}
		if (bo->bo_rmblk == (mblk_t *)NULL) {
			if (bo->bo_rmblk = allocb (HDLC_MAX_BUF, BPRI_LO))
				bo->bo_hdlcin.base = bo->bo_rmblk->b_rptr;
			else {
				BBA_DEBUG (BBAD_INTRERR, ("bba_rxintr:Cant allocb\n"));
				goto Next_Chan;
			}
		}
#ifdef ISDNIOC_SNOOP
		if (BBA_softc.bs_snoop)
			bba_snoop (BBA_softc.bs_rbuf [BBA_softc.bs_rind]);
#endif
		if (bo->bo_flags & BOS_56KB)
			rfunc = hdlc_recv_bba_56KB;
		else
			rfunc = hdlc_recv_bba;
		(*rfunc) (&bo->bo_hdlcin, BBA_softc.bs_rbuf
			       [BBA_softc.bs_rind],
			       BBA_DMASIZE, bba_frame_complete, bo);
	}
	Next_Chan:
	if (BBA_softc.bs_status & BBAS_B2VOICE) {
		/*
		 * Voice connected to B2 No hdlc here.
		 */
	} else if (BBA_softc.bs_status & BBAS_B2) {
		if ((bo = BBA_softc.bs_b2) == (struct BBA_open *)NULL) {
			BBA_DEBUG (BBAD_INTRERR, ("bba_rxintr: Missing B2\n"));
			return;
		}
		BBA_INCREMENT (bii_b2rx);
		if (bo->bo_flags & BOS_NOHDLC) {
			bba_recv_raw_data (bo, BBA_softc.bs_rbuf 
				      [BBA_softc.bs_rind] + 1);
			goto Map_Chan;
		}
		if (bo->bo_rmblk == (mblk_t *)NULL) {
			if (bo->bo_rmblk = allocb (HDLC_MAX_BUF, BPRI_LO))
				bo->bo_hdlcin.base = bo->bo_rmblk->b_rptr;
			else {
				BBA_DEBUG (BBAD_INTRERR, ("bba_rxintr:Cant allocb (B2)\n"));
				goto Map_Chan;
			}
		}
#ifdef ISDNIOC_SNOOP
		if (BBA_softc.bs_snoop)
			bba_snoop (BBA_softc.bs_rbuf [BBA_softc.bs_rind] + 1);
#endif
		if (bo->bo_flags & BOS_56KB)
			rfunc = hdlc_recv_bba_56KB;
		else
			rfunc = hdlc_recv_bba;
		(*rfunc) (&bo->bo_hdlcin, BBA_softc.bs_rbuf
				       [BBA_softc.bs_rind] + 1,
				       BBA_DMASIZE, bba_frame_complete, bo);
	}
	Map_Chan:
	/*
	 * Map is connected to Bd.
	 */
	if (BBA_softc.bs_status & BBAS_HANDSET) {
		register u_char *dst, *src;
		register x;
		mblk_t *bp;

		if ((bo = BBA_softc.bs_mapin) == (struct BBA_open *)NULL)
			goto Out;
		if (!(bo->bo_flags & BOS_HANDSET))
			goto Out;
		if ((bp = allocb (BBA_DMASIZE, BPRI_MED)) == (mblk_t *)NULL) {
#ifdef BBA_DO_PRINTFS
			printf ("bba:Map_in:Can't alloc %d\n", BBA_DMASIZE);
#endif
			return;
		}
		BBA_DEBUG (BBAD_STRALLOC, 
			    ("bba:Map_In:bp %lx %d\n", bp, BBA_DMASIZE));
		dst = (u_char *)bp->b_rptr;
		src = (u_char *)BBA_softc.bs_rbuf [BBA_softc.bs_rind] + 2;
		for (x = 0; x < BBA_DMASIZE; src+= 4) {
			bba_record++;
			dst [x++] = *src;
		}
		bp->b_wptr = bp->b_rptr + BBA_DMASIZE;
		MBLK_TYPE (bp) = M_DATA;
		putq (bo->bo_rdq, bp);
	}
	Out:;
}

/*
 * BBA device  interrupt service routine.
 */
/* ARGSUSED */
bbaintr (ctlr)
    int ctlr;
{
	static u_long bba_last_mer = 0, bba_last_sir = 0;
	register unsigned int ireg, sir;
	
#if defined(__alpha)
	mb();
	sir = bba_read_sir () & ISDN_INTR;
	bba_write_sir (~sir);
#elif defined(mips)
	sir = *(volatile u_long *)PHYS_TO_K1(KN02CA_SIR_ADDR) & BBA_INTR;
	*(volatile unsigned long *)PHYS_TO_K1(KN02CA_SIR_ADDR) = ~sir;
#else
	xxx
#endif
	BBA_DEBUG (BBAD_INTR, ("bba: intr: sir %lx\n", sir));
	BBA_INCREMENT (bii_nintr);
	if (BBA_softc.bs_nopens == 0) {
		BBA_INCREMENT (bii_notup);
		ireg = bba_codec_read (0, IR_OFFS); /* Read IR */
		return;
	}
	/*
	 * Read the System Interrupt register and create the reset mask
	 * We should only look at bits that are not masked, but for now, take
	 * the safe approach and handle them all
	 * Reset the interrupts so that we don't recalled again this cycle
	 */
	if (ISDN_INT & sir) {
		BBA_INCREMENT (bii_isdn);
		ireg = bba_codec_read (0, IR_OFFS); /* Read IR */
		BBA_DEBUG (BBAD_DSCINTR, ("bba: dscintr %x\n", ireg));
		if (BBA_softc.bs_status & BBAS_DLLDINIT)
			ISDN_dscisr (BBA_softc.bs_dunit, ireg);
#ifdef BBA_DO_PRINTFS
		else
			printf ("bba:DCHAN intr %x DLLD not up\n", ireg);
#endif
	}
	if (sir & ISDN_DMA_ERROR) {
		BBA_INCREMENT (bii_dmaerr);
#if defined(mips)
		if (bba_last_mer == 0) {
			bba_last_mer = *(u_long *)PHYS_TO_K1 (0x0C400000);
			bba_last_sir = sir;
		}
#endif /* mips */
	}
	
	if (sir & ISDN_DMA_RXINT) {
		BBA_INCREMENT (bii_dmarx);
		/*
		 * update the next DMA address.
		 */
#if defined(mips)
		*(volatile unsigned long *)PHYS_TO_K1 (DMA_NEXT_RCV) =
			BBA_softc.bs_rcv_dma_addr [BBA_softc.bs_rind];
#elif defined(__alpha)
		*(volatile u_int *)DMA_NEXT_RCV = BBA_softc.bs_rcv_dma_addr 
						[BBA_softc.bs_rind]; mb ();
#endif
		bba_rxintr ();
		BBA_softc.bs_rind = (BBA_softc.bs_rind + 1) & 1;
	}
	if (sir & ISDN_DMA_TXINT) {
		/*
		 * Transmit done..Fill all the buffers with the next octet
		 * meant for each port. VOICE (Handset) data that is tied in
		 * with S interface are directly tied to the MAP via MUX
		 */
		BBA_INCREMENT (bii_dmatx);
		/*
		 * Set up the next transmit DMA buffer.
		 */
#if defined(mips)
		*(volatile unsigned long *)PHYS_TO_K1 (DMA_NEXT_XMT) =
			BBA_softc.bs_xmt_dma_addr [BBA_softc.bs_xind];
#elif defined(__alpha)
		*(volatile u_int *)DMA_NEXT_XMT = BBA_softc.bs_xmt_dma_addr 
			[BBA_softc.bs_xind]; mb ();
#endif
		bba_txintr ();
#if defined (__alpha)
		mb ();
#endif /* __alpha */
		BBA_softc.bs_xind = (BBA_softc.bs_xind + 1) & 1;
	}
}

/*
 * Send data on a b-chan queue
 * Copy data to the SRAM map. Set transfer size
 * A return of non-zero signifies to the caller to stop his queues.
 */
bba_bchan_send (queue_t *qp, mblk_t *bp)
{
	static int bba_custom = 0;
	register struct BBA_open *bo;
	register mblk_t *next;
	register int word, len;
	register struct hdlc_info *hi;

	bo = (struct BBA_open *)qp->q_ptr;
	ASSERT (bo != (struct BBA_open *)NULL);
	BBA_DEBUG (BBAD_SEND, ("bba: bchan_send: chan %d %d bytes ",
			       bo->bo_chan, msgdsize (bp)));

	if (!(bo->bo_flags & BOS_ENABLED)) {
#ifdef BBA_DO_PRINTFS
		printf ("B-Chan send before enable\n");
#endif
		freemsg (bp);
		return 0;
	}
	if (bo->bo_flags & BOS_SENDING) {
		BBA_INCREMENT (bii_send_busy);
		BBA_DEBUG (BBAD_SEND, ("->Busy\n"));
		return 1;
	}
	/*
	 * At this point we are OK to send out data.
	 */
	hi = &bo->bo_hdlcout;
	if (bo->bo_flags & BOS_NOHDLC) {
		for (next = bp; next != (mblk_t *)NULL; next = next->b_cont)
			bcopy (next->b_rptr, hi->base, MBLK_BLEN (next));
	} else {
		*hi->base = 0;
                hi->base[1] = 0;   /* fix in the bit-stuffed 2nd octet */ 
		hi->len = 0;
#ifdef bba_custom_debug
		if (bba_custom & 2) {
			u_char *p = bp->b_rptr;
#ifdef BBA_DO_PRINTFS
			printf ("bba:Encoding:[%02x%02x%02x%02x][%02x%02x%02x%02x]\n",
			p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
#endif
		}
#endif /* bba_custom_debug */
		for (next = bp; next != (mblk_t *)NULL; next = next->b_cont) {
			if (bo->bo_flags & BOS_56KB)
				hdlc_encode_bba_56KB (hi, next->b_rptr, 
						      MBLK_BLEN (next),
						      next == bp/* add-head */,
						      (int)next->b_cont/* add-trailer */);
			else
				hdlc_encode_bba (hi, next->b_rptr,
						 MBLK_BLEN (next),
						 next == bp/* add-head */,
						 (int)next->b_cont/* add-trailer */);
		}
#ifdef bba_custom_debug
		if (bba_custom & 2) {
			u_char *p = hi->base;
#ifdef BBA_DO_PRINTFS
			printf ("bba:Result:[%02x%02x%02x%02x][%02x%02x%02x%02x]\n",
			p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
#endif
		}
#endif /* bba_custom_debug */
	}
	freemsg (bp);
	BBA_INCREMENT (bii_send_frame);
	if (bo->bo_chan == MUX_PORT_B1) {
		BBA_b1_next = hi->base;
		BBA_b1_len = hi->len >> 3;
	} else {
		BBA_b2_next = hi->base;
		BBA_b2_len = hi->len >> 3;
	}
	if (bba_custom & 1) {
		mblk_t *xp;
		register int len;

		len = hi->len >> 3;
		if (xp = allocb (len, BPRI_LO)) {
			bcopy (hi->base, xp->b_rptr, len);
			xp->b_wptr = xp->b_rptr+len;
			MBLK_TYPE (xp) = M_DATA;
			putq (bo->bo_rdq, xp);
		}
			
	}
	BBA_DEBUG (BBAD_SEND, (" Sending %d\n", hi->len >> 3));
	bo->bo_flags |= BOS_SENDING;
	return 0;
}

/*
 * Enable of disable a B-channell.  chan is B1 or B2.
 * enable = 1 for enable 0 for disable
 * We are going to tie in HDLC-0 with B1 and HDLC-1 with B2
 */
bba_bchan_enable (queue_t *qp, int chan, int enable, int handset)
{
	register struct BBA_open *bo, *openp;
	register int x, intr = 0, oval;
	register u_char *out_base;
	
	bo = (struct BBA_open *)qp->q_ptr;
	ASSERT (bo != (struct BBA_open *)NULL);
	/*
	 * Scan through the all the open ones and make sure that
	 * nobody else has this B1 or B2 open
	 */
	BBA_DEBUG (BBAD_ENABLE,
		("bba: bchan_enable:chan %d minr %d call-typ %s (%s)\n",
			chan, bo - &BBA_softc.bs_open [0],
		 	(handset ? "Voice":"Data"),
			(enable ? "Enable" : "Disable")));
	/*
	 * Don't pay any attention to the chan or handset in the
	 * disable messages. Use the values that we beleive that they are.
	 */
	if (!enable) {
		for (x = 0; x < BBA_MAXMINOR; x++) {
			openp = &BBA_softc.bs_open [x];
			if (openp->bo_chan == chan) {
				bo = openp;
				break;
			}
		}
		handset = bo->bo_flags & BOS_VOICE;
		BBA_DEBUG (BBAD_ENABLE, ("bba:bchan_enabl:Changed to minr %d\n",
			bo - &BBA_softc.bs_open [0]));
	}
	/*
	 * On the voice chan connect Ba to B1 or B2
	 * We don't need to deal with anything else..
	 */
	if (handset) {
		if (enable) {
			bo->bo_flags |= BOS_VOICE|BOS_ENABLED;

			if (chan == MUX_PORT_B1) {
				BBA_softc.bs_status |= BBAS_B1VOICE;
				BBA_softc.bs_b1 = bo;
			} else {
				BBA_softc.bs_status |= BBAS_B2VOICE;
				BBA_softc.bs_b2 = bo;
			}
		} else {
			if (chan == MUX_PORT_B1) {
				BBA_softc.bs_status &= ~BBAS_B1VOICE;
				BBA_softc.bs_b1 = (struct BBA_open *)NULL;
			} else {
				BBA_softc.bs_status &= ~BBAS_B2VOICE;
				BBA_softc.bs_b2 = (struct BBA_open *)NULL;
			}
			bo->bo_flags &= ~BOS_ENABLED|BOS_VOICE;
		}
		return MUX_PORT_BA;
	}
	/*
	 * These are data connection..
	 */
	if (enable) {
		for (x = 0; x < BBA_MAXMINOR; x++) {
			openp = &BBA_softc.bs_open [x];
			if (openp->bo_rdq == (queue_t *)NULL)
				continue;
			if ((openp->bo_flags & BOS_ENABLED) == 0)
				continue;
			if (openp->bo_chan == chan) {
#ifdef BBA_DO_PRINTFS
				printf("Bba:dev %d alrdy attchd to %d chan\n",
					x, chan);
#endif
				return -1;
			}
		}
		bo->bo_chan = chan;
		bo->bo_flags |= BOS_ENABLED;
		if (chan == MUX_PORT_B1) {
			BBA_softc.bs_status |= BBAS_B1;
			BBA_softc.bs_b1 = bo;
			out_base = &BBA_b1_outdata [0];
		} else {
			BBA_softc.bs_status |= BBAS_B2;
			BBA_softc.bs_b2 = bo;
			out_base = &BBA_b2_outdata [0];
		}
		/*
		 * Set up the in/out hdlc state machines
		 */
		bo->bo_hdlcout.base = (u_char *)out_base;
		*bo->bo_hdlcout.base = 0;
		hdlc_reset_bba (&bo->bo_hdlcin);
		hdlc_reset_bba (&bo->bo_hdlcout);
		bo->bo_hdlcout.flag = 0x7E; /* Start with HDLC-Flags */
		bba_enable_dma ();
	} else {
		/*
		 * Disable a specific B chan, scan through the list and see if
		 * we need to disable the codec interrupt.
		 */
		bo->bo_flags &= ~BOS_ENABLED|BOS_SENDING;
		if (chan == MUX_PORT_B1) {
			BBA_softc.bs_status &= ~BBAS_B1;
			BBA_softc.bs_b1 = (struct BBA_open *)NULL;
		} else {
			BBA_softc.bs_status &= ~BBAS_B2;
			BBA_softc.bs_b2 = (struct BBA_open *)NULL;
		}
		flushq (bo->bo_rdq, FLUSHALL);
		flushq (OTHERQ (bo->bo_rdq), FLUSHALL);
	}
	return chan == MUX_PORT_B1 ? MUX_PORT_BF : MUX_PORT_BE;
}

bba_disable_voice (register struct BBA_open *bo)
{
	unsigned char data;

	data = 0;
	bba_codecindirect (MUX_MCR3, 1, &data, BBA_CODEC_WRITE);
	BBA_softc.bs_status &= ~(BBAS_B1VOICE|BBAS_B2VOICE);
	BBA_DEBUG (BBAD_ENABLE, ("bba:voice MCR3 disabled\n"));
}

/*
 * Put the messages to the upper module (This is enabled because of
 * the putq in the routine above)..
 */
void
bbarsrv (queue_t *qp)
{
	register mblk_t *bp;

	while ((bp = getq (qp)) != (mblk_t *)NULL) {
		BBA_DEBUG (BBAD_Q, ("bbarsrv: putting to %x bp %lx len %d\n",
			 qp->q_next, bp, MBLK_BLEN (bp)));
                putnext (qp, bp);
	}
}

/*
 * We just stick it in the queue because we don't want to call
 * the upper module at interrupt priority.
 */
void
bba_put_dchan_rdq (int dunit, mblk_t *bp)
{
	register queue_t *qp;

	if ((qp = BBA_softc.bs_open [BBA_DCHAN].bo_rdq) == (queue_t *)NULL) {
#ifdef BBA_DO_PRINTFS
		printf ("bba_put_dchan:No Dchan Open\n");
#endif
		return;
	}
	BBA_DEBUG (BBAD_Q, ("bba: put dchan qp %lx\n", qp));
	putq (qp, bp);
	qenable (qp);
}

/*
 * This is normally done after a Q was disabled due to XMIT busy situation..
 */
void
bba_qenable (int dunit)
{
	register queue_t *qp;

	qp = BBA_softc.bs_open [BBA_DCHAN].bo_rdq;
	BBA_DEBUG (BBAD_Q, ("bba: qp %lx enabled\n", qp));
	if (qp != (queue_t *)NULL)
		qenable (OTHERQ (qp));
}

struct driver bbadriver = {
	bbaprobe,	/* See if the driver is there         */
	(int (*)())0,	/* slave-see if slave is there        */
	bbaattach,	/* controller-attach                  */
	(int (*)())0,	/* device- att                        */
	(int (*)())0,	/* fill csr/ba to start transfer (go) */
	bba_csr,	/* device-csr address                 */
	"bba",		/* Device Name                        */
	bba_devinfo,	/* Device List back pointers          */
	"bba",		/* Controller Name                    */
	&bba_ctlr,	/* back pointers                      */
};

static struct module_info strbba =  {
	0x62626130,	/* bba0 (in ascii)	*/
	"bba", 0, INFPSZ, 32<<10 /* 32k */, 128
};

static struct qinit rinit = {
	(int (*)())NULL, (int (*)())bbarsrv,
	bbaopen, bbaclose, NULL, &strbba
};

static struct qinit winit = {
	bbawput, bbawsrv, (int (*)())NULL, (int (*)())NULL, 
	NULL, &strbba
};

struct DRV_funcs bba_drv_funcs = {
	(void (*)())bba_codec_write,
	bba_codec_read,
	bba_qenable,
	bba_put_dchan_rdq,
	bba_bchan_enable
};

struct streamtab bbatab = { &rinit, &winit};
int _bba_debug = 0
/*	|BBAD_FRAMECOMP*/
/*	|BBAD_RCVDONE*/
/*	|BBAD_INTR*/
/*	|BBAD_DSCINTR*/
/*	|BBAD_XMITDONE	*/
/*	|BBAD_ENABLE	*/
/*	|BBAD_SEND	*/
/*	|BBAD_OPEN	*/
/*	|BBAD_CONF	*/
/*	|BBAD_TXINTR	*/
/*	|BBAD_RXINTR	*/
/*	|BBAD_USRERR	*/
/*	|BBAD_Q		*/
/*	|BBAD_CODEC	*/
/*	|BBAD_IOC	*/
/*	|BBAD_PUMP	*/
/*	|BBAD_SPEAKER	*/
;

