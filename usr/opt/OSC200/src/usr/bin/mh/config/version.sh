: run this script through /bin/sh
: '@(#)$Id: version.sh,v 4.1 1991/09/18 21:58:31 devrcs Exp $'
case $# in
  0) set "6.6";;
esac

OFS="$IFS" IFS=:

for A in rprompt hostname uname who
do
    for D in $PATH
    do
	if [ ! -f $D/$A ]; then
	    continue
	fi
	case $A in
	    rprompt)    LOCAL=`$A %h`
			;;
	    hostname)   LOCAL=`$A`
			;;
	    uname)	LOCAL=`$A -n`
			;;
	    who)	LOCAL=`$A am i | sed -e 's%^\(.*\)!.*$%\1%'`
			;;
	esac
	break
    done
    if [ "x$LOCAL" != x ]; then
	break
    fi
done

IFS=" "
Patch="../Patchlevel"

if [ ! -r version  ]; then echo "0" > version; fi
if [ ! -r ${Patch} ]; then Patch="" ; fi

rm -f version.c

cat version ${Patch} |\
awk -F. '
BEGIN	{ split ("'"${1}"'", ver, ".");\
	  major = ver[1]; minor = ver[2]; patch = ver[3]}\
NR == 1 { local = $(NF) }\
NR == 2 { patch = $(NF) }\
END	{ local++;\
	  if (patch > 0) {\
	  printf "char *version = \"MH %s.%s.%s #%d[UCI] ", \
		major, minor, patch, local > "version.c"; \
	  printf ".ds MH %s.%s.%s #%d[UCI]\n", \
		major, minor, patch, local > "version.rf"; \
	  printf "MH %s.%s.%s #%d[UCI]\n", major, minor, patch, local ;\
	} else {\
	  printf "char *version = \"MH %s.%s #%d[UCI] ", \
		major, minor, local > "version.c"; \
	  printf ".ds MH %s.%s #%d[UCI]\n", \
		major, minor, local > "version.rf"; \
	  printf "MH %s.%s #%d[UCI]\n", major, minor, local ;\
	}\
	  printf "%s.%s.%s.%d\n", major, minor, patch, local > "_version";\
}' && mv _version version
echo '('$LOCAL') of '`date`'";' >> version.c
