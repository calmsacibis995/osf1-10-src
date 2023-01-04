    	.TITLE	IDS_STATUS_CODES - Status and messages for Image Display Services
    	.IDENT	'Version V1.0'

    	.FACILITY IDS,493 /PREFIX = Ids$_

    	.SEVERITY SUCCESS
    	.BASE 0
!++
!	Success codes
!--
Normal			<Normal successful completion>
!++
!	Warning codes
!--
    	.SEVERITY WARNING

!++
!	Error codes
!--
	.SEVERITY ERROR

InvArgCnt	<Invalid argument count>
InvItmCod	<Invalid item code>
InvPrtTyp	<Invalid protocol type>
InvSpcTyp	<Invalid spectral type>
InvUnt		<Invalid unit specifier>
UndUntVal	<Undefined unit specifier>
UniMplFnc	<Unimplemented function>
UnsSpcTyp	<Unsupported spectral type>
XNotInUse	<Access to X window system is not present>

!++
!	Fatal codes
!--
	.SEVERITY FATAL

    	.END
