XCOMM
XCOMM Xservers file, workstation prototype
XCOMM
XCOMM This file should contain an entry to start the server on the
XCOMM local display; if you have more than one display (not screen),
XCOMM you can add entries to the list (one per line).  If you also
XCOMM have some X terminals connected which do not support XDMCP,
XCOMM you can add them here as well.  Each X terminal line should
XCOMM look like:
XCOMM	XTerminalName:0 foreign
XCOMM
XCOMM To use the shared memory transport, change the :0 in the ":0 local" 
XCOMM line to local:0 like this:
XCOMM 	local:0 local BINDIR/X 
XCOMM
:0 local BINDIR/X 
