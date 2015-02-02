/*
 * Copyright (C) 2010 Rodolfo Giometti <giometti@linux.it>
 * Copyright (C) 2010 CAEN RFID <info@caenrfid.it>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation version 2
 *  of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this package; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef _MSGBUFF_H
#define _MSGBUFF_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#include <sys/user.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <arpa/inet.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE			(4 << 10)
#endif

#define MAX_MESSAGE_HEADER_SIZE		(1 << 10)
#define MAX_MESSAGE_DATA_SIZE		(15 << 10)
#define MESSAGE_SIZE			(MAX_MESSAGE_HEADER_SIZE + \
					 MAX_MESSAGE_DATA_SIZE)

/*
 * Misc macros
 */

#define __deprecated		__attribute__ ((deprecated))
#define __packed		__attribute__ ((packed))
#define __constructor		__attribute__ ((constructor))

#define unlikely(x)		__builtin_expect(!!(x), 0)

/*
 * Logging system
 */

#include <syslog.h>

#ifndef DISABLE_DEBUG
#define __msgbuff_message(level, fmt, args...)				\
		syslog(level, "%s[%4d]: " fmt "\n" ,			\
			__FILE__, __LINE__ , ## args)

#define __msgbuff_dbg(fmt, args...)					\
		__msgbuff_message(LOG_DEBUG, fmt , ## args)

#define MSGBUFF_DUMP(code)						\
	do {								\
		if (unlikely(enable_debug)) do {			\
			code						\
		} while (0);						\
	} while (0)

#else  /* !DISABLE_DEBUG */

#define __msgbuff_message(level, fmt, args...)				\
		syslog(level, fmt "\n" , ## args)

#define __msgbuff_dbg(fmt, args...)					\
				/* do nothing! */

#define MSGBUFF_DUMP(code)						\
				/* do nothing! */
#endif /* DISABLE_DEBUG */

#define __msgbuff_info(fmt, args...)					\
		__msgbuff_message(LOG_INFO, fmt , ## args)

#define __msgbuff_err(fmt, args...)					\
		__msgbuff_message(LOG_ERR, fmt , ## args)

/*
 * Exported defines
 *
 * The following defines should be preferred to the above one into
 * normal code.
 */

#ifndef DISABLE_DEBUG
#define msgbuff_info(fmt, args...)					\
		__msgbuff_info("%s: " fmt , __func__ , ## args)
#define msgbuff_err(fmt, args...)					\
		__msgbuff_err("%s: " fmt , __func__ , ## args)
#define msgbuff_dbg(fmt, args...)					\
		__msgbuff_dbg("%s: " fmt , __func__ , ## args)

#else  /* DISABLE_DEBUG */

#define msgbuff_info(args...)	__msgbuff_info(args)
#define msgbuff_err(args...)	__msgbuff_err(args)
#define msgbuff_dbg(args...)	__msgbuff_dbg(args)

#endif /* !DISABLE_DEBUG */

/*
 * Message buffer struct
 */

struct msgbuff_shared_info {
	unsigned int ref;
};

struct msgbuff {
	/* Data part */
	int fd;
	/* uint16_t type; */
	uint32_t id;

	/* Buff management part */
	char *start;
	char *head;
	char *tail;
	char *end;
	char *data;
};

/*
 * Exported functions
 */

#ifndef DISABLE_DEBUG
extern void msgbuff_dump(struct msgbuff *buff, char *label);
#endif

extern void *msgbuff_push_head(struct msgbuff *buff, size_t len);
extern void *msgbuff_pull_head(struct msgbuff *buff, size_t len);
extern void *msgbuff_push_tail(struct msgbuff *buff, size_t len);
extern void *msgbuff_pull_tail(struct msgbuff *buff, size_t len);
extern size_t msgbuff_len(struct msgbuff *buff);
extern struct msgbuff *msgbuff_copy(struct msgbuff *buff);
extern void *msgbuff_queue(struct msgbuff *buff, struct msgbuff *data);
extern struct msgbuff *msgbuff_alloc(size_t len);
extern void msgbuff_free(struct msgbuff *buff);

#endif /* _MSGBUFF_H */
