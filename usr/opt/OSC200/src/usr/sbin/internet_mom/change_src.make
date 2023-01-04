#
# Makefile for Alpha OSF/1 Internet MOM
#
# Created 2-dec-1992 by Muhammad Ashraf 
# Brought in-line with reality on 01-Sept-1993 by Doug McKenzie
#
MOM		= internet 
MOM_BUILD	= ./

all:	
#
#
#  Create mom_prototypes.h
#
#
	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "man_status atEntry_find_matching_instance PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        avl                 *," >> $(MOM_BUILD)/mom_prototypes.h
	echo "        GET_CONTEXT_STRUCT  *  ));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "" >> $(MOM_BUILD)/mom_prototypes.h

	echo "man_status ipNetToMediaEntry_find_matching_instance PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        avl                 *," >> $(MOM_BUILD)/mom_prototypes.h
	echo "        GET_CONTEXT_STRUCT  *  ));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "" >> $(MOM_BUILD)/mom_prototypes.h

	echo "man_status tcpConnEntry_find_matching_instance PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        avl                 *," >> $(MOM_BUILD)/mom_prototypes.h
	echo "        GET_CONTEXT_STRUCT  *  ));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "" >> $(MOM_BUILD)/mom_prototypes.h

	echo "man_status udpEntry_find_matching_instance PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        avl                 *," >> $(MOM_BUILD)/mom_prototypes.h
	echo "        GET_CONTEXT_STRUCT  *  ));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "man_status   load_system_structures PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        ));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "man_status   refresh_system_list PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        system_DEF *));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "man_status   refresh_interfaces_list PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        interfaces_DEF *));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "man_status   refresh_ifEntry_list PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        ifEntry_DEF *));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "man_status   refresh_at_list PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        at_DEF *));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "man_status   refresh_atEntry_list PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        atEntry_DEF *));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "man_status   refresh_ip_list PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        ip_DEF *));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "man_status   refresh_ipAddrEntry_list PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        ipAddrEntry_DEF *));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "man_status   refresh_ipRouteEntry_list PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        ipRouteEntry_DEF *));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "man_status   refresh_ipNetToMediaEntry_list PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        ipNetToMediaEntry_DEF *));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "man_status   refresh_icmp_list PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        icmp_DEF *));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "man_status   refresh_tcp_list PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        tcp_DEF *));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "man_status   refresh_tcpConnEntry_list PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        tcpConnEntry_DEF *));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "man_status   refresh_udp_list PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        udp_DEF *));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "man_status   refresh_udpEntry_list PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        udpEntry_DEF *));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "man_status   refresh_egp_list PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        egp_DEF *));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "man_status   refresh_egpNeighEntry_list PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        egpNeighEntry_DEF *));" >> $(MOM_BUILD)/mom_prototypes.h

	echo "" >> $(MOM_BUILD)/mom_prototypes.h
	echo "man_status   refresh_snmp_list PROTOTYPE((" >> $(MOM_BUILD)/mom_prototypes.h
	echo "        snmp_DEF *));" >> $(MOM_BUILD)/mom_prototypes.h

#
#
# Insert function call "load_system_structures()" into init.c 
#
#
	cd $(MOM_BUILD); \
	sed 's/\/\*\* MOM specific \#includes go here. \*\*\//\#include \"inet_mom_specific\.h\"/; \
	s/\/\*\* Place call to any general initialization function here. \*\*\//status = \(man_status\) load_system_structures\(\); if ERROR_CONDITION\(status\) return status;/;\
	s/\/\*\* MOM-specific initialization here. \*\*\//         inet_mom_specific_init\(\);/;\
	s/\"rfc1213_mom\"/\"internet_mom\"/;\
	s/global_mom_name = \"rfc1213\"/global_mom_name = \"internet\"/;\
	s/MODIFICATION HISTORY:/MODIFICATION HISTORY: call added to load_system_structures\(\)/' init.c > tmp.$$$$; \
	mv -f tmp.$$$$ init.c
#
#
# CLASS = SYSTEM
#
#    system_perform_getnext.c - insert function call 
	cd $(MOM_BUILD); \
	sed 's/\/\*\* add function call to assure class structure is up-to-date (if necessary) \*\*\//status = \(man_status\) refresh_system_list\(new_system_header\); if ERROR_CONDITION\(status\) return status;/' system_perform_getnext.c > tmp.$$$$; \
	mv -f tmp.$$$$ system_perform_getnext.c

#    system_get_instance.c - insert function call 
	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_system_list\(new_system_header\); if ERROR_CONDITION\(status\) return status;/' system_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ system_get_instance.c

#    system_perform_set.c - insert function call 
#	cd $(MOM_BUILD); \
#	sed -f ../system_perform_set.scr system_perform_set.c > tmp.$$$$; \
#	mv -f tmp.$$$$ system_perform_set.c
#
#
# CLASS = interfaces 
#
#    interfaces_perform_getnext.c - insert function call 
	cd $(MOM_BUILD); \
	sed 's/\/\*\* add function call to assure class structure is up-to-date (if necessary) \*\*\//status = \(man_status\) refresh_if_list\(new_interfaces_header\); if ERROR_CONDITION\(status\) return status;/' interfaces_perform_getnext.c > tmp.$$$$; \
	mv -f tmp.$$$$ interfaces_perform_getnext.c

#    interfaces_get_instance.c - insert function call 
	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_if_list\(new_interfaces_header\); if ERROR_CONDITION\(status\) return status;/' interfaces_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ interfaces_get_instance.c
#
#
# CLASS = ifEntry
#
#    ifEntry_perform_getnext.c - insert function call 
	cd $(MOM_BUILD); \
	sed 's/\/\*\* add function call to assure class structure is up-to-date (if necessary) \*\*\//status = \(man_status\) refresh_ifEntry_list\(new_ifEntry_header\); if ERROR_CONDITION\(status\) return status;/' ifEntry_perform_getnext.c > tmp.$$$$; \
	mv -f tmp.$$$$ ifEntry_perform_getnext.c

#    ifEntry_get_instance.c - insert function call 
	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_ifEntry_list\(new_ifEntry_header\); if ERROR_CONDITION\(status\) return status;/' ifEntry_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ ifEntry_get_instance.c

#    ifEntry_perform_set.c - insert function call 
	cd $(MOM_BUILD); \
	sed '/case ifEntry_ATTR_ifAdminStatus  \:/,/break\;/d' ifEntry_perform_set.c > tmp.$$$$; \
	mv -f tmp.$$$$ ifEntry_perform_set.c
#
#
# CLASS = at 
#
#    at_perform_getnext.c - insert function call 
	cd $(MOM_BUILD); \
	sed 's/\/\*\* add function call to assure class structure is up-to-date (if necessary) \*\*\//status = \(man_status\) refresh_at_list\(new_at_header\); if ERROR_CONDITION\(status\) return status;/' at_perform_getnext.c > tmp.$$$$; \
	mv -f tmp.$$$$ at_perform_getnext.c

#    at_get_instance.c - insert function call 
	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_at_list\(new_at_header\); if ERROR_CONDITION\(status\) return status;/' at_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ at_get_instance.c
#
#
# CLASS = atEntry
#
#    atEntry_perform_set.c - insert function call 
	cd $(MOM_BUILD); \
	sed '/case atEntry_ATTR_atPhysAddress  \:/,/break\;/d' atEntry_perform_set.c > tmp.$$$$; \
	mv -f tmp.$$$$ atEntry_perform_set.c
#
#
# CLASS = ip 
#
#    ip_get_instance.c - insert function call 
	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_ip_list\(new_ip_header\); if ERROR_CONDITION\(status\) return status;/' ip_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ ip_get_instance.c

#    ip_perform_set.c - insert function call 
	cd $(MOM_BUILD); \
	sed '/case ip_ATTR_ipForwarding  \:/,/break\;/d' ip_perform_set.c > tmp.$$$$; \
	mv -f tmp.$$$$ ip_perform_set.c
#
#
# CLASS = ipRouteEntry
#
#    ipRouteEntry_perform_set.c - insert function call 
	cd $(MOM_BUILD); \
	sed '/case ipRouteEntry_ATTR_ipRouteIfIndex  \:/,/break\;/d' ipRouteEntry_perform_set.c > tmp.$$$$; \
	mv -f tmp.$$$$ ipRouteEntry_perform_set.c
#
#
# CLASS = ipNetToMediaEntry
#
#    ipNetToMediaEntry_perform_set.c - insert function call 
	cd $(MOM_BUILD); \
	sed '/case ipNetToMediaEntry_ATTR_ipNetToMediaPhysAddress  \:/,/break\;/d' ipNetToMediaEntry_perform_set.c > tmp.$$$$; \
	mv -f tmp.$$$$ ipNetToMediaEntry_perform_set.c
#
#
# CLASS = icmp
#
#    icmp_perform_getnext.c - insert function call 
	cd $(MOM_BUILD); \
	sed 's/\/\*\* add function call to assure class structure is up-to-date (if necessary) \*\*\//status = \(man_status\) refresh_icmp_list\(new_icmp_header\); if ERROR_CONDITION\(status\) return status;/' icmp_perform_getnext.c > tmp.$$$$; \
	mv -f tmp.$$$$ icmp_perform_getnext.c

#    icmp_get_instance.c - insert function call 
	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_icmp_list\(new_icmp_header\); if ERROR_CONDITION\(status\) return status;/' icmp_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ icmp_get_instance.c
#
#
# CLASS = tcp
#
#    tcp_get_instance.c - insert function call 
	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_tcp_list\(new_tcp_header\); if ERROR_CONDITION\(status\) return status;/' tcp_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ tcp_get_instance.c
#
#
# CLASS = tcpConnEntry
#
#    tcpConnEntry_perform_set.c - insert function call 
	cd $(MOM_BUILD); \
	sed '/case tcpConnEntry_ATTR_tcpConnState  \:/,/break\;/d' tcpConnEntry_perform_set.c > tmp.$$$$; \
	mv -f tmp.$$$$ tcpConnEntry_perform_set.c
#
#
# CLASS = udp
#
#    udp_perform_getnext.c - insert function call 
	cd $(MOM_BUILD); \
	sed 's/\/\*\* add function call to assure class structure is up-to-date (if necessary) \*\*\//status = \(man_status\) refresh_udp_list\(new_udp_header\); if ERROR_CONDITION\(status\) return status;/' udp_perform_getnext.c > tmp.$$$$; \
	mv -f tmp.$$$$ udp_perform_getnext.c

#    udp_get_instance.c - insert function call 
	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_udp_list\(new_udp_header\); if ERROR_CONDITION\(status\) return status;/' udp_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ udp_get_instance.c
#
#
# CLASS =  egp
#
#    egp_get_instance.c - insert function call 
	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_egp_list\(new_egp_header\); if ERROR_CONDITION\(status\) return status;/' egp_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ egp_get_instance.c
#
#
# CLASS = egpNeighEntry
#
#    egpNeighEntry_perform_set.c - insert function call 
	cd $(MOM_BUILD); \
	sed '/case egpNeighEntry_ATTR_egpNeighEventTrigger  \:/,/break\;/d' egpNeighEntry_perform_set.c > tmp.$$$$; \
	mv -f tmp.$$$$ egpNeighEntry_perform_set.c
#
#
# CLASS = snmp
#
#    snmp_perform_getnext.c - insert function call 
	cd $(MOM_BUILD); \
	sed 's/\/\*\* add function call to assure class structure is up-to-date (if necessary) \*\*\//status = \(man_status\) refresh_snmp_list\(new_snmp_header\); if ERROR_CONDITION\(status\) return status;/' snmp_perform_getnext.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmp_perform_getnext.c

#    snmp_get_instance.c.c - insert function call 
	cd $(MOM_BUILD); \
	sed 's/\/\*\* Remove this if not using the queue instance header. \*\*\//status = \(man_status\) refresh_snmp_list\(new_snmp_header\); if ERROR_CONDITION\(status\) return status;/' snmp_get_instance.c > tmp.$$$$; \
	mv -f tmp.$$$$ snmp_get_instance.c; \

#    snmp_perform_set.c - insert function call 
#	cd $(MOM_BUILD); \
#	sed -f ../snmp_perform_set.scr snmp_perform_set.c > tmp.$$$$; \
#	mv -f tmp.$$$$ snmp_perform_set.c

#
#
#    Add developer's code to the Makefile
#
#
	cd $(MOM_BUILD); \
	sed 's/system_SRC =/system_SRC = system_load_structures.c inet_mom_specific.c /; \
		s/interfaces_SRC =/interfaces_SRC = if_grp.c/; \
		s/ifEntry_SRC =/ifEntry_SRC = if_entry.c/; \
		s/at_SRC =/at_SRC = at_grp.c/; \
		s/atEntry_SRC =/atEntry_SRC = at_entry.c/; \
		s/ip_SRC =/ip_SRC = ip_grp.c/; \
		s/ipAddrEntry_SRC =/ipAddrEntry_SRC = ip_addrentry.c/; \
		s/ipRouteEntry_SRC =/ipRouteEntry_SRC = ip_routeentry.c/; \
		s/ipNetToMediaEntry_SRC =/ipNetToMediaEntry_SRC = ip_netentry.c/; \
		s/icmp_SRC =/icmp_SRC = icmp_grp.c/; \
		s/tcp_SRC =/tcp_SRC = tcp_grp.c/; \
		s/tcpConnEntry_SRC =/tcpConnEntry_SRC = tcp_connentry.c/; \
		s/udp_SRC =/udp_SRC = udp_grp.c/; \
		s/udpEntry_SRC =/udpEntry_SRC = udp_entry.c/; \
		s/egp_SRC =/egp_SRC = egp_grp.c/; \
		s/egpNeighEntry_SRC =/egpNeighEntry_SRC = egp_entry.c/; \
		s/snmp_SRC =/snmp_SRC = snmp_grp.c /' Makefile > tmp.$$$$; \
	mv -f tmp.$$$$ Makefile

#
#
#    Copy in the sources that MUST override the MOMgen-generated sources
#       (Customizations required to make rolling work OK, among other things)
#
#
	cp `genloc /src/usr/sbin/${MOM}_mom/inet_mom_specific.h`  		$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/inet_mom_specific.c`  		$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/system_load_structures.c` 		$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/if_grp.c` 				$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/if_entry.c` 				$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/at_grp.c`  				$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/at_entry.c`  			$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/ip_grp.c` 				$(MOM_BUILD)/. 
	cp `genloc /src/usr/sbin/${MOM}_mom/ip_addrentry.c` 			$(MOM_BUILD)/. 
	cp `genloc /src/usr/sbin/${MOM}_mom/ip_routeentry.c` 			$(MOM_BUILD)/. 
	cp `genloc /src/usr/sbin/${MOM}_mom/ip_netentry.c` 			$(MOM_BUILD)/. 
	cp `genloc /src/usr/sbin/${MOM}_mom/icmp_grp.c` 				$(MOM_BUILD)/. 
	cp `genloc /src/usr/sbin/${MOM}_mom/tcp_grp.c` 				$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/tcp_connentry.c` 			$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/udp_grp.c` 				$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/udp_entry.c` 			$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/egp_grp.c` 				$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/egp_entry.c` 			$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/snmp_grp.c` 				$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/snmp_perform_init.c`			$(MOM_BUILD)/.
	cp `genloc /src/usr/sbin/${MOM}_mom/atentry_get_instance.c` 		$(MOM_BUILD)/atEntry_get_instance.c
	cp `genloc /src/usr/sbin/${MOM}_mom/atentry_perform_get.c` 		$(MOM_BUILD)/atEntry_perform_get.c
	cp `genloc /src/usr/sbin/${MOM}_mom/atentry_perform_getnext.c`	 	$(MOM_BUILD)/atEntry_perform_getnext.c
	cp `genloc /src/usr/sbin/${MOM}_mom/egp_perform_getnext.c`		$(MOM_BUILD)/egp_perform_getnext.c ;\
	cp `genloc /src/usr/sbin/${MOM}_mom/egpneighentry_get_instance.c`		$(MOM_BUILD)/egpNeighEntry_get_instance.c
	cp `genloc /src/usr/sbin/${MOM}_mom/egpneighentry_perform_get.c`		$(MOM_BUILD)/egpNeighEntry_perform_get.c
	cp `genloc /src/usr/sbin/${MOM}_mom/egpneighentry_perform_getnext.c`	$(MOM_BUILD)/egpNeighEntry_perform_getnext.c
	cp `genloc /src/usr/sbin/${MOM}_mom/ip_perform_getnext.c`			$(MOM_BUILD)/ip_perform_getnext.c
	cp `genloc /src/usr/sbin/${MOM}_mom/ipaddrentry_get_instance.c`		$(MOM_BUILD)/ipAddrEntry_get_instance.c
	cp `genloc /src/usr/sbin/${MOM}_mom/ipaddrentry_perform_get.c`		$(MOM_BUILD)/ipAddrEntry_perform_get.c
	cp `genloc /src/usr/sbin/${MOM}_mom/ipaddrentry_perform_getnext.c`	$(MOM_BUILD)/ipAddrEntry_perform_getnext.c
	cp `genloc /src/usr/sbin/${MOM}_mom/ipnettomediaentry_get_instance.c`	$(MOM_BUILD)/ipNetToMediaEntry_get_instance.c
	cp `genloc /src/usr/sbin/${MOM}_mom/ipnettomediaentry_perform_get.c`	$(MOM_BUILD)/ipNetToMediaEntry_perform_get.c
	cp `genloc /src/usr/sbin/${MOM}_mom/ipnettomediaentry_perform_getnext.c` 	$(MOM_BUILD)/ipNetToMediaEntry_perform_getnext.c
	cp `genloc /src/usr/sbin/${MOM}_mom/iprouteentry_get_instance.c`		$(MOM_BUILD)/ipRouteEntry_get_instance.c
	cp `genloc /src/usr/sbin/${MOM}_mom/iprouteentry_perform_get.c`		$(MOM_BUILD)/ipRouteEntry_perform_get.c
	cp `genloc /src/usr/sbin/${MOM}_mom/iprouteentry_perform_getnext.c`	$(MOM_BUILD)/ipRouteEntry_perform_getnext.c
	cp `genloc /src/usr/sbin/${MOM}_mom/tcp_perform_getnext.c`		$(MOM_BUILD)/tcp_perform_getnext.c ;\
	cp `genloc /src/usr/sbin/${MOM}_mom/tcpconnentry_get_instance.c`		$(MOM_BUILD)/tcpConnEntry_get_instance.c
	cp `genloc /src/usr/sbin/${MOM}_mom/tcpconnentry_perform_get.c`		$(MOM_BUILD)/tcpConnEntry_perform_get.c
	cp `genloc /src/usr/sbin/${MOM}_mom/tcpconnentry_perform_getnext.c`	$(MOM_BUILD)/tcpConnEntry_perform_getnext.c
	cp `genloc /src/usr/sbin/${MOM}_mom/udpentry_get_instance.c`		$(MOM_BUILD)/udpEntry_get_instance.c
	cp `genloc /src/usr/sbin/${MOM}_mom/udpentry_perform_get.c`		$(MOM_BUILD)/udpEntry_perform_get.c
	cp `genloc /src/usr/sbin/${MOM}_mom/udpentry_perform_getnext.c`		$(MOM_BUILD)/udpEntry_perform_getnext.c
	cp `genloc /src/usr/sbin/${MOM}_mom/udpentry_find_instance.c`		$(MOM_BUILD)/udpEntry_find_instance.c
	chmod 644 $(MOM_BUILD)/*.c
	chmod 644 $(MOM_BUILD)/*.h
