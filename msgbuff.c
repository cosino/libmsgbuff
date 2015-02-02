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

#include "msgbuff.h"

#define HEADER_MIN_SIZE		PAGE_SIZE

/*
 * Local functions
 */

static void *msgbuff_head_ptr(struct msgbuff *buff)
{
	return buff->head;
}

static void *msgbuff_tail_ptr(struct msgbuff *buff)
{
	return buff->tail;
}

/*
 * Exported functions
 */

#ifndef DISABLE_DEBUG
void msgbuff_dump(struct msgbuff *buff, char *label)
{
	unsigned char *data;
	int len;

	int i, p;
	char num[3], line[81];

	data = msgbuff_head_ptr(buff);
	len = msgbuff_len(buff);

	msgbuff_dbg("%s", label);

	p = 0;
	memset(line, ' ', 80);
	for (i = 0; i < len; i++) {
		sprintf(num, "%02x", data[i]);
		memcpy(&line[p * 3], num, 2);
		line[50 + p] = isprint(data[i]) ? data[i] : '.';

		p++;
		if (p == 16) {
			line[80] = '\0';
			msgbuff_dbg("%s", line);
			p = 0;
			memset(line, ' ', 80);
		}
	}
	if (p != 0) {
		line[80] = '\0';
		msgbuff_dbg("%s", line);
	}
}
#endif

void *msgbuff_push_head(struct msgbuff *buff, size_t len)
{
	int r;
	char *old;

	r = buff->start - (buff->head - len);
	if (r > 0)
		return NULL;

	old = buff->head;

	buff->head -= len;

	return buff->head;
}

void *msgbuff_pull_head(struct msgbuff *buff, size_t len)
{
	char *old;

	if (buff->head + len > buff->tail)
		return NULL;

	old = buff->head;
	buff->head += len;

	return old;
}

void *msgbuff_push_tail(struct msgbuff *buff, size_t len)
{
	int r, nbytes;
	char *o_data, *o_start;
	char *old;

	r = (buff->tail + len) - buff->end;
	if (r > 0) {
		nbytes = ((buff->end - buff->start) + r + PAGE_SIZE) &
		    (~PAGE_SIZE);
		o_start = buff->start;

		o_data = realloc(buff->data, nbytes);
		if (!o_data)
			return NULL;
		buff->data = o_data;

		buff->start = buff->data;
		buff->end = buff->start + nbytes;
		buff->head = (buff->head - o_start) + buff->data;
		buff->tail = (buff->tail - o_start) + buff->data;
	}

	old = buff->tail;

	buff->tail += len;

	return old;
}

void *msgbuff_pull_tail(struct msgbuff *buff, size_t len)
{
	int r;
	char *old;

	r = buff->start - (buff->tail - len);
	if (r > 0)
		return NULL;

	old = buff->tail;

	buff->tail -= len;

	return old;
}

size_t msgbuff_len(struct msgbuff *buff)
{
	return msgbuff_tail_ptr(buff) - msgbuff_head_ptr(buff);
}

struct msgbuff *msgbuff_copy(struct msgbuff *buff)
{
	struct msgbuff *cbuff;
	size_t size;
	char *ptr;

	cbuff = msgbuff_alloc(PAGE_SIZE);
	if (!cbuff)
		return NULL;

	/* Copy the header */
	cbuff->fd = buff->fd;
	cbuff->id = buff->id;

	/* Copy the buffer */
	size = msgbuff_len(buff);
	ptr = msgbuff_push_tail(cbuff, size);
	if (!ptr) {
		msgbuff_free(cbuff);
		return NULL;
	}
	memcpy(ptr, msgbuff_head_ptr(buff), size);

	return cbuff;
}

void *msgbuff_queue(struct msgbuff *buff, struct msgbuff *data)
{
	char *ptr;
	size_t len;

	len = msgbuff_len(data);
	ptr = msgbuff_push_tail(buff, len);
	if (!ptr)
		return NULL;

	return memcpy(ptr, msgbuff_head_ptr(data), len);
}

struct msgbuff *msgbuff_alloc(size_t len)
{
	struct msgbuff *buff;
	int nbytes;

	/* Force allocating at least HEADER_MIN_SIZE bytes for the header */
	nbytes = len + HEADER_MIN_SIZE;

	/* Allocate the msgbuff */
	buff = malloc(sizeof(struct msgbuff));
	if (!buff)
		goto exit_error;

	/* Allocate the msgbuf's data */
	buff->data = realloc(NULL, nbytes);
	if (!buff->data)
		goto free_buff;

	buff->start = buff->data;
	buff->end = buff->start + nbytes;
	buff->head = buff->data + HEADER_MIN_SIZE;
	buff->tail = buff->head;

	return buff;

free_buff:
	free(buff);
exit_error:
	return NULL;
}

void msgbuff_free(struct msgbuff *buff)
{
	char *data;

	if (!buff)
		return;
	data = buff->data;

	/* The struct msgbuff can be freed now */
	free(buff);

	free(data);
}
