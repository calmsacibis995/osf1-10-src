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
static char *rcsid = "@(#)$RCSfile: softfp.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/07/30 21:21:07 $";
#endif

#include <sys/types.h>
#include <sys/buf.h>
#include <kern/thread.h>
#include <arch/alpha/reg.h>
#include <arch/alpha/pcb.h>
#include <arch/alpha/fpu.h>
#include <arch/alpha/inst.h>
#include <arch/alpha/softfp.h>
#include <arch/alpha/mathlib.h>


#ifdef  DEBUG
int fpdebug= 1;
#define DPRINTF(args)        printf args
#else
#define DPRINTF(args)
#endif

/*
 * OSF PAL does not fix up +/- infinity rounding like the mips chip does.
 * Instead the alpha chip issues a FEN trap which the PAL code changes
 * into an OPDEC exception.  The kernel trap handler calls this routine
 * to determine if the OPDEC was indeed caused by +/- infinity rounding
 * rather than a "normal" illegal instruction exception.
 * If it was +/- infinity then we'll fix up the result and return the 
 * information in the correct user fp register and return silently to
 * the user.  If we got here and the reason was not +/- infinity rounding,
 * then we'll return a zero which will cause the trap handler to deliver
 * an "illegal instruction exception".
 *
 * This code can be called either from user or kernel space.  The only
 * legitimate kernel invocation is from ieee floating point emulation.
 */

fp_infinity(exc_frame, excsum, excmask, user_space, result)
u_long *exc_frame;	/* pointer to the exception frame */
long   *excsum, *excmask, *result;
{
	union alpha_instruction fp;
	union fpcr fpcontrol;
	int rounding = -1, retval;
	ulong_t freg_save_area[32];
	struct pcb *pcb = current_thread()->pcb;

	/* if proc not doing fp, this can't be a legal floating point inst. */
	if (!pcb->pcb_ownedfp)
		return(FP_NOT_FP_INSTR);

	/* Fetch faulting instruction */
	if (user_space) {
		if (!useracc(exc_frame[EF_PC], sizeof(int), B_READ))
			return(0);
		fp.word = fuiword(exc_frame[EF_PC]);
	}
	else { /* kernel space */
		u_int *addr = (u_int *)exc_frame[EF_PC];
		fp.word = *addr;
	}

	/*
	 * If this is not an fp operation, it must be an illegal instruction.
	 */
	if (fp.f_format.opcode != FP_OP_OPCODE)
		return(FP_NOT_FP_INSTR);

	/* 
	 * read the floating point control register.
	 */
	fpcontrol.fc_word = _get_fpcr();

	/*
	 * If dynamic rounding is selected, look at the fpcr to test
	 * for +/- infinity rounding.
	 * Else, look at the instruction for minus infinity rounding.
	 */
	if ((fp.f_format.function & DYNAMIC_MASK) == DYNAMIC_MASK) {
		if ((fpcontrol.fc_struct.dyn_rm == ROUND_PINF) ||
		    (fpcontrol.fc_struct.dyn_rm == ROUND_MINF))
			rounding = fpcontrol.fc_struct.dyn_rm;
		else 
			return(FP_NOT_INFINITY);
	}
	else if ((fp.f_format.function & MINUS_MASK) == MINUS_MASK) {
			rounding = ROUND_MINF;
	}
	else {
		return (FP_NOT_INFINITY);
	}

	/* 
	 * Save user floating point registers.
	 * This code asumes that the trap handler has left the fp regs intact.
	 */
	freg_save(&freg_save_area[0]);

	/*
	 * We now know that we have a floating point instruction that
	 * got here because of a +/- infinity rounding error.
	 */
	DPRINTF(("fp.word = %x\n", fp.word));
	DPRINTF(("rounding = %x\n", rounding));
	DPRINTF(("fa(%x) = %lx fb(%x) = %lx fc(%x) = %lx\n",
			fp.f_format.fa, freg_save_area[fp.f_format.fa],
			fp.f_format.fb, freg_save_area[fp.f_format.fb],
			fp.f_format.fc, freg_save_area[fp.f_format.fc]));

	retval = round_result(fp, &fpcontrol, excsum, excmask, freg_save_area);

	/* 
	 * Restore user floating point registers.
	 */
	freg_restore(&freg_save_area[0]);
	if(!user_space)
		*result = freg_save_area[fp.f_format.fc];

	return (retval);
}

/*
 * Call the proper assembler routine to perform the actual rounding.
 */
round_result(fp, fpcontrol, excsum, excmask, freg_save_area)
union alpha_instruction fp;
union fpcr *fpcontrol;
long *excsum, *excmask;
ulong_t *freg_save_area;
{
	int retcode = FP_FIXED_INFINITY;
	int status = 1;
	ulong fa = freg_save_area[fp.f_format.fa];
	ulong fb = freg_save_area[fp.f_format.fb];
	ulong fc = freg_save_area[fp.f_format.fc];

	*excsum = 0;

	/* 
	 * OK. we're here to round the result.  Instructions encoded
	 * with the /M modifier are no problem since we know that we
	 * have to do -infinity rounding. If the instruction modifier 
	 * is /D, then it could be either +/- infinity.  We'll detect
	 * the proper rounding from the fpcr.  If minus infinity
	 * is selected, we'll convert the instruction to the /M modifier
	 * format so that any future reference to the /D mmodifier is
	 * guaranteed to be plus infinity rounding. For instance:
	 * an ADDTD instruction with -infinity selected in the fpcr will
	 * be changed to and ADDTM instruction.
	 */
	if (((fp.f_format.function & DYNAMIC_MASK) == DYNAMIC_MASK) &&
                		(fpcontrol->fc_struct.dyn_rm == ROUND_MINF)) {
		/*
		 * strip the dynamic bit in the instruction function field
		 * to convert it to the /M format.
		 */
		fp.f_format.function &= ~DYNAMIC_BIT;
        }

	switch (fp.f_format.function) {
		case ADDSD:
		case ADDSM:
		case ADDSUM:
		case ADDSUD:
		case ADDSSUM:
		case ADDSSUD:
		case ADDSSUIM:
		case ADDSSUID:
 			status = DOADDS(fp.f_format.function, &fa, &fb, &fc);
			break;
		case ADDTD:
		case ADDTM:
		case ADDTUM:
		case ADDTUD:
		case ADDTSUM:
		case ADDTSUD:
		case ADDTSUIM:
		case ADDTSUID:
			status = DOADDT(fp.f_format.function, &fa, &fb, &fc);
			break;

		case CVTQSM:
		case CVTQSD:
		case CVTQSSUIM:
		case CVTQSSUID:
			status = DOCVTQS(fp.f_format.function, &fb, &fc);
			break;
		case CVTQTM:
		case CVTQTD:
		case CVTQTSUIM:
		case CVTQTSUID:
			status = DOCVTQT(fp.f_format.function, &fb, &fc);
			break;

		case CVTTQD: 
		case CVTTQVD:
		case CVTTQSVD:
		case CVTTQSVID:
		case CVTTQM:
		case CVTTQVM:
		case CVTTQSVM:
		case CVTTQSVIM:
			status = DOCVTTQ(fp.f_format.function, &fb, &fc);
			break;

		case CVTTSM: 
		case CVTTSD: 
		case CVTTSUM:
		case CVTTSUD:
		case CVTTSSUM:
		case CVTTSSUD:
		case CVTTSSUIM:
		case CVTTSSUID:
			status = DOCVTTS(fp.f_format.function, &fb, &fc);
			break;

		case DIVSM: 	
		case DIVSD: 
		case DIVSUM:
		case DIVSUD:
		case DIVSSUM: 
		case DIVSSUD:
		case DIVSSUIM:
		case DIVSSUID:
			status = DODIVS(fp.f_format.function, &fa, &fb, &fc);
			break;
		case DIVTM: 	
		case DIVTD: 
		case DIVTUM:
		case DIVTUD:
		case DIVTSUM: 
		case DIVTSUD:
		case DIVTSUIM:
		case DIVTSUID:
			status = DODIVT(fp.f_format.function, &fa, &fb, &fc);
			break;

		case MULSM: 	
		case MULSD: 
		case MULSUM:
		case MULSUD:
		case MULSSUM: 
		case MULSSUD:
		case MULSSUIM:
		case MULSSUID:
			status = DOMULS(fp.f_format.function, &fa, &fb, &fc);
			break;
		case MULTM: 	
		case MULTD: 
		case MULTUM:
		case MULTUD:
		case MULTSUM: 
		case MULTSUD:
		case MULTSUIM:
		case MULTSUID:
			status = DOMULT(fp.f_format.function, &fa, &fb, &fc);
			break;

		case SUBSM: 	
		case SUBSD: 
		case SUBSUM:
		case SUBSUD:
		case SUBSSUM: 
		case SUBSSUD:
		case SUBSSUIM:
		case SUBSSUID:
			status = DOSUBS(fp.f_format.function, &fa, &fb, &fc);
			break;
		case SUBTM: 	
		case SUBTD: 
		case SUBTUM:
		case SUBTUD:
		case SUBTSUM: 
		case SUBTSUD:
		case SUBTSUIM:
		case SUBTSUID:
			status = DOSUBT(fp.f_format.function, &fa, &fb, &fc);
			break;

		default:	/* fail */
			/*
			 * instruction could not generate +/- infinity error
			 */
			retcode = FP_NOT_INFINITY;
			break;
	}

	/*
	 * Status bits are as follows:
	 *  FPCR part  |    EXCSUM part |N|       
	 * ------------|----------------|-|
	 *  2 2 1 1 1 1|             0 0| |
	 *  1 0 9 8 7 6|   7 6 5 4 3 2 1|0|
	 * ------------|----------------|-|
	 *  I I U F D I|   I I U F D I S| |
	 *  O N N O Z N|   O N N O Z N W| |
	 *  V E F V E V|   V E F V E V C| |
	 *
	 * N if set means no arithmatic trap
	 */

	DPRINTF(("round_result: status = %lx\n",status));

	/* return rounded result to user */
	freg_save_area[fp.f_format.fc] = fc;

	/* fix up excsum and excmask */
	*excsum = (((long)status >> 1) & 0x7f);
	*excmask = ((long)1 << ((long)fp.f_format.fc + 32));

	/* fix up fpcr */
	fpcontrol->fc_word |= (((long)status >> 16) << 52);
	_set_fpcr(fpcontrol->fc_word);

	if ((status & 1) == 0) {
		/* cause SIGFPE to be generated */
		retcode = FP_SIGNAL_USER;
	}
	else
		retcode = FP_FIXED_INFINITY;
	
	return(retcode);
}
