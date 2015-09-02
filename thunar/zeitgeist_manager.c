/* $Id$ */
/*-
 * Copyright (c) 2015 Steve Dodier-Lazaro <sidi@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "zeitgeist_manager.h"

#include <glib.h>
#include <unistd.h>
#include <time.h>

ZeitgeistSubject *zeitgeist_get_ucl_subject (const pid_t pid)
{
    ZeitgeistSubject *subject = zeitgeist_subject_new ();

    gchar *pid_str = pid >= 0? g_strdup_printf ("%d", pid) : g_strdup ("n/a");

    gchar *study_uri = g_strdup_printf ("activity://null///pid://%s///winid://n/a///", pid_str);
    free (pid_str);

    zeitgeist_subject_set_uri (subject, study_uri);
    free (study_uri);

    zeitgeist_subject_set_interpretation (subject, ZEITGEIST_NFO_SOFTWARE);
    zeitgeist_subject_set_manifestation (subject, ZEITGEIST_ZG_WORLD_ACTIVITY);
    zeitgeist_subject_set_mimetype (subject, "application/octet-stream");
    zeitgeist_subject_set_text (subject, "ucl-study-metadata");

    return subject;
}

char *zeitgeist_get_actor_name_from_pid (const pid_t pid)
{
    if (pid <= 0)
        return NULL;

    char *link_file = g_strdup_printf ("/proc/%d/exe", pid);
    if (!link_file)
        return NULL;

    char buff[PATH_MAX+1];
    ssize_t len = readlink(link_file, buff, sizeof(buff)-1);
    g_free (link_file);
    if (len < 0)
        return NULL;

    buff[len] = '\0';
    gchar *split = strrchr (buff, '/');

    if (!split)
        return NULL;

    gchar *actor_name = g_strdup_printf ("application://%s.desktop", split+1);
    return actor_name;
}


