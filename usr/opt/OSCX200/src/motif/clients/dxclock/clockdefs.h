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
#ifdef OSF1
/*
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
#ifndef lint
static char *BuildSystemHeader =
  "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxclock/clockdefs.h,v 1.1.7.3 1993/10/19 19:56:28 Susan_Ng Exp $";
#endif
#endif
/*
static char *BuildSystemHeader =
  "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxclock/clockdefs.h,v 1.1.7.3 1993/10/19 19:56:28 Susan_Ng Exp $";
*/
/*
**++

  Copyright (c) Digital Equipment Corporation,
  1987, 1988, 1989, 1990, 1991, 1992
  All Rights Reserved.  Unpublished rights reserved
  under the copyright laws of the United States.

  The software contained on this media is proprietary
  to and embodies the confidential technology of
  Digital Equipment Corporation.  Possession, use,
  duplication or dissemination of the software and
  media is authorized only pursuant to a valid written
  license from Digital Equipment Corporation.

  RESTRICTED RIGHTS LEGEND   Use, duplication, or
  disclosure by the U.S. Government is subject to
  restrictions as set forth in Subparagraph (c)(1)(ii)
  of DFARS 252.227-7013, or in FAR 52.227-19, as
  applicable.

**--
**/

/*
**++
**  MODULE NAME:
**	clockdefs.h
**
**  FACILITY:
**      OOTB Clock
**
**  ABSTRACT:
**	Defines and typedefs for OOTB clock
**
**  AUTHORS:
**      Dennis McEvoy, Neal Finnegan
**
**  RELEASER:
**
**  CREATION DATE:     
**	6-OCT-1987
**
**  MODIFICATION HISTORY:
**
**    Version:
**
**	Nov 1990 - V3.0 	(ASP) - Motif conversion.
**      May 1992 - V3.1         (SP)  - I18N and maintenance changes.
**--
**/
#define NULL 0

#include <stdio.h>
#include <time.h>

#ifdef VMS
#include <types.h>
#include <descrip.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif
#ifdef VMS
#define DWTVMS
#else
#define DWTUNIX
#endif

#define bell_width 11
#define bell_height 11
static char bell_bits[] = {
   0x20, 0x00, 0x50, 0x00, 0x88, 0x00, 0x04, 0x01, 0x04, 0x01, 0x04, 0x01,
   0x82, 0x03, 0x81, 0x04, 0xff, 0x07, 0x20, 0x00, 0x20, 0x00};

                                 
#define	APPL_NAME		"Clock"
#ifdef VMS
#define	CLASS_NAME		"DECW$CLOCK"
#else
#define	CLASS_NAME		"DXclock"
#endif

#ifndef NO_HYPERHELP
#ifdef VMS
#define CLOCK_HELP		"DECW$CLOCK"
#else
#define CLOCK_HELP		"DXclock"
#endif
#endif


#define xrm_geometry		"geometry"
#define xrc_geometry		"Geometry"

#define DwtNfontFamily		"fontFamily"
#define XtCFontFamily		"FontFamily"
#define DwtNdateFontFamily	"dateFontFamily"
#define XtCDateFontFamily	"dateFontFamily"
#define DwtNdigitalFontFamily	"digitalFontFamily"
#define XtCDigitalFontFamily	"DigitalFontFamily"

#define DwtNanalogOn		"analogOn"
#define XtCAnalogOn		"AnalogOn"
#define DwtNdigitalOn		"digitalOn"
#define XtCDigitalOn		"DigitalOn"
#define DwtNdateOn		"dateOn"
#define XtCDateOn		"DateOn"
#define DwtNmilitaryOn		"militaryOn"
#define XtCMilitaryOn		"MilitaryOn"

#define DwtNmenubarOn		"menubarOn"
#define XtCMenubarOn		"MenubarOn"

#define DwtNalarmOn		"alarmOn"
#define XtCAlarmOn		"AlarmOn"
#define DwtNrepeatOn		"repeatOn"
#define XtCRepeatOn		"RepeatOn"
#define DwtNalarmPM		"alarmPM"
#define XtCAlarmPM		"AlarmPM"
#define DwtNalarmHour		"alarmHour"
#define XtCAlarmHour		"AlarmHour"
#define DwtNalarmMinute		"alarmMinute"
#define XtCAlarmMinute		"AlarmMinute"
#define DwtNalarmMessage	"alarmMessage"
#define XtCAlarmMessage		"AlarmMessage"
#define DwtNdatePercDn		"datePercDn"
#define XtCDatePercDn		"DatePercDn"
#define DwtNdatePercDa		"datePercDa"
#define XtCDatePercDa		"DatePercDa"
#define DwtNanalogPercDa	"analogPercDa"
#define XtCAnalogPercDa		"AnalogPercDa"
#define DwtNanalogPercNa	"analogPercNa"
#define XtCAnalogPercNa		"AnalogPercNa"
#define DwtNanalogPercDna	"analogPercDna"
#define XtCAnalogPercDna	"AnalogPercDna"

#define DwtNlanguage		"language"
#define XtCLanguage		"Language"
#define DwtNdateFormat		"dateFormat"
#define XtCDateFormat		"DateFormat"
#define DwtNtimeFormat		"timeFormat"
#define XtCTimeFormat		"TimeFormat"
#define DwtNmilitaryFormat	"militaryFormat"
#define XtCMilitaryFormat	"MilitaryFormat"
#define DwtNbellOn		"bellOn"
#define XtCBellOn		"BellOn"
#define DwtNaudioOn		"audioOn"
#define XtCAudioOn		"AudioOn"
#define DwtNspeakerOn		"speakerOn"
#define XtCSpeakerOn		"SpeakerOn"
#define DwtNaudioVolume		"audioVolume"
#define XtCAudioVolume		"AudioVolume"
#define DwtNaudioFilename	"audioFilename"
#define XtCAudioFilename	"AudioFilename"
#define DwtNaudioDirMask	"audioDirMask"
#define XtCAudioDirMask		"AudioDirMask"

#define GeoMode(r) ((r)->request_mode)

/* These defines allow easy access to the pieces of the widget structure */
#define ClockHierarchy(w) ((w)->clock.clock_hierarchy)
#define ContextPtr(w) ((w)->clock.context_ptr)
#define ArrayContextPtr(w) ((w)->clock.array_context_ptr)
#define AnalogPart(w) ((w)->clock.analog_part)
#define DigitalPart(w) ((w)->clock.digital_part)
#define DatePart(w) ((w)->clock.date_part)
#define AlarmBell(w) ((w)->clock.alarm_bell)
#define CurrentDay(w) ((w)->clock.current_day)

#define Dpy(w) ((w)->clock.dpy)
#define Win(w) (XtWindow((w)->clock.analog_part))

#define TopMenuBar(w) ((w)->clock.top_menu_bar)
#define Menu(w) ((w)->clock.menu)
#define Settings(w) ((w)->clock.settings)
#define AlarmSettings(w) ((w)->clock.alarm_settings)

#define AnalogWid(w) ((w)->clock.analog_wid)
#define DigitalWid(w) ((w)->clock.digital_wid)
#define DateWid(w) ((w)->clock.date_wid)
#define AlarmWid(w) ((w)->clock.alarm_wid)
#define RepeatWid(w) ((w)->clock.repeat_wid)
#define MilitaryTimeWid(w) ((w)->clock.military_time_wid)
#define MenubarWid(w) ((w)->clock.menubar_wid)
#define AmWid(w) ((w)->clock.am_wid)
#define PmWid(w) ((w)->clock.pm_wid)
#define HrWid(w) ((w)->clock.hr_wid)
#define MinWid(w) ((w)->clock.min_wid)
#define AlarmMesWid(w) ((w)->clock.alarm_mes_wid)
#define SettingsOkWid(w) ((w)->clock.settings_ok_wid)
#define MessageWid(w) ((w)->clock.message_wid)
#define HelpWidget(w) ((w)->clock.help_widget)
#define ErrorMessageWid(w) ((w)->clock.error_message_wid)
#define BellOnWid(w) ((w)->clock.bell_on_wid)
#define AudioOnWid(w) ((w)->clock.audio_on_wid)
#define SpeakerOnWid(w) ((w)->clock.speaker_on_wid)
#define HeadphoneOnWid(w) ((w)->clock.headphone_on_wid)
#define AudioVolumeWid(w) ((w)->clock.audio_volume_wid)
#define AudioFnameTextWid(w) ((w)->clock.audio_fname_text_wid)
#define AudioFSBoxWid(w) ((w)->clock.audio_fsbox_wid)
#define AudioFnameButtonWid(w) ((w)->clock.audio_fname_button_wid)
#define AudioPlayButtonWid(w) ((w)->clock.audio_play_button_wid)
#define AudioStopButtonWid(w) ((w)->clock.audio_stop_button_wid)
#define AudioOutputMenuWid(w) ((w)->clock.audio_output_menu_wid)

#define UserDatabase(w) ((w)->clock.user_database)
#define SystemDatabase(w) ((w)->clock.system_database)
#define MergedDatabase(w) ((w)->clock.merged_database)
/*
#define DefaultsName(w) ((w)->clock.defaults_name)
#define SystemDefaultsName(w) ((w)->clock.system_defaults_name)
*/
#define DigitalPresent(w) ((w)->clock.digital_present)
#define AnalogPresent(w) ((w)->clock.analog_present)
#define DatePresent(w) ((w)->clock.date_present)
#define MilitaryTime(w) ((w)->clock.military_time)
#define MenubarPresent(w) ((w)->clock.menubar_present)
#define NoMbMinwidth(w) ((w)->clock.no_mb_minwidth)
#define NoMbMinheight(w) ((w)->clock.no_mb_minheight)
#define Minwidth(w) ((w)->clock.minwidth)
#define Minheight(w) ((w)->clock.minheight)
#define ClockType(w) ((w)->clock.clock_type)

#define LanguageId(w) ((w)->clock.language_id)
#define IsUSLocale(w) ((w)->clock.is_us_locale)
#define Is12HrLocale(w) ((w)->clock.is_12hr_locale)
#define Language(w) ((w)->clock.language)

#define CSDateFormat(w) ((w)->clock.CSdate_format)
#define CSTimeFormat(w) ((w)->clock.CStime_format)
#define CSMilitaryFormat(w) ((w)->clock.CSmilitary_format)

#define DateFormat(w) ((w)->clock.date_format)
#define TimeFormat(w) ((w)->clock.time_format)
#define MilitaryFormat(w) ((w)->clock.military_format)
     
#define DayPtr(w) ((w)->clock.day_ptr)
#define MonthPtr(w) ((w)->clock.month_ptr)
#define NumPtr(w) ((w)->clock.num_ptr)
#define DatePtr(w) ((w)->clock.date_ptr)
#define DigPtr(w) ((w)->clock.dig_ptr)
#define Months(w) ((w)->clock.months)
#define Days(w) ((w)->clock.days)
#define Amstr(w) ((w)->clock.amstr)
#define Pmstr(w) ((w)->clock.pmstr)
#define LongestDate(w) ((w)->clock.longest_date)
#define LongestTime(w) ((w)->clock.longest_time)

#define DatePercDn(w) ((w)->clock.date_perc_dn)
#define DatePercDa(w) ((w)->clock.date_perc_da)
#define AnalogPercDa(w) ((w)->clock.analog_perc_da)
#define AnalogPercNa(w) ((w)->clock.analog_perc_na)
#define AnalogPercDna(w) ((w)->clock.analog_perc_dna)

#define AlarmOn(w) ((w)->clock.alarm_on)
#define RepeatOn(w) ((w)->clock.repeat_on)
#define AlarmPm(w) ((w)->clock.alarm_pm)
#define AlarmHr(w) ((w)->clock.alarm_hr)
#define AlarmMin(w) ((w)->clock.alarm_min)
#define AlarmMes(w) ((w)->clock.alarm_mes)
#define AudioCapable(w) ((w)->clock.audio_capable)
#define BellOn(w) ((w)->clock.bell_on)
#ifndef NO_AUDIO 
#define AudioChannel(w) ((w)->clock.audio_channel)
#define AudioHardware(w) ((w)->clock.audio_hardware)
#endif
#define AudioOn(w) ((w)->clock.audio_on)
#define SpeakerOn(w) ((w)->clock.speaker_on)
#define HeadphoneOn(w) ((w)->clock.headphone_on)
#define AudioVolume(w) ((w)->clock.audio_volume)
#define AudioFname(w) ((w)->clock.audio_fname)
#define AudioFilename(w) ((w)->clock.audio_filename)
#define AudioDirMask(w) ((w)->clock.audio_dirmask)
#define AudioDirectoryMask(w) ((w)->clock.audio_dir_mask)
#define AudioAsyncActive(w) ((w)->clock.audio_async_active)
#define AudioPlayLabel(w) ((w)->clock.audio_play_label)
#define AudioStopLabel(w) ((w)->clock.audio_stop_label)
#define AudioState(w) ((w)->clock.audio_state)
#define AlarmHour(w) ((w)->clock.alarm_hour)
#define AlarmMinute(w) ((w)->clock.alarm_minute)
#define AlarmMessage(w) ((w)->clock.alarm_message)
#define AnalogGC(w) ((w)->clock.analog_gc)
#define DigitalGC(w) ((w)->clock.digital_gc)
#define DateGC(w) ((w)->clock.date_gc)
#define AnalogClearGC(w) ((w)->clock.analog_clear_gc)
#define DigitalClearGC(w) ((w)->clock.digital_clear_gc)
#define DateClearGC(w) ((w)->clock.date_clear_gc)
#define BellGC(w) ((w)->clock.bell_gc)
#define BellPixmap(w) ((w)->clock.bell_pixmap)

#define HandWidth(w) ((w)->clock.hand_width)
#define LastTm(w) ((w)->clock.last_tm)
#define DigitalFontHeight(w) ((w)->clock.digital_font_height)
#define DigitalFont(w) ((w)->clock.digital_font)
#define DateFontHeight(w) ((w)->clock.date_font_height)
#define DateFont(w) ((w)->clock.date_font)
/* Begin I18N_CHANGE */
#define DigitalFontList(w) ((w)->clock.digital_fontlist)
#define DateFontList(w) ((w)->clock.date_fontlist)
/* End I18N_CHANGE */

/*jv - inserted to kluge around problem that XmDrawingArea widget doesn't
** have a font list resource that clock tries to access for the digital part
*/
#define ClockFont(w) ((w)->clock.clock_font)

#define Fonts(w) ((w)->clock.fonts)
#define NumFonts(w) ((w)->clock.num_fonts)
#define FontHeights(w) ((w)->clock.font_heights)
#define DateFontWidths(w) ((w)->clock.date_font_widths)
#define DigitalFontWidths(w) ((w)->clock.digital_font_widths)

#define Parent(w) ((w)->core.parent)
#define Width(w) ((w)->core.width)
#define Height(w) ((w)->core.height)
#define X(w) ((w)->core.x)
#define Y(w) ((w)->core.y)

#define Double(x)	((x) << 1)
#define Half(x)		((x) >> 1)

#define TotalWidth(w)   (XtWidth  (w) + Double (XtBorderWidth (w)))
#define TotalHeight(w)  (XtHeight (w) + Double (XtBorderWidth (w)))


/* Global Definitions */

#define		k_analog_part		1
#define		k_digital_part		2
#define		k_date_part		3
#define		k_day_part		4
#define		k_month_part		5
#define		k_mode_settings		6
#define		k_alarm_settings	7
#define		k_message		8
#define		k_menu			9
#define		k_error_message		10
#define		k_menu_bar		11

#define		k_settings_analog	20
#define		k_settings_digital	21
#define		k_settings_date		22
#define		k_settings_alarm	23
#define		k_settings_hr		24
#define		k_settings_min		25
#define		k_settings_am		26
#define		k_settings_pm		27
#define		k_settings_alarm_mes	28
#define		k_settings_ok		29
#define		k_settings_military	30
#define		k_settings_menubar	31
#define		k_settings_repeat	32
#define		k_settings_bell		33
#define		k_settings_audio	34
#define		k_settings_speaker	35
#define		k_settings_headphone	36
#define		k_settings_volume	37
#define		k_settings_audio_fname	38
#define		k_settings_fname_button	39
#define		k_settings_play_button	40
#define		k_settings_stop_button	41
#define		k_settings_audio_menu	42

#define		k_main_help		99

#define 	audio_stop_state	0
#define		audio_play_state	1

#define	app_name	"Clock"
#define	BadLanguage	"Can't get language from toolkit, using US English \n"
#define	BadLocale	"Can't open locale \n"
#define	NoPopup		"Can't fetch popup menu \n"
#define NoHierarchy 	"Can't open hierarchy \n"
#define	NoClockMain	"Can't fetch clock widget \n"
#define	NoBellPixmap	"Can't create logo pixmap \n"
#define	NoHelpWidget	"Can't fetch help widget \n"
#define	NoSettings	"Can't fetch settings widget \n"
#define NoFSBox		"Can't fetch file selection box \n"


#define PI			3.14159265358979
#define TWOPI			(2. * PI)

#define	NUM_DAYS		7	/* Number of days in the week	*/
#define	NUM_MONTHS		12	/* Number of months in a year	*/
/* Begin I18N_CHANGE */
#define DAY_NUMERAL		31	/* Number of Asian Numeral	*/
#define MIN_NUMERAL 		60
#define HOUR_NUMERAL 		24
/* End I18N_CHANGE */

#define MAX_FILE_LEN		256

#define AM 			0	/* am flag 			   */
#define PM			1	/* pm flag 			   */
#define MINUTE_HAND		0	/* minute hand flag		   */
#define HOUR_HAND		1	/* hour hand flag		   */

#define MINUTE_HAND_FRACT	70	/* % of radius to make minute hand */
#define HOUR_HAND_FRACT		40	/* % of radius to make hour hand   */

#define SEC_UPDATE		1	/* how often to update clock -
					   every second to begin 	   */
#define MIN_UPDATE	    	60      /* every minute once its started   */

#define BOX_FRACTION		20	/* fraction of the window size 
					   to make boxes on clock face     */

#define DEF_PADDING		8	/* default padding for subwindows  */

/* define clock type literals */

#define DigitalDateAnalog	1
#define DigitalDate	        2
#define DigitalAnalog		3
#define DateAnalog		4
#define Analog			5
#define Digital			6
#define Date			7

/*
 *  The following letters are used to identify clock
 *  types in the following literals.
 * 	D : date
 *	N : digital (would you believe numeric?)
 *	A : analog
 *  So, _DNA identifies a clock with all three subwindows:
 *  date, digital, and analog.
 */

#define DATE_PERC_DN		50	/* % of DN clock for date subwindow */
#define DATE_PERC_DA		75	/* % of DN clock for date subwindow */
#define ANALOG_PERC_DNA		40	/* % of DNA clock for analog subwindow 
*/
#define ANALOG_PERC_NA		75	/* % of NA clock for analog subwindow */
#define ANALOG_PERC_DA		75	/* % of DA clock for analog subwindow */

#define INFORMATION		0
#define WARNING			1
#define ERROR			2
#define FATAL			3

/*
  	Several OS specific defines
OK_STATUS - status to return for success
ERROR_STATUS - status to return for failure
*/                                                      

#ifdef VMS
#define	OK_STATUS	1
#define ERROR_STATUS	0
#else
#define	OK_STATUS	0
#define ERROR_STATUS	1
#endif

#ifdef VMS

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *                      VMSDSC (VMS descriptors)
 *                      ========================
 *
 *  These macros require the following header file:
 *
 *              descrip.h
 *
 * VMSDSC (typedef)
 * ------
 *
 * $INIT_VMSDSC (intialize a VMS static descriptor)
 * ------------
 *      dsc             : VMSDSC        : write : value
 *      len             : int           : read  : value
 *      addr            : char          : read  : ref
 *
 *    RETURNS: address of the descriptor (dsc)
 */

#define VMSDSC          struct dsc$descriptor

#define $INIT_VMSDSC(dsc, len, addr)            \
        ((dsc .dsc$b_class) = DSC$K_CLASS_S,    \
         (dsc .dsc$b_dtype) = DSC$K_DTYPE_T,    \
         (dsc .dsc$w_length) = (len),           \
         (dsc .dsc$a_pointer) = (addr),         \
         (& (dsc) )                             \
        )
#endif

/* Begin I18N_CHANGE */
#define DayNumerals(w) ((w)->clock.day_numbers)
#define HourNumerals(w) ((w)->clock.hour_numbers)
#define MinNumerals(w) ((w)->clock.min_numbers)
#define DaySuffix(w) ((w)->clock.day_suffix)
#define HourSuffix(w) ((w)->clock.hour_suffix)
#define MinSuffix(w) ((w)->clock.min_suffix)
#define DigitalFontLists(w) ((w)->clock.digital_fontlists)
#define DateFontLists(w) ((w)->clock.date_fontlists)

#define IsAsianLocale(s) (((s) != NULL) && \
	((((s)[0]=='j') && ((s)[1]=='a')) || \
	 (((s)[0]=='i') && ((s)[1]=='w')) || \
	 (((s)[0]=='k') && ((s)[1]=='o')) || \
	 (((s)[0]=='z') && ((s)[1]=='h'))))

#define IsJaLocale(s) (((s) != NULL) && \
	(((s)[0]=='j') && ((s)[1]=='a')))
#define IsKoLocale(s) (((s) != NULL) && \
	(((s)[0]=='k') && ((s)[1]=='o')))
#define IsCNLocale(s) (((s) != NULL) && \
	(((s)[0]=='z') && ((s)[1]=='h') && ((s)[2]=='_') && \
	 ((s)[3]=='C') && ((s)[4]=='N')))
#define IsTWLocale(s) (((s) != NULL) && \
	(((s)[0]=='z') && ((s)[1]=='h') && ((s)[2]=='_') && \
	 ((s)[3]=='T') && ((s)[4]=='W')))
#define IsHeLocale(s) (((s) != NULL) && \
	(((s)[0]=='i') && ((s)[1]=='w')))
/* End I18N_CHANGE */

