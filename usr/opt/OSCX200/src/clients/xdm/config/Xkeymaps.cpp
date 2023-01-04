XCOMM 
XCOMM This file defines the language-keymap mapping
XCOMM 
XCOMM 
XCOMM   The first line contains the name of the
XCOMM   link to be created to the default keymap.
XCOMM 
LIBDIR/keymap_default
XCOMM 
XCOMM   This is the directory where the keymap files are to be found.
XCOMM 
LIBDIR/keymaps/
XCOMM 
XCOMM   The following lines must contain: <number> <language> <keymap-filename>
XCOMM 
XCOMM 	The <number> field is a 2-byte hex value where the first byte
XCOMM 	represents the keyboard type and the second byte is the value of the
XCOMM 	console's language variable.  The values for the keyboard types are:
XCOMM 		LK401	0
XCOMM 		PCXAL	1
XCOMM 		LK201	2
XCOMM 		LK421	3
XCOMM 		LK443/4	4 
XCOMM 
XCOMM 	Don't put any 8-bit characters in the language names or the the
XCOMM 	isspace() function used in parsing this may think they're spaces
XCOMM 	causing the lines to be parsed incorrectly.
XCOMM 
XCOMM 	If the <keymap-filename> field is blank, this has the special meaning
XCOMM 	that no keymap_default link will be created, nor will any existing
XCOMM 	keymap_default be modified.
XCOMM 
XCOMM 	The keymap specified for the "fallback" lines is used for any
XCOMM 	language value missing from the table for the corresponding keyboard
XCOMM 	type.
XCOMM 
000	fallback		us_lk401aa.decw_keymap
030	Dansk			danish_lk401ad_tw.decw_keymap
032	Deutsch			austrian_german_lk401ag.decw_keymap
034	Deutsch(Schweiz)	swiss_german_lk401al_tw.decw_keymap
036	English(American)	us_lk401aa.decw_keymap
038	English(British/Irish)	uk_lk401aa.decw_keymap
03a	Espanol			spanish_lk401as_tw.decw_keymap
03c	Francais		belgian_french_lk401ap_tw.decw_keymap
03e	Francais(Canadien)	canadian_french_lk401ac_tw.decw_keymap
040	Francais(SuisseRomande)	swiss_french_lk401ak_tw.decw_keymap
042	Italiano		italian_lk401ai_tw.decw_keymap
044	Nederlands		dutch_us_lk401ah.decw_keymap
046	Norsk			norwegian_lk401an_tw.decw_keymap
048	Portugues		portuguese_lk401av.decw_keymap
04a	Suomi			finnish_lk401af_tw.decw_keymap
04c	Svenska			swedish_lk401am_tw.decw_keymap
04e	Vlaams			flemish_lk401ab_tw.decw_keymap

100	fallback		us_pcxalka.decw_keymap
130	Dansk			danish_pcxalkd.decw_keymap
132	Deutsch			austrian_german_pcxalkg.decw_keymap
134	Deutsch(Schweiz)	swiss_german_pcxalmh.decw_keymap
136	English(American)	us_pcxalka.decw_keymap
138	English(British/Irish)	uk_pcxalae.decw_keymap
13a	Espanol			spanish_pcxalks.decw_keymap
13c	Francais		french_pcxalkp.decw_keymap
13e	Francais(Canadien)	french_canadian_pcxalac.decw_keymap
140	Francais(SuisseRomande)	swiss_french_pcxalap.decw_keymap
142	Italiano		italian_pcxalki.decw_keymap
144	Nederlands		dutch_pcxalgh.decw_keymap
146	Norsk			norwegian_pcxalkn.decw_keymap
148	Portugues		portuguese_pcxalkv.decw_keymap
14a	Suomi			finnish_pcxalca.decw_keymap
14c	Svenska			swedish_pcxalma.decw_keymap
14e	Vlaams			belgian_pcxalkb.decw_keymap

200	fallback		us_lk201re.decw_keymap
230	Dansk			danish_lk201ld_tw.decw_keymap	*
232	Deutsch			austrian_german_lk201lg_tw.decw_keymap	*
234	Deutsch(Schweiz)	swiss_german_lk201ll_tw.decw_keymap
236	English(American)	us_lk201re.decw_keymap
238	English(British/Irish)	uk_lk201re.decw_keymap
23a	Espanol			spanish_lk201ls_tw.decw_keymap
23c	Francais		belgian_french_lk201lp_tw.decw_keymap
23e	Francais(Canadien)	canadian_french_lk201lc_tw.decw_keymap
240	Francais(SuisseRomande)	swiss_french_lk201lk_tw.decw_keymap
242	Italiano		italian_lk201li_tw.decw_keymap
244	Nederlands		dutch_lk201lh_tw.decw_keymap	*
246	Norsk			norwegian_lk201ln_tw.decw_keymap
248	Portugues		portuguese_lk201lv.decw_keymap
24a	Suomi			finnish_lk201lf_tw.decw_keymap	*
24c	Svenska			swedish_lk201lm_tw.decw_keymap	*
24e	Vlaams			flemish_lk201lb_tw.decw_keymap

300	fallback		us_lk421aa.decw_keymap
336	English(American)	us_lk421aa.decw_keymap
338	English(British/Irish)	uk_lk421aa.decw_keymap

400	fallback		us_lk443aa.decw_keymap
430	Dansk			danish_lk444kd.decw_keymap
432	Deutsch			austrian_german_lk444kg.decw_keymap
434	Deutsch(Schweiz)	swiss_german_lk444mh.decw_keymap
436	English(American)	us_lk443aa.decw_keymap
438	English(British/Irish)	uk_lk444ae.decw_keymap
43a	Espanol			spanish_lk444ks.decw_keymap
43c	Francais		french_lk444kp.decw_keymap
43e	Francais(Canadien)	french_canadian_lk444ac.decw_keymap
440	Francais(SuisseRomande)	swiss_french_lk444ap.decw_keymap
442	Italiano		italian_lk444ki.decw_keymap
444	Nederlands		dutch_lk444gh.decw_keymap
446	Norsk			norwegian_lk444kn.decw_keymap
448	Portugues		portuguese_lk444kv.decw_keymap
44a	Suomi			finnish_lk444ca.decw_keymap
44c	Svenska			swedish_lk444ma.decw_keymap
44e	Vlaams			belgian_lk444kb.decw_keymap
