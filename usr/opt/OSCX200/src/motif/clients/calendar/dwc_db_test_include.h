#ifdef VMS
#define Show_stat {\
		    int DWC$$DB_DUMP_JOB_INFO(); \
		    DWC$$DB_DUMP_JOB_INFO(); \
		  }
#else
#define Show_stat
#endif
