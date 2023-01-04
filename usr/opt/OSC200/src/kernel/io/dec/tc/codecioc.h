/*
 * @(#)$RCSfile: codecioc.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/11/04 16:59:19 $
 */
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
 * @(#)$RCSfile: codecioc.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/11/04 16:59:19 $
 */

/*
 * OLD HISTORY
 * Revision 1.1.2.6  92/11/18  10:31:26  Narayan_Mohanram
 * 	"PPP additions"
 * 
 * Revision 1.1.4.2  92/11/11  14:11:10  Narayan_Mohanram
 * 	Added define for CODECIOC_HDLC
 * 
 * Revision 1.1.2.5  92/08/19  16:14:34  Narayan_Mohanram
 * 	added isdnioc_snoop ioctl
 * 	[92/08/19  16:13:38  Narayan_Mohanram]
 * 
 * Revision 1.1.2.4  92/08/03  13:43:16  Narayan_Mohanram
 * 	fix ODE branch screwup.
 * 	[92/08/03  13:41:42  Narayan_Mohanram]
 * 
 * Revision 1.1.3.2  92/06/22  11:01:56  Narayan_Mohanram
 * 	Added new ioctl to support 56KB conn -- narayan
 * 
 * Revision 1.1.2.2  92/05/21  10:04:02  Narayan_Mohanram
 * 	New header file for device controls -narayan-
 * 	[92/05/21  10:02:16  Narayan_Mohanram]
 * 
 * 	Initial check in - narayan-
 * 	[92/05/19  17:03:14  Narayan_Mohanram]
 * 
 */

#ifndef _ISDNIOC_H_
#define _ISDNIOC_H_
/*
 * Ioctl's that are for dealing with the codec's
 */
#define ISDNIOC_SPEAKER_ENABLE	(('A' << 8) | 1)
#define ISDNIOC_EAR_ENABLE	(('A' << 8) | 2)
#define ISDNIOC_MIC_ENABLE	(('A' << 8) | 3)
#define ISDNIOC_VOLUME_CHANGE	(('A' << 8) | 4) /* DON'T USE - OLD */
#define ISDNIOC_DIAL_TONE	(('A' << 8) | 5) /* Start/stop dial tone*/
#define ISDNIOC_MIC_CODEC_0	(('A' << 8) | 6) /* Send mic to codec 0	*/
#define ISDNIOC_MIC_INPUT	(('A' << 8) | 7) /* Get mic-input	*/
#define ISDNIOC_SPEAKER_OUTPUT	(('A' << 8) | 8) /* Speaker Output	*/
#define ISDNIOC_DTMF		(('A' << 8) | 9) /* DTMF Generation	*/
#define ISDNIOC_RING_TONE	(('A' << 8) | 10) /* Ringing Generation	*/
#define ISDNIOC_56KB		(('A' << 8) | 11)/* Make this a 56KB con*/
#define ISDNIOC_SNOOP		(('A' << 8) | 12)/* Sniffer program	*/
#define ISDNIOC_HDLC		(('A' << 8) | 13)/* Enable HDLC		*/

/* This define is for reading from and writing to registers on the audio
   chip.  Changing chip settings may cause internal flags kept by the
   driver to be out-of-sync with the chip.  Use with caution.
   */
#define BBAIOC_CODEC_INDIRECT	(('A' << 8) | 14)/* read/write register	*/

/* RE: ALL GAIN SETTINGS
   Values for all of the gain settings are from 0 to 100 with 0 being
   the minimum setting, and 100 being the maximum.  In certain cases the
   minimum setting may attenuate the signal (bring the signal below 0 dB).
   The maximum setting may bring the signal to 0dB.  -1 indicates the
   signal should be muted.

   Register	Minimum	Default	Maximum	(all values in dB)
   GR		-12	0	0
   GER		-10	0	18
   GA		0	0	24
   GX		0	0	12
   STG		-18	-18	0
   */

/* This is used to set the voice gain (GR register).  The voice gain deals with
   the digital audio signal comming from the mux heading to the digitial->analog
   converter.
   */
#define BBA_SET_VOICE_OUT_GAIN	(('A' << 8) | 15)

/* This is used to set the hearing impaired gain (GER register).
   This register is used to set the gain for the sum of: the voice, 
   tone generators, and Sidetone signals. */
#define BBA_SET_H_I_GAIN	(('A' << 8) | 16)

/* This is used to set the microphone preamp gain (GA register) which comes
   before the analog->digital converter. */
#define BBA_SET_PREAMP_GAIN	(('A' << 8) | 17)

/* This is used to set the gain on the input channel's (microphone) digital
   signal (GX register). */
#define BBA_SET_IN_GAIN		(('A' << 8) | 18)

/* This is used to set the Sidetone/Feedback gain (STG register).  This register
   controls the feedback from the microphone input to the speaker output. */
#define BBA_SET_FEEDBACK_GAIN	(('A' << 8) | 19)

/* The following are used as the first int argument of the BBA_SET_*_GAIN
   calls. */
#define BBA_GAIN_PERCENTAGE	0 /* The gain is in percent */
#define BBA_GAIN_DB		1 /* The gain is in dB */
#define BBA_MUTE		2 /* Mute the device */
#define BBA_STEP_VOLUME		3 /* Step the volume up/down 1 increment.
				     Result is in dB */

/* This is used to find the gain parameters for the codec.  Used in conjunction
   with struct bba_get_gain_param.
   NOTE: BBA_GET_CURRENT_GAIN is a newer interface, and should be used.
   */
#define BBA_GET_GAIN_PARAMETERS	(('A' << 8) | 20)

/* Structure used for BBA_GET_GAIN_PARAMETERS */
struct bba_get_gain_param
{
	int bba_call;		/* The call you want params for (eg. BBA_SET_H_I_GAIN) */
	int minval;		/* The minimum legal value in dB in 1/10dB */
	int maxval;		/* The maximum legal value in dB in 1/10dB */
	int step;		/* The minimum granularity in 1/10dB */
	int map_reg;		/* The map register (used w/bbaioc_codec_indirect) */
};

/* Used to find out about the codec. */
#define BBA_GET_AUDIO_DEV_CAP_SIZE (('A' << 8) | 21)

/* Get the table of supported sampling frequencies */
#define BBA_GET_AUDIO_DEV_CAP_TABLE (('A' << 8) | 22)

/* Set the sample rate in Hz */
#define BBA_SET_SAMPLE_RATE_TYPE (('A' << 8) | 23)

/* Get the current sample rate in Hz */
#define BBA_GET_SAMPLE_RATE_TYPE (('A' << 8) | 24)

/* Set current sample number (for Play) */
#define BBA_SET_SAMPLE_NUMBER	(('A' << 8) | 25)

/* Get current sample number (for Play) */
#define BBA_GET_SAMPLE_NUMBER	(('A' << 8) | 26)

/* Get in_sample count, out_sample count, and timestamp */
#define BBA_GET_SAMPLE_COUNTS	(('A' << 8) | 27)

struct bba_sample_counts
{
	ulong	record;		/* Sample count on input */
	ulong	play;		/* Sample count on playback */
	struct timeval timestamp; /* Timestamp */
};

/* Get The current gain settings */
#define BBA_GET_CURRENT_GAIN	(('A' << 8) | 28)

struct bba_get_current_gain
{
	int	bba_call;	/* The call you want params for (eg. BBA_SET_H_I_GAIN) */
	int	minval;		/* The minimum legal value in dB in 1/10dB */
	int	maxval;		/* The maximum legal value in dB in 1/10dB */
	int	step;		/* The minimum granularity in 1/10dB */
	int	map_reg;	/* The map register (used w/bbaioc_codec_indirect) */
	int	muted;		/* If True, then this is muted! (invalid
				   for mic preamp gain) */
	int	gain_db;	/* The gain in db */
	int	gain_pct;	/* The gain in percent */
};

/* Structure used for BBAIOC_CODEC_INDIRECT */
/* Structure used for BBAIOC_CODEC_INDIRECT */
struct bba_indirect
{
	int bi_regno;		/* The register number */
	int bi_len;		/* The length of the data to be read/written */
	int bi_direction;	/* The direction */
};

#define BBA_DT_LINEAR	0x01	/* Linear data */
#define BBA_DT_MU_LAW	0x02	/* Mu-Law data */
#define BBA_DT_A_LAW	0x04	/* A-Law data */
#define BBA_DT_ADPCM	0x08	/* Adaptive Delta Pulse Code Modulation (IMA) */

struct bba_audio_dev_cap
{
	int fq;			/* Frequency of samples/second */
	int bits_sample;	/* Bits per sample */
	int opt_samps;		/* Optimum # samples in read/write (samples returned by read) */
	int min_samps;		/* Minimum # samples in read/write */
	int max_samps;		/* Maximum # samples in read/write */
	int chan_in;		/* Number of channels supported on input */
	int chan_out;		/* Number of channels supported on output */
	int types_in;		/* The types of data accepted by the driver/codec */
	int types_out;
	int dsp_type;		/* 0 == codec */
	char dsp_name[16];	/* ASCII representation of dsp name */
};

#endif /* _ISDNIOC_H_ */
