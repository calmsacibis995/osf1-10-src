MOM		= trn 
MOM_BUILD	= ./

all:
#
# Insert function call into init.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* MOM-specific initialization here. \*\*\//         dot5_mom_specific_init\(\);/;\
	s/\"rfc1231_mom\"/\"trn_mom\"/;\
	s/global_mom_name = \"rfc1231\"/global_mom_name = \"trn\"/;\
	s/LOG_PID, LOG_DAEMON/LOG_PID\|LOG_CONS\|LOG_NDELAY, LOG_DAEMON/;\
	s/MODIFICATION HISTORY:/MODIFICATION HISTORY: call added to dot5_mom_specific_init\(\)/' init.c > tmp.$$$$; \
	mv -f tmp.$$$$ init.c


#
# Insert function call into EXP_dot5_perform_getnext.c and 
# EXP_dot5_get_instance.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* add function call to assure class structure is up-to-date (if necessary) \*\*\//status = \(man_status\) refresh_EXP_dot5_list\(new_EXP_dot5_header\); if ERROR_CONDITION\(status\) return status;/' EXP_dot5_perform_getnext.c > tmp.$$$$; \
	mv -f tmp.$$$$ EXP_dot5_perform_getnext.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_EXP_dot5_list\(new_EXP_dot5_header\); if ERROR_CONDITION\(status\) return status;/' EXP_dot5_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ EXP_dot5_get_instance.c


#
# Insert function call into dot5Entry_perform_getnext.c and 
# dot5Entry_get_instance.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* add function call to assure class structure is up-to-date (if necessary) \*\*\//status = \(man_status\) refresh_dot5Entry_list\(new_dot5Entry_header\); if ERROR_CONDITION\(status\) return status;/' dot5Entry_perform_getnext.c > tmp.$$$$; \
	mv -f tmp.$$$$ dot5Entry_perform_getnext.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_dot5Entry_list\(new_dot5Entry_header\); if ERROR_CONDITION\(status\) return status;/' dot5Entry_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ dot5Entry_get_instance.c


#
# Insert function call into dot5StatsEntry_perform_getnext.c and
# dot5StatsEntry_get_instance.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* add function call to assure class structure is up-to-date (if necessary) \*\*\//status = \(man_status\) refresh_dot5StatsEntry_list\(new_dot5StatsEntry_header\); if ERROR_CONDITION\(status\) return status;/' dot5StatsEntry_perform_getnext.c > tmp.$$$$; \
	mv -f tmp.$$$$ dot5StatsEntry_perform_getnext.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_dot5StatsEntry_list\(new_dot5StatsEntry_header\); if ERROR_CONDITION\(status\) return status;/' dot5StatsEntry_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ dot5StatsEntry_get_instance.c

#
# Insert function call into dot5TimerEntry_perform_getnext.c and
# dot5TimerEntry_get_instance.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* add function call to assure class structure is up-to-date (if necessary) \*\*\//status = \(man_status\) refresh_dot5TimerEntry_list\(new_dot5TimerEntry_header\); if ERROR_CONDITION\(status\) return status;/' dot5TimerEntry_perform_getnext.c > tmp.$$$$; \
	mv -f tmp.$$$$ dot5TimerEntry_perform_getnext.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_dot5TimerEntry_list\(new_dot5TimerEntry_header\); if ERROR_CONDITION\(status\) return status;/' dot5TimerEntry_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ dot5TimerEntry_get_instance.c

# Add the modules to the Makefile
	cd $(MOM_BUILD); \
	sed 's/dot5_SRC =/dot5_SRC = dot5_mom_specific.c /; \
	s/EXP_dot5_SRC =/EXP_dot5_SRC = EXP_dot5_grp.c/; \
	s/dot5Entry_SRC =/dot5Entry_SRC = dot5_entry.c/; \
	s/dot5StatsEntry_SRC =/dot5StatsEntry_SRC = dot5Stats_entry.c/; \
	s/dot5TimerEntry_SRC =/dot5TimerEntry_SRC = dot5Timer_entry.c/' Makefile > tmp.$$$$; \
	mv -f tmp.$$$$ Makefile
	cp `genloc /src/usr/sbin/${MOM}_mom/dot5_mom_specific.h`          $(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/dot5_mom_specific.c`          $(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/EXP_dot5_grp.c`          	$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/dot5_entry.c`          	$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/dot5Stats_entry.c`          	$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/dot5Timer_entry.c`         	$(MOM_BUILD)/.
	chmod 644 $(MOM_BUILD)/*.c
	chmod 644 $(MOM_BUILD)/*.h
