Options = -DSYS5DIR -D_BSD -DISI -DMHRC -DWHATNOW -DMHE -DBSD42 -DBERK \
          -DBSD43 -DLOCKF -DNETWORK -DBIND -DRPATHS -DDPOP -DRPOP -DUK \
          -DFOLDPROT='"0700"' -DMSGPROT='"0600"' -DTYPESIG='void' \
          -DPOPSERVICE='"pop"' -DSPRINTF='int' -DSENDMTS -DSMTP -DPOP -DX400
INCFLAGS = -I..
BBDIR	=	/usr/spool/bboards
BBETC	=	$(BBDIR)/etc
BBUID	=	bboards
CFLAGS = ${CFLAGS} ${Options}
LIBES1	=	../../zotnet/libzot.a
LIBES2	=	../../config/config.o ../../sbr/libmh.a $(LIBES1)
LDLIBS1	=	$(LIBES1)
LDLIBS2	=	$(LIBES2)

$(BBDIR)/BBoards: BBoards
		-@if [ ! -f $@ ]; then \
		    $(REMOVE) $@ zBBoards; \
		    cp BBoards $@; \
		else \
		    echo "Will not overwrite existing $@"; \
		fi
		-$(CHOWN) $(BBUID) $@
		-@ls -l $@
		-@echo ""


######################################################################
# bbaka
######################################################################

inst-bbaka:	$(BBETC)/bbaka

$(BBETC)/bbaka: xbbaka
		-$(REMOVE) $@ zbbaka
		cp xbbaka $@
		-$(CHOWN) $(BBUID) $@
		-@ls -l $@
		-@echo ""

bbaka:		xbbaka

xbbaka:		bbaka.o $(LIBES1)
		$(CC) $(LDFLAGS) -o $@ bbaka.o $(LDLIBS1)

l-bbaka:;	$(LINT) $(LFLAGS) bbaka.c $(LLIBS1)


######################################################################
# bbexp
######################################################################

inst-bbexp:	$(BBETC)/bbexp

$(BBETC)/bbexp: xbbexp
		-$(REMOVE) $@ zbbexp
		cp xbbexp $@
		-$(CHOWN) $(BBUID) $@
		-@ls -l $@
		-@echo ""

bbexp:		xbbexp

xbbexp:		bbexp.o ../../uip/dropsbr.o $(LIBES2)
		$(CC) $(LDFLAGS) -o $@ bbexp.o ../../uip/dropsbr.o $(LDLIBS2)

l-bbexp:;	$(LINT) $(LFLAGS) bbexp.c ../../uip/dropsbr.c $(LLIBS2)


######################################################################
# bbtar
######################################################################

inst-bbtar:	$(BBETC)/bbtar

$(BBETC)/bbtar: xbbtar
		-$(REMOVE) $@ zbbtar
		cp xbbtar $@
		-$(CHOWN) $(BBUID) $@
		-@ls -l $@
		-@echo ""

bbtar:		xbbtar

xbbtar:		bbtar.o $(LIBES1)
		$(CC) $(LDFLAGS) -o $@ bbtar.o $(LDLIBS1)

l-bbtar:;	$(LINT) $(LFLAGS) bbtar.c $(LLIBS1)


######################################################################
# bboards.daily
######################################################################

inst-daily:	$(BBETC)/bboards.daily

$(BBETC)/bboards.daily: bboards.daily
		-$(REMOVE) $@ zbboards.daily
		cp bboards.daily $@
		-$(CHOWN) $(BBUID) $@
		-@chmod +x $@
		-@ls -l $@
		-@echo ""


######################################################################
# bboards.weekly
######################################################################

inst-weekly:	$(BBETC)/bboards.weekly

$(BBETC)/bboards.weekly: bboards.weekly
		-$(REMOVE) $@ zbboards.weekly
		cp bboards.weekly $@
		-$(CHOWN) $(BBUID) $@
		-@chmod +x $@
		-@ls -l $@
		-@echo ""


######################################################################
# MakeBBoards
######################################################################

inst-makebboards:	$(BBETC)/Makefile

$(BBETC)/Makefile: MakeBBoards
		-$(REMOVE) $@ zMakeBBoards
		cp MakeBBoards $@
		-$(CHOWN) $(BBUID) $@
		-@ls -l $@
		-@echo ""


######################################################################
# .cshrc
######################################################################

inst-cshell:	$(BBDIR)/.cshrc

$(BBDIR)/.cshrc: cshrc
		-$(REMOVE) $@ z.cshrc
		cp cshrc $@
		-$(CHOWN) $(BBUID) $@
		-@ls -l $@
		-@echo ""


######################################################################
# .mh_profile
######################################################################

inst-profile:	$(BBDIR)/.mh_profile

$(BBDIR)/.mh_profile: mh_profile
		-$(REMOVE) $@ z.mh_profile
		cp mh_profile $@
		-$(CHOWN) $(BBUID) $@
		-@ls -l $@
		-@touch $(BBETC)/context
		-$(CHOWN) $(BBUID) $(BBETC)/context
		-@echo ""


##############################################################################
#	Miscellaneous tasks
##############################################################################

unconfig:	distribution
		-rm -f Makefile

distribution:	clean
		-rm -f bboards.daily bboards.weekly crontab MakeBBoards

clean:		unclean
		-rm -f x* *.o

unclean:;	-rm -f z* _* :* core eddep makedep MAKEBB


##############################################################################
#	Dependencies
##############################################################################

MODULES	=	bbaka bbexp bbtar

depend:;	for m in $(MODULES); do ( \
		    i=`basename $$m .c`; \
		    echo $$i.o: $$i.c >> makedep; \
		    grep '^#[ 	]*include' $$i.c | \
			sed -e 's,[^"]*"/\([^"]*\)".*,'$$i'.o: /\1,' \
			    -e 's,[^"]*"\([^"]*\)".*,'$$i'.o: \1,' \
			    -e 's,[^<]*<\(.*\)>.*,#'$$i'.o: /usr/include/\1,' \
			>> makedep \
		); done
		echo '/^# DO NOT DELETE THIS LINE/+2,$$d' > eddep
		echo '$$r makedep' >> eddep
		echo 'w' >> eddep
		cp Makefile _Makefile
		ed - Makefile < eddep
		rm eddep makedep
		echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
		echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile

# DO NOT DELETE THIS LINE
# DEPENDENCIES START HERE
bbaka.o: bbaka.c
bbaka.o: ../../h/strings.h
bbaka.o: ../../zotnet/bboards.h
#bbaka.o: /usr/include/stdio.h
bbaka.o: ../../zotnet/mts.h
bbexp.o: bbexp.c
bbexp.o: ../../h/mh.h
bbexp.o: ../../h/dropsbr.h
bbexp.o: ../../zotnet/bboards.h
#bbexp.o: /usr/include/pwd.h
#bbexp.o: /usr/include/signal.h
#bbexp.o: /usr/include/stdio.h
#bbexp.o: /usr/include/sys/types.h
#bbexp.o: /usr/include/sys/stat.h
#bbexp.o: /usr/include/stdio.h
bbexp.o: ../../zotnet/bboards.h
bbtar.o: bbtar.c
#bbtar.o: /usr/include/pwd.h
#bbtar.o: /usr/include/stdio.h
#bbtar.o: /usr/include/sys/types.h
#bbtar.o: /usr/include/sys/stat.h
bbtar.o: ../../zotnet/bboards.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
