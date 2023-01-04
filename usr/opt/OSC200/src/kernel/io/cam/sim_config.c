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
static char *rcsid = "@(#)$RCSfile: sim_config.c,v $ $Revision: 1.1.21.4 $ (DEC) $Date: 1993/09/22 15:06:08 $";
#endif

/************************************************************************
 *
 * File:	sim_config.c
 * Date:	June 17, 1991
 * Author:	Robert P. Scott
 *
 * Description:
 *	CAM peripheral driver configuration information.
 *
 * Modification History:
 *
 *	91/07/03	rps	Changed TC option names to match std.
 *
 *	91/07/20	rps	Added spurious interrupt handling code.
 *
 * 	91/09/09	rps	Added temp. MAXINE/BIGMAX support.
 *
 *	91/09/26	rps	Needed check for turbo channel exist.
 *
 *	91/10/22	janet
 *	o Removed "#ifndef GENERIC" ifdefs.
 *	o Modified sim_enable_interrupts to take SIM_SOFTC pointer
 *	  as an argument.  Added "#ifdef (DS5000 | DS5000_100)" around
 *	  call to tc_enable_option().
 *
 *	91/10/24	janet
 *	Added Mipsmate support.
 *
 *	91/10/25	janet
 *	Added default configuration functins: hba_default_probe(),
 *	hba_default_attach(), hba_default_chip_reset(), dme_default_attach(),
 *	dme_default_unload().
 *
 *	91/11/15	jag
 *	Made the changes from LARRY to the LR specific defines.
 */

#include <io/common/iotypes.h>
#include <sys/types.h>			/* system level types */
#include <sys/param.h>			/* system level parameter defs */
#include <sys/buf.h>
#include <sys/time.h>
#include <kern/lock.h>
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_all.h>
#include <hal/cpuconf.h>
#include <io/dec/tc/tc.h>
#include <io/cam/cam_config.h>
#include <io/cam/sim_config.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/sim.h>
#include <io/cam/sim_common.h>
#include <io/cam/sim_94.h>
#include <io/cam/sim_sii.h>
#include <io/cam/scsi_status.h>
#ifdef __alpha
#include <io/cam/dme_tcds_94_dma.h>
#else
#include <io/cam/dme_3min_94_dma.h>
#endif
#include <io/common/devdriver.h>

#include "asc.h"
#include "sii.h"
#include "tza.h"

#ifdef __alpha
#include "siop.h"
#include "aha.h"
#include "skz.h"
#else
#include "kzq.h"
#endif

/* ---------------------------------------------------------------------- */
/* Local defines.
 */

#ifdef mips
void sii_ds3100_spurious_interrupt( int );
#endif /* mips */

extern int cpu;
extern SIM_SOFTC *softc_directory[];

#ifdef DS5000_300	/* 3max+ */
    extern dma94_kn03_dme_attach();
#endif

#if NASC > 0
extern sim94_probe(), sim94_attach(), sim94_unload();
extern sim_kn02_chip_reset(), sim_kn02ba_chip_reset();
extern sim_fast_chip_reset(), simfast_attach();

extern ram94_dme_attach(), ram94_dme_unload();
extern dma94_dme_attach(), dma94_dme_unload();
extern tcds_dme_attach(), tcds_dme_unload();
#endif

#if NSII > 0
extern simsii_probe(), simsii_attach(), simsii_unload();
extern sim_kn01_chip_reset();

extern ds3100_dme_attach(), ds3100_dme_unload();
extern ds5100_dme_attach(), ds5100_dme_unload();
#endif

#if NTZA > 0
extern kztsa_probe(), kztsa_attach(), kztsa_unload();
extern kztsa_adap_reset();
extern kztsa_dme_attach(), kztsa_dme_unload();
#endif
                                 
#ifdef __alpha
#if NSIOP > 0
extern siop_probe(), siop_kn430_attach(), siop_unload(), siop_chip_reset();
#endif	/* NSIOP > 0 */

#if NAHA > 0
extern aha_probe(), aha_attach(), aha_unload(), aha_chip_reset();
#endif  /* NAHA > 0 */

#if NSKZ > 0
extern skz_probe(), skz_attach(), skz_reset_attach(), skz_unload();
#endif /* NSKZ > 0 */

extern dme_null_attach(), dme_null_null();

#else /* __alpha */
#if NKZQ > 0
extern simkzq_probe(), simkzq_slave(), simkzq_attach(), simkzq_unload();
extern sim_kzq_chip_reset();

extern ds5500_kzq_dme_attach(), ds5500_kzq_dme_unload();
extern ds5500_kzq_dme_attach(), ds5500_kzq_dme_unload();
#endif /* NKZQ */
#endif /* __alpha */

/* Default config functions */
int hba_default_probe(), hba_default_attach(), hba_default_chip_reset(),
    dme_default_attach(), dme_default_unload();


int cam_hba_limit = INIT_CAM_HBA_LIMIT;
int cam_dme_limit = INIT_CAM_DME_LIMIT;

CAM_HBA_LIST_ENTRY           cam_hba_list[INIT_CAM_HBA_LIMIT+1];
CAM_DME_LIST_ENTRY           cam_dme_list[INIT_CAM_DME_LIMIT+1];

int cam_hba_entries = 0;
int cam_dme_entries = 0;

static int config_init = 0;        /* remove this or do it right */

void
init_sim_components( void )
{
    int i;

    if ( config_init != 4192 )           /* init these structs on first */
      {                                  /* time through only */
	for( i=0; i<cam_hba_limit; i++ )
	  {
	    cam_hba_list[i].cs_name = (char *) NULL;
	    cam_hba_list[i].cs_probe = NULL;
	    cam_hba_list[i].cs_unload = NULL;
	  }
	cam_hba_entries = 0;
	
	for( i=0; i<cam_dme_limit; i++ )
	  {
	    cam_dme_list[i].dme_name = (char *) NULL;
	    cam_dme_list[i].dme_init = NULL;
	    cam_dme_list[i].dme_unload = NULL;
	  }
	cam_dme_entries = 0;
	
#if NASC > 0
    add_cam_hba_entry( "asc", sim94_probe, sim94_attach, sim_kn02_chip_reset, 
		      sim94_unload );
    add_cam_hba_entry( "PMAZ-AA ", sim94_probe, sim94_attach, 
		      sim_kn02_chip_reset, sim94_unload );
    add_cam_hba_entry( "PMAZ-BA ", sim94_probe, sim94_attach, 
		      sim_kn02ba_chip_reset, sim94_unload );
    add_cam_hba_entry( "PMAZ-DS ", sim94_probe, sim94_attach, 
		      sim_kn02ba_chip_reset, sim94_unload );
    add_cam_hba_entry( "PMAZ-FS ", sim94_probe, simfast_attach, 
		      sim_fast_chip_reset, sim94_unload );
    add_cam_hba_entry( "PMAZB-AA", sim94_probe, sim94_attach, 
		      sim_kn02ba_chip_reset, sim94_unload );
    add_cam_hba_entry( "PMAZB-AB", sim94_probe, sim94_attach, 
		      sim_kn02ba_chip_reset, sim94_unload );
    add_cam_hba_entry( "PMAZC-AA", sim94_probe, simfast_attach, 
		      sim_fast_chip_reset, sim94_unload );
#ifdef DS5000_300	/* 3max+ */
    add_cam_hba_entry( "PMAZ-CA ", sim94_probe, sim94_attach, 
		      sim_kn02ba_chip_reset, sim94_unload );
#endif /* DS5000_300 */

    add_cam_dme_entry( "ram94", ram94_dme_attach, ram94_dme_unload );
    add_cam_dme_entry( "PMAZ-DS ", tcds_dme_attach, tcds_dme_unload );
    add_cam_dme_entry( "PMAZ-FS ", tcds_dme_attach, tcds_dme_unload );
    add_cam_dme_entry( "PMAZB-AA", tcds_dme_attach, tcds_dme_unload );
    add_cam_dme_entry( "PMAZB-AB", tcds_dme_attach, tcds_dme_unload );
    add_cam_dme_entry( "PMAZC-AA", tcds_dme_attach, tcds_dme_unload );
    add_cam_dme_entry( "PMAZ-AA ", ram94_dme_attach, ram94_dme_unload );
    add_cam_dme_entry( "PMAZ-BA ", dma94_dme_attach, dma94_dme_unload );

#ifdef DS5000_300	/* 3max+ */
    add_cam_dme_entry("PMAZ-CA ", dma94_kn03_dme_attach, dma94_dme_unload);
#endif /* DS5000_300 */
#endif /* NASC */

#if NSII > 0
    add_cam_hba_entry( "sii", simsii_probe, simsii_attach, sim_kn01_chip_reset,
	simsii_unload );
    add_cam_dme_entry( "siiram", ds3100_dme_attach, ds3100_dme_unload );
    add_cam_dme_entry( "sii_kn230", ds5100_dme_attach, ds5100_dme_unload );
#endif /* NSII */

#if NTZA > 0
    add_cam_hba_entry( "KZTSA-AA", kztsa_probe, kztsa_attach,
                      kztsa_adap_reset, kztsa_unload );
    add_cam_dme_entry( "KZTSA-AA", kztsa_dme_attach, kztsa_dme_unload );
#endif /* NTZA */
                       
#ifdef __alpha
#if NSIOP > 0
    add_cam_hba_entry("siopco",siop_probe,siop_kn430_attach,siop_chip_reset,siop_unload);
#endif /* NSIOP > 0 */

#if NAHA > 0
    add_cam_hba_entry("aha",aha_probe,aha_attach,aha_chip_reset,aha_unload);
#endif /* NAHA > 0 */

#if NSKZ > 0
    add_cam_hba_entry ( "skz", skz_probe, skz_attach,
			skz_reset_attach, skz_unload );
#endif /* NSKZ > 0 */

    add_cam_dme_entry("null",dme_null_attach,dme_null_null);

#else /* __alpha */
#if NKZQ > 0
    add_cam_hba_entry( "kzq", simkzq_probe, simkzq_attach, sim_kzq_chip_reset,
	simkzq_unload );
    add_cam_dme_entry( "kzqram", ds5500_kzq_dme_attach, ds5500_kzq_dme_unload );
#endif /* NKZQ */
#endif /* __alpha */

    /*
     * Default entries.
     */
    add_cam_hba_entry( "hba_default", hba_default_probe, hba_default_attach,
		       hba_default_chip_reset);
    add_cam_dme_entry( "dme_default", dme_default_attach, dme_default_unload);
    

      }

    config_init = 4192;              /* so we don't go through again */
}

int
add_cam_hba_entry( char *name, int (*probe)(), int (*attach)(),
		  int (*reset_attach)(), int (*unload)() )
{
    int i;

    if ( cam_hba_entries == cam_hba_limit )
      {
        PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT, 
	       ("[b/t/l] (add_cam_hba_entry) Count = %d, cannot yet expand list.\n" ));
	return CAM_REQ_CMP_ERR;
      }

    for( i=0; i<cam_hba_entries && cam_hba_list[i].cs_name &&
	strcmp( name, cam_hba_list[i].cs_name );
	i++ )
      {}

    if ( i < cam_hba_entries )   /* we didn't hit the end of the list? */
      {
	PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT, 
	       ("[b/t/l] (add_cam_hba_entry) duplicate entry '%s'\n", name ));
	return CAM_REQ_CMP_ERR;
      }

    cam_hba_list[cam_hba_entries].cs_name = name;
    cam_hba_list[cam_hba_entries].cs_probe = probe;
    cam_hba_list[cam_hba_entries].cs_attach = attach;
    cam_hba_list[cam_hba_entries].cs_reset_attach = reset_attach;
    cam_hba_list[cam_hba_entries].cs_unload = unload;

    cam_hba_entries++;
    return CAM_REQ_CMP;
}

int 
remove_cam_hba_entry( char *name )
{
    int i;

    for( i=0; i<cam_hba_entries && cam_hba_list[i].cs_name && 
	strcmp( name, cam_hba_list[i].cs_name );
	i++ )
      {}

    if ( i == cam_hba_entries )          /* we hit the end of the list? */
        return CAM_REQ_CMP_ERR;

    while( i < cam_hba_entries )
      {
	cam_hba_list[i].cs_name = 
            cam_hba_list[i+1].cs_name;
	cam_hba_list[i].cs_probe = 
            cam_hba_list[i+1].cs_probe;
	cam_hba_list[i].cs_unload = 
            cam_hba_list[i+1].cs_unload;
	
	i++;
      }

    cam_hba_list[i].cs_name = (char *) NULL;
    cam_hba_list[i].cs_probe = NULL;
    cam_hba_list[i].cs_unload = NULL;

    cam_hba_entries--;
    return CAM_REQ_CMP;
}

int
name_lookup_hba_probe(name, ba, prb)
char *name;
char *ba;
struct controller *prb;
{
  int i;

  for( i=0; i<cam_hba_entries && cam_hba_list[i].cs_name &&
      strcmp( name, cam_hba_list[i].cs_name );
      i++ )
    {}

  if ( i == cam_hba_entries )
    {
      PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	     ("[b/t/l] (name_lookup_hba_probe) name '%s' not found.\n", name ) );
      return CAM_REQ_CMP_ERR;
    }

  return cam_hba_list[i].cs_probe( ba, prb );
}

int
name_lookup_hba_attach( char *name, SIM_SOFTC *sc )
{
  int i;

    PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	   ("[b/t/l] (name_lookup_hba_attach) entry - name='%s', sc=0x%x \n", 
	    name, sc ) );

  for( i=0; i<cam_hba_entries && cam_hba_list[i].cs_name &&
      strcmp( name, cam_hba_list[i].cs_name );
      i++ )
    {}

  if ( i == cam_hba_entries )
    {
      PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	     ("[b/t/l] (name_lookup_hba_attach) name '%s' not found.\n", name ) );
      return CAM_REQ_CMP_ERR;
    }

  return cam_hba_list[i].cs_attach( sc );
}

unsigned long
name_lookup_hba_reset_attach( char *name, SIM_SOFTC *sc )
{
  int i;

    PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	   ("[b/t/l] (name_lookup_hba_reset_attach) entry - name='%s', sc=0x%x \n", 
	    name, sc ) );

  for( i=0; i<cam_hba_entries && cam_hba_list[i].cs_name &&
      strcmp( name, cam_hba_list[i].cs_name );
      i++ )
    {}

  if ( i == cam_hba_entries )
    {
      PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	     ("[b/t/l] (name_lookup_hba_reset_attach) name '%s' not found.\n", name ) );
      return CAM_REQ_CMP_ERR;
    }

  return (unsigned long) cam_hba_list[i].cs_reset_attach;
}


int
add_cam_dme_entry( char *name, int (*init)(), int (*unload)() )
{
    int i;

    if ( cam_dme_entries == cam_dme_limit )
      {
        PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT, 
	       ("[b/t/l] (add_cam_dme_entry) Count = %d, cannot yet expand list.\n" ) );
	return CAM_REQ_CMP_ERR;
      }
    
    for( i=0; i<cam_dme_entries && cam_dme_list[i].dme_name &&
        strcmp( name, cam_dme_list[i].dme_name );
        i++ )
      {}

    if ( i < cam_dme_entries )
      {
	PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT, 
	       ("[b/t/l] (add_cam_dme_entry) duplicate name '%s'\n", name ) );
        return CAM_REQ_CMP_ERR;
      }

    cam_dme_list[cam_dme_entries].dme_name = name;
    cam_dme_list[cam_dme_entries].dme_init = init;
    cam_dme_list[cam_dme_entries].dme_unload = unload;

    cam_dme_entries++;
    return CAM_REQ_CMP;
}

int 
remove_cam_dme_list( char *name )
{
    int i;

    for( i=0; i<cam_dme_entries &&  cam_dme_list[i].dme_name &&
	strcmp( name, cam_dme_list[i].dme_name );
	i++ )
      {}

    if ( i == cam_dme_entries )          /* we hit the end of the list? */
        return CAM_REQ_CMP_ERR;

    while( i < cam_dme_entries )
      {
	cam_dme_list[i].dme_name = 
            cam_dme_list[i+1].dme_name;
	cam_dme_list[i].dme_init = 
            cam_dme_list[i+1].dme_init;
	cam_dme_list[i].dme_unload = 
            cam_dme_list[i+1].dme_unload;
	
	i++;
      }

    cam_dme_list[i].dme_name = (char *) NULL;
    cam_dme_list[i].dme_init = NULL;
    cam_dme_list[i].dme_unload = NULL;

    cam_dme_entries--;
    return CAM_REQ_CMP;
}

int
name_lookup_dme_init( char *name, SIM_SOFTC *sc )
{
  int i;

    PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	   ("[b/t/l] (name_lookup_dme_init) entry - name='%s', sc=0x%x \n", 
	    name, sc ) );

  for( i=0; i<cam_dme_entries && cam_dme_list[i].dme_name &&
      strcmp( name, cam_dme_list[i].dme_name );
      i++ )
    {}

  if ( i == cam_dme_entries )
    {
      PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	     ("[b/t/l] (name_lookup_dme_init) name '%s' not found.\n", name ) );
      return CAM_REQ_CMP_ERR;
    }

  return cam_dme_list[i].dme_init( sc );
}

int
hba_dme_attach( U32 pathid, SIM_SOFTC *sc )
{
    int retval = CAM_REQ_CMP;
    char *hbaname, *dmename;            /* used in config process */
    char modname[32];                   /* TC module name - what size? -RPS */
    caddr_t csr;			/* CSR addess from softc struct */
    unsigned long tpt;

    PRINTD( NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	   ("[b/t/l] (hba_dme_attach) entry - pathid=%d, sc=0x%x \n", pathid, sc ) );

    csr = sc->csr_probe;		/* get what was stored by probe */

    switch( cpu )
        {
	case DS_3100:             /* sii base */
	    hbaname = "sii";
	    dmename = "siiram";
	    break;

        case DS_5000:             /* asc base */
	    if ( pathid == 0 )                  /* base scsi? */
	      {
              hbaname = "asc";
	      dmename = "ram94";
	      }
            else                               /* TC scsi */
	      {
		if( tc_addr_to_name( csr, modname ) == -1)
		  PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT, 
			 ("[b/t/l] (hba_dme_attach) tc_addr_to_name failed ") );
		hbaname = modname;
		dmename = modname;
	      }
	    break;

#ifdef __alpha
	case DEC_4000:
	    hbaname = "siopco";
	    dmename = "null";
	    break;

        case DEC_2000_300:
            hbaname = "aha";
            dmename = "null";
            break;

	case DEC_7000:
	    hbaname = "skz";
	    dmename = "null";
	    break;

	case DEC_3000_500:
	case DEC_3000_300:
	/* FALLTHROUGH */

#endif /* __alpha */

        case DS_5000_100:         /* asc base */
#ifdef DSPERSONAL_DECSTATION
	case DS_MAXINE:
#endif /* DSPERSONAL_DECSTATION */
	    if( tc_addr_to_name( csr, modname ) == -1)
	      PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT, 
		     ("[b/t/l] (hba_dme_attach) tc_addr_to_name failed ") );
	    hbaname = modname;
	    dmename = modname;
	    break;

#ifdef DS5000_300
	case DS_5000_300:         /*3MAX+, BIGMAX*/
	    if( tc_addr_to_name( csr, modname ) == -1)
	      PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT, 
		     ("[b/t/l] (hba_dme_attach) tc_addr_to_name failed ") );
	    hbaname = modname;
	    dmename = modname;
	    break;
#endif /* DS5000_300 */

	case DS_5100:
	    hbaname = "sii";
	    dmename = "sii_kn230";
	    break;

	case DS_5500:             /* 94 but no TC on baseboard  */
	    if ( pathid == 0 ||  pathid == 1 ||  pathid == 2 || pathid == 3 )                  /* base scsi? */
	    {
		/* 
		 * Get our controller name and compare it
		 */
		if(strcmp(((struct controller *)sc->um_probe)->ctlr_name, 
				"asc") == NULL){ 
		    hbaname = "asc";
		    dmename = "ram94";
		}
		else if(strcmp(((struct controller *)sc->um_probe)->ctlr_name, 
				"kzq") == NULL){ 
		    hbaname = "kzq";
		    dmename = "kzqram";
		}
		else {
		     hbaname = "hba_default";
		     dmename = "dme_default";
		}
	    }
	    else {
		/*
		 * Invalid pathid fail it now.
		 */
		printf("hba_dme_attach: (%d/n/n) - hba '%s' path invalid.\n",
			 pathid, hbaname );
		return(  CAM_REQ_CMP_ERR );
	    }
	    break;

	case DS_5400:             /* No such SCSI! */

	case DS_5800:             /* ? */

        default:
	    hbaname = "hba_default";
	    dmename = "dme_default";
	    break;
	}

    /*
     * First perform the attach function which will link the SIM XPT to the
     * underlying SIM HBA's and DME's. The softc for this HBA is setup to
     * allow further initialization of the CAM subsystem.
     */
    if ( name_lookup_hba_attach( hbaname, sc ) != CAM_REQ_CMP )
        {
	/* 
	 * If the attach fails don't do any further initialization.
	 */
	printf("hba_dme_attach: (%d/n/n) - hba '%s' attach failed.\n", 
	       pathid, hbaname );
	retval = CAM_REQ_CMP_ERR;
        }
    else
    {
	tpt = name_lookup_hba_reset_attach( hbaname, sc );
	if ( tpt != CAM_REQ_CMP_ERR )
	{
	    sc->hba_chip_reset = (void *) tpt;
	}
	else
	{
	    printf("hba_dme_attach: (%d/n/n) - reset attach of '%s' failed.\n",
		   pathid, hbaname );
	}
    }

    if ( name_lookup_dme_init( dmename, sc ) != CAM_REQ_CMP )
	{
	printf("hba_dme_attach: (%d/n/n) - dme '%s' attach failed.\n",
	       pathid, dmename );
	retval = CAM_REQ_CMP_ERR;
	}

    return retval;
}

#ifdef mips
/*
 * We need to figure out the best way to get this... This symbol should only
 * be defined in kn01.c in machine/mips...
 */
#define KN01SII_ADDR  PHYS_TO_K1(0x1a000000)/* phys addr of sii registers */

/**
 * sii_ds3100_spurious_interrupt -
 *
 * FUNCTIONAL DESCRIPTION:
 * In the event of an interrupt from the SII chip before the CAM subsystem
 * has been initialized, reset the SII chip.
 *
 * FORMAL PARAMETERS:  		controller - Which SII controller.
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_REQ_CMP
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
void
sii_ds3100_spurious_interrupt( int controller )
{
    SIMSII_REG *siireg;
    
    if (controller == 0)
      {
	siireg = (SIMSII_REG *) KN01SII_ADDR; /* Get address of SII CSR */
	
	/*
	 * Clear interrupt enable on SII to allow system to boot, by disabling
	 * further SII interrupts.
	 */
	siireg->csr = 0;
	
	PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	       ("[%d/t/l] (sii_ds3100_spurious_interrupt) Interrupt received before subsystem initialized,", controller ) );
	PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
		("[%d/t/l] (sii_ds3100_spurious_interrupt) interrupt now disabled.\n",
		controller ))
	}
    else
      {
	PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	       ("[%d/t/l] (sii_ds3100_spurious_interrupt) Interrupt received for unknown controller ",
		controller ) );
	PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
		("[%d/t/l] (sii_ds3100_spurious_interrupt) before subsystem initialized.\n",
		controller ));
	panic("CAM: Spurious interrupt for unknown controller\n");
	
      };
    return;
};    /* cam_sii_spurious_interrupt_before_initialization  */
#endif /* mips */

sim_enable_interrupts(sc)
SIM_SOFTC *sc;
{
    struct controller attach;	/* used by tc_enable_option */
    struct controller *prb = (struct controller *)sc->um_probe;
    char adapt[30];

    (&attach)->tcindx = prb->tcindx; /* used by tc_enable_option */

    if( strncmp(prb->bus_name, "tc", 2) == 0 )  {
	/*
	 * CPU doesn't matter in this case, if it's on a tc (or tcds)
	 * then we must enable the tc slot for interrupts.
	 */
	tc_enable_option( &attach );
    }

    switch( cpu )
    {
#ifdef DS3100
    case DS_3100:
	simsii_enable_interrupts((U32)0);
	break;
#endif /* DS3100 */

    default:
	break;
	
    }
}

hba_default_probe()
{
    printf("Unable to determine SCSI host bus adapter type!\n");
    return(0);
}
hba_default_attach()
{
    printf("Unable to determine SCSI host bus adapter type!\n");
}
hba_default_chip_reset()
{
    printf("Unable to determine SCSI host bus adapter type!\n");
}
dme_default_attach()
{
    printf("Unable to determine SCSI host bus adapter type!\n");
}
dme_default_unload()
{
    printf("Unable to determine SCSI host bus adapter type!\n");
}
