#include <stdio.h>
#include <img/ImgDef.h>

calc_crc (fid)
long   fid;
    {
    char *bufptr;
    int buflen;
    unsigned int crc_value;
    int input_ctx;
    int arsize;
    int polynomial  = 0x00008408; /* this is the CCITT polynomial */
    int initial_crc = 0x0000FFFF;
    int string;
    int ind;
    int ind2;
    struct GET_ITMLST itmlst[2];

    itmlst[0].GetL_Code  = Img_VirtualArsize;
    itmlst[0].GetL_Length = sizeof(arsize);
    itmlst[0].GetA_Buffer = (char *)&arsize;
    itmlst[0].GetA_Retlen = 0;
    itmlst[0].GetL_Index = 0;


    itmlst[1].GetL_Code  = 0;
    itmlst[1].GetL_Length = 0;
    itmlst[1].GetA_Buffer = 0;
    itmlst[1].GetA_Retlen = 0;
    itmlst[1].GetL_Index = 0;

    ImgGetFrameAttributes(fid,itmlst);
    /* Data streams must be multiples of 8 bits in length, if not, 
    ** they must be right adjusted in the string with leading 0 bits.
    */
    buflen = (((arsize + 7) / 8));
    bufptr = (char *) malloc (buflen, sizeof(char));
    ImgExportBitmap (fid, 
		     0, 
		     bufptr, 
		     buflen, 
		     0, 0, 0, 0);

    for (ind = 0; ind < (buflen); ind++)
        {

	/* xor string byte with right eight bits of the crc */
        string = (char)*bufptr;
	/* zero out all but low 8 bits */
	string = string & 0377;
	initial_crc = initial_crc ^ string;

	/* shift crc right one bit, insert zero on the left */ 
	/* The right most bit of the CRC (lost by the shift)
	** is used to control the XORing of the CRC polynomial
	** with the resultant CRC.
	** If the bit is set, the polynomial is XORed with the CRC.
	** Then the CRC is again shifted right and the polynomial is 
	** conditionally  XORed with the result a total of 8 times.
	** a 32 bit crc is produced.
	*/
    	for (ind2 = 0; ind2 < 8; ind2++)
            {
	    if (initial_crc & 0x01) 
		{
  	        initial_crc = (unsigned int) initial_crc >> 1;
	        initial_crc = (int) initial_crc ^ (int) polynomial;
		}
	    else
		initial_crc = (unsigned int)initial_crc >> 1;
	    }
	(char)bufptr++;
	}
    crc_value = initial_crc;
    printf ("The CRC for the resultant image is: %d\n",crc_value);
    return 1;
    }
