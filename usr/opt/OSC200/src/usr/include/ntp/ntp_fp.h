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
 *	@(#)$RCSfile: ntp_fp.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:41:15 $
 */
/*
 */

/*
 * ntp_fp.h - definitions for NTP fixed point arithmetic
 */

/*
 * NTP uses two fixed point formats.  The first (l_fp) is the "long" format
 * and is 64 bits long with the decimal between bits 31 and 32.  This
 * is used for time stamps in the NTP packet header (in network byte
 * order) and for internal computations of offsets (in local host byte
 * order).  We use the same structure for both signed and unsigned values,
 * which is a big hack but saves rewriting all the operators twice.  Just
 * to confuse this, we also sometimes just carry the fractional part in
 * calculations, in both signed and unsigned forms.  Anyway, an l_fp looks
 * like:
 *
 *    0			  1		      2			  3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |			       Integral Part			     |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |			       Fractional Part			     |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */
typedef struct {
	union {
		u_int Xl_ui;
		int Xl_i;
	} Ul_i;
	union {
		u_int Xl_uf;
		int Xl_f;
	} Ul_f;
} l_fp;

#define l_ui	Ul_i.Xl_ui		/* unsigned integral part */
#define	l_i	Ul_i.Xl_i		/* signed integral part */
#define	l_uf	Ul_f.Xl_uf		/* unsigned fractional part */
#define	l_f	Ul_f.Xl_f		/* signed fractional part */

/*
 * Fractional precision (of an l_fp) is actually the number of
 * bits in a long.
 */
#define	FRACTION_PREC	(32)


/*
 * The second fixed point format is 32 bits, with the decimal between
 * bits 15 and 16.  There is a signed version (s_fp) and an unsigned
 * version (u_fp).  This is used to represent synchronizing distance
 * and synchronizing dispersion in the NTP packet header (again, in
 * network byte order) and internally to hold both distance and
 * dispersion values (in local byte order).  In network byte order
 * it looks like:
 *
 *    0			  1		      2			  3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |		  Integer Part	     |	   Fraction Part	     |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */
typedef int s_fp;
typedef u_int u_fp;

/*
 * A unit second in fp format.  Actually 2**(half_the_bits_in_a_long)
 */
#define	FP_SECOND	(0x10000)

/*
 * Byte order conversions
 */
#define	HTONS_FP(x)	(htonl(x))
#define	HTONL_FP(h, n)	do { (n)->l_ui = htonl((h)->l_ui); \
			     (n)->l_uf = htonl((h)->l_uf); } while (0)
#define	NTOHS_FP(x)	(ntohl(x))
#define	NTOHL_FP(n, h)	do { (h)->l_ui = ntohl((n)->l_ui); \
			     (h)->l_uf = ntohl((n)->l_uf); } while (0)
#define	NTOHL_MFP(ni, nf, hi, hf) \
	do { (hi) = ntohl(ni); (hf) = ntohl(nf); } while (0)
#define	HTONL_MFP(hi, hf, ni, nf) \
	do { (ni) = ntohl(hi); (nf) = ntohl(hf); } while (0)

/* funny ones.  Converts ts fractions to net order ts */
#define	HTONL_UF(uf, nts) \
	do { (nts)->l_ui = 0; (nts)->l_uf = htonl(uf); } while (0)
#define	HTONL_F(f, nts) do { (nts)->l_uf = htonl(f); \
				if ((f) & 0x80000000) \
					(nts)->l_i = -1; \
				else \
					(nts)->l_i = 0; \
			} while (0)

/*
 * Conversions between the two fixed point types
 */
#define	MFPTOFP(x_i, x_f)	(((x_i)<<16) | (((x_f)>>16)&0xffff))
#define	LFPTOFP(v)		MFPTOFP((v)->l_ui, (v)->l_uf)

/*
 * Primative operations on long fixed point values.  If these are
 * reminiscent of assembler op codes it's only because some may
 * be replaced by inline assembler for particular machines someday.
 * These are the (kind of inefficient) run-anywhere versions.
 */
#define	M_NEG(v_i, v_f) 	/* v = -v */ \
	do { \
		if ((v_f) == 0) \
			(v_i) = -(v_i); \
		else { \
			(v_f) = -(v_f); \
			(v_i) = ~(v_i); \
		} \
	} while(0)

#define	M_NEGM(r_i, r_f, a_i, a_f) 	/* r = -a */ \
	do { \
		if ((a_f) == 0) { \
			(r_f) = 0; \
			(r_i) = -(a_i); \
		} else { \
			(r_f) = -(a_f); \
			(r_i) = ~(a_i); \
		} \
	} while(0)

#define M_ADD(r_i, r_f, a_i, a_f) 	/* r += a */ \
	do { \
		register u_int lo_tmp; \
		register u_int hi_tmp; \
		\
		lo_tmp = ((r_f) & 0xffff) + ((a_f) & 0xffff); \
		hi_tmp = (((r_f) >> 16) & 0xffff) + (((a_f) >> 16) & 0xffff); \
		if (lo_tmp & 0x10000) \
			hi_tmp++; \
		(r_f) = ((hi_tmp & 0xffff) << 16) | (lo_tmp & 0xffff); \
		\
		(r_i) += (a_i); \
		if (hi_tmp & 0x10000) \
			(r_i)++; \
	} while (0)

#define M_ADD3(r_ovr, r_i, r_f, a_ovr, a_i, a_f) /* r += a, three word */ \
	do { \
		register u_int lo_tmp; \
		register u_int hi_tmp; \
		\
		lo_tmp = ((r_f) & 0xffff) + ((a_f) & 0xffff); \
		hi_tmp = (((r_f) >> 16) & 0xffff) + (((a_f) >> 16) & 0xffff); \
		if (lo_tmp & 0x10000) \
			hi_tmp++; \
		(r_f) = ((hi_tmp & 0xffff) << 16) | (lo_tmp & 0xffff); \
		\
		lo_tmp = ((r_i) & 0xffff) + ((a_i) & 0xffff); \
		if (hi_tmp & 0x10000) \
			lo_tmp++; \
		hi_tmp = (((r_i) >> 16) & 0xffff) + (((a_i) >> 16) & 0xffff); \
		if (lo_tmp & 0x10000) \
			hi_tmp++; \
		(r_i) = ((hi_tmp & 0xffff) << 16) | (lo_tmp & 0xffff); \
		\
		(r_ovr) += (a_ovr); \
		if (hi_tmp & 0x10000) \
			(r_ovr)++; \
	} while (0)

#define M_SUB(r_i, r_f, a_i, a_f)	/* r -= a */ \
	do { \
		register u_int lo_tmp; \
		register u_int hi_tmp; \
		\
		if ((a_f) == 0) { \
			(r_i) -= (a_i); \
		} else { \
			lo_tmp = ((r_f) & 0xffff) + ((-(a_f)) & 0xffff); \
			hi_tmp = (((r_f) >> 16) & 0xffff) \
			    + (((-(a_f)) >> 16) & 0xffff); \
			if (lo_tmp & 0x10000) \
				hi_tmp++; \
			(r_f) = ((hi_tmp & 0xffff) << 16) | (lo_tmp & 0xffff); \
			\
			(r_i) += ~(a_i); \
			if (hi_tmp & 0x10000) \
				(r_i)++; \
		} \
	} while (0)

#define	M_RSHIFTU(v_i, v_f)		/* v >>= 1, v is unsigned */ \
	do { \
		(v_f) = (u_int)(v_f) >> 1; \
		if ((v_i) & 01) \
			(v_f) |= 0x80000000; \
		(v_i) = (u_int)(v_i) >> 1; \
	} while (0)

#define	M_RSHIFT(v_i, v_f)		/* v >>= 1, v is signed */ \
	do { \
		(v_f) = (u_int)(v_f) >> 1; \
		if ((v_i) & 01) \
			(v_f) |= 0x80000000; \
		if ((v_i) & 0x80000000) \
			(v_i) = ((v_i) >> 1) | 0x80000000; \
		else \
			(v_i) = (v_i) >> 1; \
	} while (0)

#define	M_LSHIFT(v_i, v_f)		/* v <<= 1 */ \
	do { \
		(v_i) <<= 1; \
		if ((v_f) & 0x80000000) \
			(v_i) |= 0x1; \
		(v_f) <<= 1; \
	} while (0)

#define	M_LSHIFT3(v_ovr, v_i, v_f)	/* v <<= 1, with overflow */ \
	do { \
		(v_ovr) <<= 1; \
		if ((v_i) & 0x80000000) \
			(v_ovr) |= 0x1; \
		(v_i) <<= 1; \
		if ((v_f) & 0x80000000) \
			(v_i) |= 0x1; \
		(v_f) <<= 1; \
	} while (0)

#define	M_ADDUF(r_i, r_f, uf) 		/* r += uf, uf is u_int fraction */ \
	M_ADD((r_i), (r_f), 0, (uf))	/* let optimizer worry about it */

#define	M_SUBUF(r_i, r_f, uf)		/* r -= uf, uf is u_int fraction */ \
	M_SUB((r_i), (r_f), 0, (uf))	/* let optimizer worry about it */

#define	M_ADDF(r_i, r_f, f)		/* r += f, f is a int fraction */ \
	do { \
		if ((f) > 0) \
			M_ADD((r_i), (r_f), 0, (f)); \
		else if ((f) < 0) \
			M_ADD((r_i), (r_f), (-1), (f));\
	} while(0)

#define	M_ISNEG(v_i, v_f) 		/* v < 0 */ \
	(((v_i) & 0x80000000) != 0)

#define	M_ISHIS(a_i, a_f, b_i, b_f)	/* a >= b unsigned */ \
	(((u_int)(a_i)) > ((u_int)(b_i)) || \
	  ((a_i) == (b_i) && ((u_int)(a_f)) >= ((u_int)(b_f))))

#define	M_ISGEQ(a_i, a_f, b_i, b_f)	/* a >= b signed */ \
	(((int)(a_i)) > ((int)(b_i)) || \
	  ((a_i) == (b_i) && ((u_int)(a_f)) >= ((u_int)(b_f))))

#define	M_ISEQU(a_i, a_f, b_i, b_f)	/* a == b unsigned */ \
	((a_i) == (b_i) && (a_f) == (b_f))

/*
 * Operations on the long fp format
 */
#define	L_ADD(r, a)	M_ADD((r)->l_ui, (r)->l_uf, (a)->l_ui, (a)->l_uf)
#define	L_SUB(r, a)	M_SUB((r)->l_ui, (r)->l_uf, (a)->l_ui, (a)->l_uf)
#define	L_NEG(v)	M_NEG((v)->l_ui, (v)->l_uf)
#define L_ADDUF(r, uf)	M_ADDUF((r)->l_ui, (r)->l_uf, (uf))
#define L_SUBUF(r, uf)	M_SUBUF((r)->l_ui, (r)->l_uf, (uf))
#define	L_ADDF(r, f)	M_ADDF((r)->l_ui, (r)->l_uf, (f))
#define	L_RSHIFT(v)	M_RSHIFT((v)->l_i, (v)->l_uf)
#define	L_RSHIFTU(v)	M_RSHIFT((v)->l_ui, (v)->l_uf)
#define	L_LSHIFT(v)	M_LSHIFT((v)->l_ui, (v)->l_uf)
#define	L_CLR(v)	((v)->l_ui = (v)->l_uf = 0)

#define	L_ISNEG(v)	(((v)->l_ui & 0x80000000) != 0)
#define	L_ISHIS(a, b)	((a)->l_ui > (b)->l_ui || \
			  ((a)->l_ui == (b)->l_ui && (a)->l_uf >= (b)->l_uf))
#define	L_ISGEQ(a, b)	((a)->l_i > (b)->l_i || \
			  ((a)->l_i == (b)->l_i && (a)->l_uf >= (b)->l_uf))
#define	L_ISEQU(a, b)	M_ISEQU((a)->l_ui, (a)->l_uf, (b)->l_ui, (b)->l_uf)
