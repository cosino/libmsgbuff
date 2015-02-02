#
# Regular cron jobs for the libmsgbuff package
#
0 4	* * *	root	[ -x /usr/bin/libmsgbuff_maintenance ] && /usr/bin/libmsgbuff_maintenance
