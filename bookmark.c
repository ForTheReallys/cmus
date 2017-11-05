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

#include "player.h"
#include "cmus.h"
#include "list.h"
#include "bookmark.h"
#include "xmalloc.h"
#include "ui_curses.h"
#include "debug.h"
#include "command_mode.h"
#include "utils.h"
#include "file.h"
#include "misc.h"
#include "prog.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

struct list_head bookmark_head;

LIST_HEAD(bookmark_head);

void bookmark_add(const char *filename, const char *name, int pos)
{
	struct bookmark *mark = bookmark_find_silent(filename, name);

	if (mark != NULL) {
		if (yes_no_query("Bookmark '%s' already exists. Overwrite? [y/N]", name))
			bookmark_remove(filename, name);
		else
			return;
	}

	mark = xnew(struct bookmark, 1);
	mark->filename = xstrdup(filename);
	mark->name = xstrdup(name);
	mark->pos = pos;

	list_add_tail(&mark->node, &bookmark_head);
}

void bookmark_remove(const char *filename, const char *name)
{
	struct bookmark *mark = bookmark_find(filename, name);

	if (mark != NULL){
		list_del(&mark->node);
		free(mark->filename);
		free(mark->name);
		free(mark);
	} else {
		error_msg("Could not find bookmark '%s'", name);
	}
}

void bookmark_play(const char *filename, const char *name)
{
	struct bookmark *bookmark = bookmark_find(filename, name);
	if (bookmark != NULL)
		if (cmus_play_file(filename))
			player_seek(bookmark->pos, 0, 1);
}

struct bookmark *bookmark_find(const char *filename, const char *name)
{
	struct bookmark *mark = bookmark_find_silent(filename, name);
	if (mark == NULL)
		error_msg("Could not find bookmark '%s' for %s", name, filename);
	return mark;
}

struct bookmark *bookmark_find_silent(const char *filename, const char *name)
{
	struct bookmark *mark;
	list_for_each_entry(mark, &bookmark_head, node) {
		if (strcmp(filename, mark->filename) == 0)
			if (strcmp(name, mark->name) == 0)
				return mark;
	}
	return NULL;
}

void bookmark_exit(void)
{
	struct bookmark *mark;
	char filename_tmp[512];
	char filename[512];
	FILE *f;
	int i;

	snprintf(filename_tmp, sizeof(filename_tmp), "%s/bookmarks.tmp", cmus_config_dir);

	f = fopen(filename_tmp, "w");
	if (f == NULL) {
		warn_errno("creating %s", filename_tmp);
		return;
	}

	list_for_each_entry(mark, &bookmark_head, node) {
		fprintf(f, "%s %d %s\n", mark->name, mark->pos, mark->filename);
	}

	fprintf(f, "\n");
	fclose(f);

	snprintf(filename, sizeof(filename), "%s/bookmarks", cmus_config_dir);

	i = rename(filename_tmp, filename);
	if (i)
		warn_errno("renaming %s to %s", filename_tmp, filename);
}

static int handle_bookmark_line(void *data, const char *line)
{
	char *name, *arg;
	char *filename;
	long pos;

	if (!parse_command(line, &name, &arg))
		return 0;

	if (arg == NULL)
		goto out;

	filename = strchr(arg, ' ');
	if (filename == NULL)
		goto out;
	*filename++ = 0;
	while (*filename == ' ')
		filename++;

	if (str_to_int(arg, &pos) == 0)
		bookmark_add(filename, name, pos);

	free(arg);
out:
	free(name);
	return 0;
}

void bookmark_load(void)
{
	char filename[512];
	snprintf(filename, sizeof(filename), "%s/bookmarks", cmus_config_dir);

	if (file_for_each_line(filename, handle_bookmark_line, NULL) == -1) {
		if (errno != ENOENT)
			error_msg("loading %s: %s", filename, strerror(errno));
		return;
	}
}
