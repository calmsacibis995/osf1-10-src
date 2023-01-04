	/***** DVI COMMANDS: operation codes found in the DVI file *****/

#define ID_BYTE 2	/* current version of the dvi format */
#define SET_CHAR_0 0	/* typeset character 0 and move right */
#define SET_CHAR_127 127 /* typeset character 127 and move right */
#define SET1 128	/* typeset a character and move right */
#define SET4 131	/* typeset a character and move right */
#define SET_RULE 132	/* typeset a rule and move right */
#define PUT1 133	/* typeset a character */
#define PUT_RULE 137	/* typeset a rule */
#define NOP 138		/* no operation */
#define BOP 139		/* beginning of page */
#define EOP 140		/* ending of page */
#define PUSH 141	/* save the current positions */
#define POP 142		/* restore previous positions */
#define RIGHT1 143	/* move right */
#define W0 147		/* move right by |w| */
#define W1 148		/* move right and set |w| */
#define X0 152		/* move right by |x| */
#define X1 153		/* move right and set |x| */
#define DOWN1 157	/* move down */
#define Y0 161		/* move down by |y| */
#define Y1 162		/* move down and set |y| */
#define Z0 166		/* move down by |z| */
#define Z1 167		/* move down and set |z| */
#define FNT_NUM_0 171	/* set current font to 0 */
#define FNT_NUM_63 234	/* set current font to 63 */
#define FNT1 235	/* set current font (number is in parameter) */
#define XXX1 239	/* extension to dvi primitives (\special) */
#define XXX4 242	/* potentially long extension to dvi primitives */
#define FNT_DEF1 243	/* define a font */
#define PRE 247		/* preamble */
#define POST 248	/* postamble beginning */
#define POST_POST 249	/* postamble ending */
#define UNDEFINED_COMMAND 250

        /***** ERROR HANDLING *****/
        /* The globalvalues are ID's of user msgs stored in an external message
        /* object file.  The longword passes the length of a string argument to
        /* such a message. */


/***** THE txf STRUCTURE STORES INFORMATION FOR ONE TEX FONT. PICTORIALLY:
/*
/*          +---+---+------+-------+-------+------+------+
/*          | bc| ec| space| design| scaled| *font| *font|
/*          |   |   |      | size  | size  |  name| width|
/*          +---+---+------+-------+-------+------+------+
/**/

struct txf {unsigned bc;		/*beginning char (lowest code)*/
	    unsigned ec;		/*ending char (highest code)*/
	    int space;
	    int design_size;   
	    int scaled_size; 
	    char *font_name;		/*ptr to filename*/
	    int *font_width;	/*ptr to table of widths*/
	    int *font_height;	/* ptr to table of heights*/
	    int *font_depth;	/* ptr to table of depths*/
	    int bbox_depth;	/* max depth in font	*/
	   };

#define GLOBAL globaldef
#define EXTERN globalref

/******* MACRO definitions ************************************************/

#define max(x,y) (((x)>(y))?(x):(y))
#define min(x,y) (((x)<(y))?(x):(y))
#define pixel_round(x) ((int) ((x<0) ?  (conv*(x)-0.5) : (conv*(x)+0.5)))

/*****miscellaneous ****/
#define MAXCHARINTITLE 50

/***** VALUES TO CONTROL \special PROCESSING *****/

#define spec_execute	1
#define spec_noexecute	2

#define DEFPOINTSPECL	 0
#define CONNECTSPECL	 1
#define PLOTFILESPECL	 2
#define RESETPOINTSSPECL 3
#define DVIFILESPECL	 4
#define ONLINETOCSPECL   5
#define ONLINEIDXSPECL   6
#define ONLINECHUNK	 7
#define HOTSPOT		 8
#define CHUNKTITLE       9
#define EXTENSION	 10
#define LMF		 11
#define SHELF		 12
#define BOOK_SYMBOL	 13
	/* NOSPECIALS is, ambiguously, "number of specials" & "no specials" */
#define NOSPECIALS	 14

	/* SPECIALOUTPUTERROR means we got an I/O error writing to O/P file */
#define SPECIALOUTPUTERROR  65535


/***** DATATYPES FOR GRAPHICS *****/

#define PLOT_RAGS     1
#define PLOT_IMAGE    11
#define PLOT_IMAGE75  1175
#define PLOT_IMAGE100 11100
#define CHILD_REFERENCE 2
#define PIXELS_PER_INCH (400.0)

#define pts2pixels(n)  (n * 400 / 72)
#define scaledptsperinch (4718592.0)
#define pixelsperinch (400.0)
#define scaledptsperpixel (scaledptsperinch / pixelsperinch)
#define centimetersperinch (2.54)

/***** TYPES OF DIRECTORIES: Need not be contiguous *****/

#define DIR_CONTENTS  1
#define DIR_INDEX     2

/***** TYPES OF TOPICS: Must be contiguous from zero *****/

#define TOPIC_MAINLINE  0
#define TOPIC_FORMAL    1
#define TOPIC_DIRECTORY 2
#define NUM_TOPIC_TYPES 3

/************ HOTSPOT AND EXTENSION DEFINES **********************************/
#define RECTANGLE 1
#define POLYGON   2

#define HOT_START 0
#define HOT_END   1
#define EXTENSION_START 2
#define EXTENSION_END   3

       /***************(IN_ prefix INCLUDE)********************/
           /* things to control including DVI files */

#define IN_PRIMARYDVI 0
#define IN_INCLUDEDVI 1
#define IN_MAXDVISEQ 20

/************* Instead of 8 different stacks we have an array of structs *****/
struct coordinates {
        int h;      /*horizontal position in TeX scaled points*/
        int v;      /*vertical position in TeX scaled points*/
        int x;
        int y;
        int z;
        int w;
        int hh;     /*pixel-rounded version of 'h'*/
        int vv;     /*pixel-rounded version of 'v'*/
        int hoff;   /*horizontal offset of origin, in pixels*/
        int voff;   /*vertical offset of origin, in pixels*/
        };

/************ NEW struct for defining vector points *********************/
struct POINTS {
	int x;
	int y;
	};

/** defines a region of text for hotspots or extensions ******/
struct region_t{
    int     xorg;
    int     yorg;
    int     width;
    int     height;
    int     depth;
    int     numpoints;
    struct POINTS pt_vec[8];
    int     delete_flag;
    int     shape;
    int     type;
    char    *symbol_name;
    struct  region_t    *next;
    struct  region_t    *prev;
    };
   



	/***** DATA STRUCTURE TO HOLD DESCRIPTION OF DEFERRED RECORD *****/
	/*       A DEFERRED RECORD MAY BE A PLOTFILE OR A HOTSPOT        */

struct DEFERREDREC {
    struct DEFERREDREC	    *dfr_nextrec;
    int	    dfr_datatype;
    int	    dfr_bbox[4];	/*Xtopleft, Ytopleft, Xbotright, Ybotright */
				/*in units specific to datatype		   */
    int	    dfr_offX;		/*X and Y offset from start of parent chunk*/
    int	    dfr_offY;
    int	    dfr_normalwidth;	/*Width and height in "standard" units     */
    int	    dfr_normalheight;
    int	    dfr_crop_flag;
    unsigned char   dfr_crop_value;
    int	    dfr_type;		/* HOTSPOT or EXTENSION */
    int	    dfr_numpoints;      /* number of  x/y coord in this region */
    struct POINTS  (*dfr_pt_vec)[8];	/* ptr to array of coordinates that */
					/* define  a region */
    char    *dfr_filnam;	/*Ptr to name of file containing graphic  */
};


#define MAXTEXFONTS 512   /* used to be 100 */
#define DEFAULT_CHARSET 0
#define TEXMATH_REMAP  99
#define PAGE_INCR 2000;

struct enumtable {
    int enumdef[MAXTEXFONTS+1];         /*TRUE means font defined in curr DVI*/
    int enumval[MAXTEXFONTS+1];         /*DVI's font number for this font*/
};

struct box{
    int xorg;
    int yorg;
    int width;
    int height;
    int depth;
    };

/** Put  all the status and mode flags in one struct so we can look **/
/** in one place to get the status or mode of the build		    **/

struct build_status {
    int		hot_spot;
    int		hot_spot_type;
    int		hotpacket;
    int		numpoints;
    int		saveveebox;
    int		setposition;
    int		saveveeboxctr;
    int		num_fonts;		/*number of fonts defined so far*/
    int		current_font;		/*internal number of current font*/
    int		building_dir;		/* building a directory if TRUE */
    int		insubentry;		
    int		extracttitle;		/* grabbing ascii title		*/
    int		indextitle;		/* grabbing index title for deferrment*/
    int		endformalpending;	/* formal topic closed waiting for EOP*/
    int		newchunkneeded;		/* just what it says */
    };

