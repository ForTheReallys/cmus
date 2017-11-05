/*
 * Copyright 2008-2013 Various Authors
 * Copyright 2004 Timo Hirvonen
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BOOKMARK_H
#define BOOKMARK_H

#include "list.h"

struct bookmark {
	char *name;
	char *filename;
	struct list_head node;
	int pos;                 /* In seconds */
};

extern struct list_head bookmark_head;

void bookmark_add(const char *filename, const char *name, int pos);
void bookmark_remove(const char *filename, const char *name);
void bookmark_play(const char *filename, const char *name);

void bookmark_exit(void);
void bookmark_load(void);

struct bookmark *bookmark_find(const char *filename, const char *name);
struct bookmark *bookmark_find_silent(const char *filename, const char *name);

#endif
