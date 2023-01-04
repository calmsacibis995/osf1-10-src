# /usr/sbin/fddi_mom/change_src.make
MOM		= fddi 
MOM_BUILD	= ./

all:
#
# Insert function call into init.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* MOM-specific initialization here. \*\*\//         fddi_mom_specific_init\(\);/;\
	s/\"rfc1285_mom\"/\"fddi_mom\"/;\
	s/global_mom_name = \"rfc1285\"/global_mom_name = \"fddi\"/;\
	s/LOG_PID, LOG_DAEMON/LOG_PID\|LOG_CONS\|LOG_NDELAY, LOG_DAEMON/;\
	s/MODIFICATION HISTORY:/MODIFICATION HISTORY: call added to fddi_mom_specific_init\(\)/' init.c > tmp.$$$$; \
	mv -f tmp.$$$$ init.c

#
# Insert function call into snmpFddiSMT_perform_getnext.c and 
# snmpFddiSMT_get_instance.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* add function call to assure class structure is up-to-date (if necessary) \*\*\//status = \(man_status\) refresh_smt_list\(new_snmpFddiSMT_header\); if ERROR_CONDITION\(status\) return status;/' snmpFddiSMT_perform_getnext.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiSMT_perform_getnext.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status =\(man_status\) refresh_smt_list\(new_snmpFddiSMT_header\); if ERROR_CONDITION\(status\) return status;/' snmpFddiSMT_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiSMT_get_instance.c

#
# Insert function call into snmpFddiSMTEntry_perform_getnext.c and 
# snmpFddiSMTEntry_get_instance.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* add function call to assure class structure is up-to-date (if necessary) \*\*\//status = \(man_status\) refresh_smtEntry_list\(new_snmpFddiSMTEntry_header\); if ERROR_CONDITION\(status\) return status;/' snmpFddiSMTEntry_perform_getnext.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiSMTEntry_perform_getnext.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_smtEntry_list\(new_snmpFddiSMTEntry_header\); if ERROR_CONDITION\(status\) return status;/' snmpFddiSMTEntry_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiSMTEntry_get_instance.c

#    snmpFddiSMTEntry_perform_set.c - insert function call
	cd $(MOM_BUILD); \
	sed '/case snmpFddiSMTEntry_ATTR_snmpFddiSMTOpVersionId  \:/,/break\;/d' snmpFddiSMTEntry_perform_set.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiSMTEntry_perform_set.c

#
# Insert function call into snmpFddiMAC_perform_getnext.c and
# snmpFddiMAC_get_instance.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* add function call to assure class structure is up-to-date (if necessary) \*\*\//status = \(man_status\) refresh_mac_list\(new_snmpFddiMAC_header\); if ERROR_CONDITION\(status\) return status;/' snmpFddiMAC_perform_getnext.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiMAC_perform_getnext.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_mac_list\(new_snmpFddiMAC_header\); if ERROR_CONDITION\(status\) return status;/' snmpFddiMAC_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiMAC_get_instance.c

#
# Insert function call into snmpFddiMACEntry_perform_getnext.c and
# snmpFddiMACEntry_get_instance.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* add function call to assure class structure is up-to-date (if necessary) \*\*\//status = \(man_status\) refresh_macEntry_list\(new_snmpFddiMACEntry_header\); if ERROR_CONDITION\(status\) return status;/' snmpFddiMACEntry_perform_getnext.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiMACEntry_perform_getnext.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_macEntry_list\(new_snmpFddiMACEntry_header\); if ERROR_CONDITION\(status\) return status;/' snmpFddiMACEntry_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiMACEntry_get_instance.c

#    snmpFddiMACEntry_perform_set.c - insert function call
	cd $(MOM_BUILD); \
	sed '/case snmpFddiMACEntry_ATTR_snmpFddiMACTMaxGreatestLowerBound  \:/,/break\;/d' snmpFddiMACEntry_perform_set.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiMACEntry_perform_set.c

#
# Insert function call into snmpFddiPORT_perform_getnext.c and
# snmpFddiPORT_get_instance.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* add function call to assure class structure is up-to-date (if necessary) \*\*\//status = \(man_status\) refresh_port_list\(new_snmpFddiPORT_header\); if ERROR_CONDITION\(status\) return status;/' snmpFddiPORT_perform_getnext.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiPORT_perform_getnext.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_port_list\(new_snmpFddiPORT_header\); if ERROR_CONDITION\(status\) return status;/' snmpFddiPORT_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiPORT_get_instance.c

#
# Insert function call into snmpFddiPORTEntry_perform_getnext.c and
# snmpFddiPORTEntry_get_instance.c


	cd $(MOM_BUILD); \
	sed 's/\/\*\* add function call to assure class structure is up-to-date (if necessary) \*\*\//status = \(man_status\) refresh_portEntry_list\(new_snmpFddiPORTEntry_header\); if ERROR_CONDITION\(status\) return status;/' snmpFddiPORTEntry_perform_getnext.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiPORTEntry_perform_getnext.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_portEntry_list\(new_snmpFddiPORTEntry_header\); if ERROR_CONDITION\(status\) return status;/' snmpFddiPORTEntry_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiPORTEntry_get_instance.c

#    snmpFddiPORTEntry_perform_set.c - insert function call
	cd $(MOM_BUILD); \
	sed '/case snmpFddiPORTEntry_ATTR_snmpFddiPORTConnectionPolicies  \:/,/break\;/d' snmpFddiPORTEntry_perform_set.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiPORTEntry_perform_set.c

#
# Insert function call into snmpFddiATTACHMENT_perform_getnext.c and
# snmpFddiATTACHMENT_get_instance.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* add function call to assure class structure is up-to-date (if necessary) \*\*\//status = \(man_status\) refresh_attachment_list\(new_snmpFddiATTACHMENT_header\); if ERROR_CONDITION\(status\) return status;/' snmpFddiATTACHMENT_perform_getnext.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiATTACHMENT_perform_getnext.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_attachment_list\(new_snmpFddiATTACHMENT_header\); if ERROR_CONDITION\(status\) return status;/' snmpFddiATTACHMENT_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiATTACHMENT_get_instance.c

#
# Insert function call into snmpFddiATTACHMENTEntry_perform_getnext.c and
# snmpFddiATTACHMENTEntry_get_instance.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* add function call to assure class structure is up-to-date (if necessary) \*\*\//status = \(man_status\) refresh_attachmentEntry_list\(new_snmpFddiATTACHMENTEntry_header\); if ERROR_CONDITION\(status\) return status;/' snmpFddiATTACHMENTEntry_perform_getnext.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiATTACHMENTEntry_perform_getnext.c

	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_attachmentEntry_list\(new_snmpFddiATTACHMENTEntry_header\); if ERROR_CONDITION\(status\) return status;/' snmpFddiATTACHMENTEntry_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiATTACHMENTEntry_get_instance.c; \


#    snmpFddiATTACHMENTEntry_perform_set.c - insert function call
	cd $(MOM_BUILD); \
	sed '/case snmpFddiATTACHMENTEntry_ATTR_snmpFddiATTACHMENTInsertPolicy  \:/,/break\;/d' snmpFddiATTACHMENTEntry_perform_set.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmpFddiATTACHMENTEntry_perform_set.c

# Add the modules to the Makefile
	cd $(MOM_BUILD); \
	sed 's/fddi_SRC =/fddi_SRC = fddi_mom_specific.c /; \
	s/snmpFddiSMT_SRC =/snmpFddiSMT_SRC = smt_grp.c/; \
	s/snmpFddiSMTEntry_SRC =/snmpFddiSMTEntry_SRC = smt_entry.c/; \
	s/snmpFddiMAC_SRC =/snmpFddiMAC_SRC = mac_grp.c/; \
	s/snmpFddiMACEntry_SRC =/snmpFddiMACEntry_SRC = mac_entry.c/; \
	s/snmpFddiPORT_SRC =/snmpFddiPORT_SRC = port_grp.c/; \
	s/snmpFddiPORTEntry_SRC =/snmpFddiPORTEntry_SRC = port_entry.c/; \
	s/snmpFddiATTACHMENT_SRC =/snmpFddiATTACHMENT_SRC = attach_grp.c/; \
	s/snmpFddiATTACHMENTEntry_SRC =/snmpFddiATTACHMENTEntry_SRC = attach_entry.c /' Makefile > tmp.$$$$; \
	mv -f tmp.$$$$ Makefile
	cp `genloc /src/usr/sbin/${MOM}_mom/fddi_mom_specific.h`         $(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/fddi_mom_specific.c`          $(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/attach_grp.c`          	$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/attach_entry.c`          	$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/mac_grp.c`          		$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/mac_entry.c`          	$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/smt_grp.c`          		$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/smt_entry.c`          	$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/port_grp.c`          		$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/port_entry.c`          	$(MOM_BUILD)/.
	chmod 644 $(MOM_BUILD)/*.c
	chmod 644 $(MOM_BUILD)/*.h
	
