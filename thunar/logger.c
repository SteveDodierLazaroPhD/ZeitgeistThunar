/*      $Id$

        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2, or (at your option)
        any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., Inc., 51 Franklin Street, Fifth Floor, Boston,
        MA 02110-1301, USA.


        oroborus - (c) 2001 Ken Lynch
        xfwm4    - (c) 2002-2015 Olivier Fourdan
        xfwm4-zg - (c) 2015 Steve Dodier-Lazaro

 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>

#include <glib.h>
#include <sys/time.h>
#include <time.h>
#include <zeitgeist.h>

#include "logger.h"
#include "thunar-file.h"
#include "zeitgeist_manager.h"



static void log_debug_cb (GObject *source_object,
                           GAsyncResult *res,
                           gpointer user_data)
{
    fprintf (stderr, "Debugging callback\n");
    ZeitgeistLog *log = user_data;

    GError *error = NULL;
    zeitgeist_log_insert_events_finish (log, res, &error);
    if (error)
    {
      g_error ("Impossible to log event: %s ", error->message);
      g_error_free (error);
    }
}

void log_rename (ThunarFile *file, GFile *old, GFile *new)
{
    if(!file || !old || !new) return;

    gchar *old_uri = g_file_get_uri (old);
    gchar *new_uri = g_file_get_uri (new);

    // Send event to Zeitgeist
		gchar *origin = g_path_get_dirname (old_uri);
		ZeitgeistSubject *subject = zeitgeist_subject_new_full (
			old_uri,
			NULL, // subject interpretation - auto-guess
			NULL, // subject manifestation - auto-guess
			NULL, // content type - auto-guess
			origin,
			thunar_file_get_display_name (file),
			NULL // storage - auto-guess
		);
		g_free (origin);
		zeitgeist_subject_set_current_uri (subject, new_uri);

		ZeitgeistEvent *event = zeitgeist_event_new_full (
			ZEITGEIST_ZG_MOVE_EVENT,
			ZEITGEIST_ZG_USER_ACTIVITY,
			ZEITGEIST_THUNAR_ACTOR,
			subject, NULL);

		// Add the UCL study metadata object
		char *study_uri = g_strdup_printf("activity://null///pid://%d///winid://n/a///", getpid());
		subject = zeitgeist_subject_new_full(study_uri,
			ZEITGEIST_FILE_MANAGING_ACTOR,
			ZEITGEIST_ZG_WORLD_ACTIVITY,
			"application/octet-stream",
			NULL,
			"ucl-study-metadata",
			NULL);
		g_free(study_uri);
		zeitgeist_subject_set_current_uri (subject, new_uri);
		zeitgeist_event_add_subject (event, subject);

    g_free (old_uri);
    g_free (new_uri);

    GError *error = NULL;
    ZeitgeistLog *log = zeitgeist_log_get_default ();
    zeitgeist_log_insert_events_no_reply (log, event, NULL, &error);

    if (error)
    {
      g_warning ("Impossible to log event for file renaming (%s): %s ", thunar_file_get_display_name (file), error->message);
      g_error_free (error);
    }
}

void log_open_files (GAppInfo *app_info, GList *path_list)
{
    if(!path_list) return;

		ZeitgeistEvent *event = zeitgeist_event_new_full (
			ZEITGEIST_THUNAR_APP_LAUNCH,
			ZEITGEIST_ZG_USER_ACTIVITY,
			ZEITGEIST_THUNAR_ACTOR,
			NULL, NULL);
		ZeitgeistSubject *subject = NULL;

    GList *lp;
    for (lp = path_list; lp != NULL; lp = lp->next)
    {
        GFile *file = lp->data;
        gchar *path = g_file_get_uri (file);
        gchar *base = g_file_get_basename (file);
		    gchar *origin = g_path_get_dirname (path);
		    subject = zeitgeist_subject_new_full (
			    path,
			    NULL, // subject interpretation - auto-guess
			    NULL, // subject manifestation - auto-guess
			    NULL, // content type - auto-guess
			    origin,
			    base,
			    NULL // storage - auto-guess
		    );
		    g_free (origin);
		    g_free (base);
		    g_free (path);
		    zeitgeist_event_add_subject (event, subject);
    }

    // Add the handler, if known
    if (app_info)
    {
  		gchar *target_actor = g_strdup_printf ("application://%s", g_app_info_get_id G_APP_INFO (app_info));
		  subject = zeitgeist_subject_new_full(target_actor,
			  ZEITGEIST_NFO_SOFTWARE,
			  ZEITGEIST_NFO_SOFTWARE_ITEM,
			"application/x-desktop",
			  NULL,
			  g_app_info_get_display_name (app_info),
			  NULL);
		  g_free (target_actor);
		  zeitgeist_event_add_subject (event, subject);
    }

		// Add the UCL study metadata object
		char *study_uri = g_strdup_printf("activity://null///pid://%d///winid://n/a///", getpid());
		subject = zeitgeist_subject_new_full(study_uri,
			ZEITGEIST_FILE_MANAGING_ACTOR,
			ZEITGEIST_ZG_WORLD_ACTIVITY,
			"application/octet-stream",
			NULL,
			"ucl-study-metadata",
			NULL);
		g_free(study_uri);
		zeitgeist_event_add_subject (event, subject);

    GError *error = NULL;
    ZeitgeistLog *log = zeitgeist_log_get_default ();
    zeitgeist_log_insert_events_no_reply (log, event, NULL, &error);

    if (error)
    {
      g_warning ("Impossible to log event for file opening: %s ", error->message);
      g_error_free (error);
    }
}

void log_execute (ThunarFile *target, GFile *origin_file, GList *params)
{
    if(!target) return;

		ZeitgeistEvent *event = zeitgeist_event_new_full (
			ZEITGEIST_THUNAR_FILE_EXEC,
			ZEITGEIST_ZG_USER_ACTIVITY,
			ZEITGEIST_THUNAR_ACTOR,
			NULL, NULL);
		ZeitgeistSubject *subject = NULL;

    gchar *uri = thunar_file_dup_uri (target);
    gchar *origin = origin_file? g_file_get_path (origin_file) : NULL;
	  subject = zeitgeist_subject_new_full(uri,
		  ZEITGEIST_NFO_SOFTWARE,
		  ZEITGEIST_NFO_SOFTWARE_ITEM,
		  NULL,
		  origin,
		  NULL,
		  NULL);
	  g_free (uri);
	  if (origin) g_free (origin);
	  zeitgeist_event_add_subject (event, subject);

    GList *lp;
    for (lp = params; lp != NULL; lp = lp->next)
    {
        GFile *file = lp->data;
        gchar *path = g_file_get_uri (file);
        gchar *base = g_file_get_basename (file);
		    gchar *origin = g_path_get_dirname (path);
		    subject = zeitgeist_subject_new_full (
			    path,
			    NULL, // subject interpretation - auto-guess
			    NULL, // subject manifestation - auto-guess
			    NULL, // content type - auto-guess
			    origin,
			    base,
			    NULL // storage - auto-guess
		    );
		    g_free (origin);
		    g_free (base);
		    g_free (path);
		    zeitgeist_event_add_subject (event, subject);
    }

		// Add the UCL study metadata object
		char *study_uri = g_strdup_printf("activity://null///pid://%d///winid://n/a///", getpid());
		subject = zeitgeist_subject_new_full(study_uri,
			ZEITGEIST_FILE_MANAGING_ACTOR,
			ZEITGEIST_ZG_WORLD_ACTIVITY,
			"application/octet-stream",
			NULL,
			"ucl-study-metadata",
			NULL);
		g_free(study_uri);
		zeitgeist_event_add_subject (event, subject);

    GError *error = NULL;
    ZeitgeistLog *log = zeitgeist_log_get_default ();
    zeitgeist_log_insert_events_no_reply (log, event, NULL, &error);

    if (error)
    {
      g_warning ("Impossible to log event for file execution: %s ", error->message);
      g_error_free (error);
    }
}

void log_insecure_dialog (ThunarFile *target, int response)
{
    if(!target) return;

		ZeitgeistEvent *event = zeitgeist_event_new_full (
			response == GTK_RESPONSE_OK ? ZEITGEIST_THUNAR_FILE_EXEC_INSECURE_LAUNCH :
			  response == GTK_RESPONSE_APPLY ? ZEITGEIST_THUNAR_FILE_EXEC_INSECURE_MARK_EXEC :
			  ZEITGEIST_THUNAR_FILE_EXEC_INSECURE_DENY,
			ZEITGEIST_ZG_USER_ACTIVITY,
			ZEITGEIST_THUNAR_ACTOR,
			NULL, NULL);
		ZeitgeistSubject *subject = NULL;

    gchar *uri = thunar_file_dup_uri (target);
    gchar *origin = g_path_get_dirname (uri);
	  subject = zeitgeist_subject_new_full(uri,
		  ZEITGEIST_NFO_SOFTWARE,
		  ZEITGEIST_NFO_SOFTWARE_ITEM,
		  NULL,
		  origin,
			thunar_file_get_display_name (target),
		  NULL);
	  g_free (uri);
	  g_free (origin);
	  zeitgeist_event_add_subject (event, subject);

		// Add the UCL study metadata object
		char *study_uri = g_strdup_printf("activity://null///pid://%d///winid://n/a///", getpid());
		subject = zeitgeist_subject_new_full(study_uri,
			ZEITGEIST_FILE_MANAGING_ACTOR,
			ZEITGEIST_ZG_WORLD_ACTIVITY,
			"application/octet-stream",
			NULL,
			"ucl-study-metadata",
			NULL);
		g_free(study_uri);
		zeitgeist_event_add_subject (event, subject);

    GError *error = NULL;
    ZeitgeistLog *log = zeitgeist_log_get_default ();
    zeitgeist_log_insert_events_no_reply (log, event, NULL, &error);

    if (error)
    {
      g_warning ("Impossible to log event for file insecure dialog: %s ", error->message);
      g_error_free (error);
    }
}

void log_create (GList *files)
{
    fprintf (stderr, "creating documents %d\n", g_list_length(files));
    if(!files) return;

		ZeitgeistEvent *event = zeitgeist_event_new_full (
			ZEITGEIST_ZG_CREATE_EVENT,
			ZEITGEIST_ZG_USER_ACTIVITY,
			ZEITGEIST_THUNAR_ACTOR,
			NULL, NULL);
		ZeitgeistSubject *subject = NULL;

    GList *lp;
    for (lp = files; lp != NULL; lp = lp->next)
    {
        GFile *file = lp->data;
        gchar *path = g_file_get_uri (file);
		    gchar *origin = g_path_get_dirname (path);
		    subject = zeitgeist_subject_new_full (
			    path,
			    ZEITGEIST_NFO_DOCUMENT, // subject interpretation - auto-guess
			    NULL, // subject manifestation - auto-guess
			    NULL, // content type - auto-guess
			    origin,
			    NULL,
			    NULL // storage - auto-guess
		    );
		    g_free (origin);
		    g_free (path);
		    zeitgeist_event_add_subject (event, subject);
    }

		// Add the UCL study metadata object
		char *study_uri = g_strdup_printf("activity://null///pid://%d///winid://n/a///", getpid());
		subject = zeitgeist_subject_new_full(study_uri,
			ZEITGEIST_FILE_MANAGING_ACTOR,
			ZEITGEIST_ZG_WORLD_ACTIVITY,
			"application/octet-stream",
			NULL,
			"ucl-study-metadata",
			NULL);
		g_free(study_uri);
		zeitgeist_event_add_subject (event, subject);

    GError *error = NULL;
    ZeitgeistLog *log = zeitgeist_log_get_default ();
    zeitgeist_log_insert_events_no_reply (log, event, NULL, &error);

    if (error)
    {
      g_warning ("Impossible to log event for file creation: %s ", error->message);
      g_error_free (error);
    }
}

void log_mkdir (GList *files)
{
    fprintf (stderr, "creating folders %d\n", g_list_length(files));
    if(!files) return;

		ZeitgeistEvent *event = zeitgeist_event_new_full (
			ZEITGEIST_ZG_CREATE_EVENT,
			ZEITGEIST_ZG_USER_ACTIVITY,
			ZEITGEIST_THUNAR_ACTOR,
			NULL, NULL);
		ZeitgeistSubject *subject = NULL;

    GList *lp;
    for (lp = files; lp != NULL; lp = lp->next)
    {
        GFile *file = lp->data;
        gchar *path = g_file_get_uri (file);
		    gchar *origin = g_path_get_dirname (path);
		    subject = zeitgeist_subject_new_full (
			    path,
			    ZEITGEIST_NFO_FOLDER, // subject interpretation - auto-guess
			    NULL, // subject manifestation - auto-guess
			    NULL, // content type - auto-guess
			    origin,
			    NULL,
			    NULL // storage - auto-guess
		    );
		    g_free (origin);
		    g_free (path);
		    zeitgeist_event_add_subject (event, subject);
    }

		// Add the UCL study metadata object
		char *study_uri = g_strdup_printf("activity://null///pid://%d///winid://n/a///", getpid());
		subject = zeitgeist_subject_new_full(study_uri,
			ZEITGEIST_FILE_MANAGING_ACTOR,
			ZEITGEIST_ZG_WORLD_ACTIVITY,
			"application/octet-stream",
			NULL,
			"ucl-study-metadata",
			NULL);
		g_free(study_uri);
		zeitgeist_event_add_subject (event, subject);

    GError *error = NULL;
    ZeitgeistLog *log = zeitgeist_log_get_default ();
    zeitgeist_log_insert_events_no_reply (log, event, NULL, &error);

    if (error)
    {
      g_warning ("Impossible to log event for folder creation: %s ", error->message);
      g_error_free (error);
    }
}

void log_trash (GList *files)
{
    fprintf (stderr, "trashing files %d\n", g_list_length(files));
    if(!files) return;

		ZeitgeistEvent *event = zeitgeist_event_new_full (
      ZEITGEIST_ZG_TRASH_EVENT,
			ZEITGEIST_ZG_USER_ACTIVITY,
			ZEITGEIST_THUNAR_ACTOR,
			NULL, NULL);
		ZeitgeistSubject *subject = NULL;

    GList *lp;
    for (lp = files; lp != NULL; lp = lp->next)
    {
        GFile *file = lp->data;
        gchar *path = g_file_get_uri (file);
		    gchar *origin = g_path_get_dirname (path);
		    subject = zeitgeist_subject_new_full (
			    path,
			    NULL, // subject interpretation - auto-guess
			    NULL, // subject manifestation - auto-guess
			    NULL, // content type - auto-guess
			    origin,
			    NULL,
			    NULL // storage - auto-guess
		    );
		    g_free (origin);
		    g_free (path);
		    zeitgeist_event_add_subject (event, subject);
    }

		// Add the UCL study metadata object
		char *study_uri = g_strdup_printf("activity://null///pid://%d///winid://n/a///", getpid());
		subject = zeitgeist_subject_new_full(study_uri,
			ZEITGEIST_FILE_MANAGING_ACTOR,
			ZEITGEIST_ZG_WORLD_ACTIVITY,
			"application/octet-stream",
			NULL,
			"ucl-study-metadata",
			NULL);
		g_free(study_uri);
		zeitgeist_event_add_subject (event, subject);

    GError *error = NULL;
    ZeitgeistLog *log = zeitgeist_log_get_default ();
    zeitgeist_log_insert_events_no_reply (log, event, NULL, &error);

    if (error)
    {
      g_warning ("Impossible to log event for file trashing: %s ", error->message);
      g_error_free (error);
    }
}

void log_unlink (GList *files)
{
    fprintf (stderr, "unlinking files %d\n", g_list_length(files));
    if(!files) return;

		ZeitgeistEvent *event = zeitgeist_event_new_full (
      ZEITGEIST_ZG_DELETE_EVENT,
			ZEITGEIST_ZG_USER_ACTIVITY,
			ZEITGEIST_THUNAR_ACTOR,
			NULL, NULL);
		ZeitgeistSubject *subject = NULL;

    GList *lp;
    for (lp = files; lp != NULL; lp = lp->next)
    {
        GFile *file = lp->data;
        gchar *path = g_file_get_uri (file);
		    gchar *origin = g_path_get_dirname (path);
		    subject = zeitgeist_subject_new_full (
			    path,
			    NULL, // subject interpretation - auto-guess
			    NULL, // subject manifestation - auto-guess
			    NULL, // content type - auto-guess
			    origin,
			    NULL,
			    NULL // storage - auto-guess
		    );
		    g_free (origin);
		    g_free (path);
		    zeitgeist_event_add_subject (event, subject);
    }

		// Add the UCL study metadata object
		char *study_uri = g_strdup_printf("activity://null///pid://%d///winid://n/a///", getpid());
		subject = zeitgeist_subject_new_full(study_uri,
			ZEITGEIST_FILE_MANAGING_ACTOR,
			ZEITGEIST_ZG_WORLD_ACTIVITY,
			"application/octet-stream",
			NULL,
			"ucl-study-metadata",
			NULL);
		g_free(study_uri);
		zeitgeist_event_add_subject (event, subject);

    GError *error = NULL;
    ZeitgeistLog *log = zeitgeist_log_get_default ();
    zeitgeist_log_insert_events_no_reply (log, event, NULL, &error);

    if (error)
    {
      g_warning ("Impossible to log event for file unlinking: %s ", error->message);
      g_error_free (error);
    }
}

void log_transfer_init (ThunarTransferJob *job)
{
    if (!job || job->event) return;

    const gchar *interpretation = NULL;

    if (job->type == THUNAR_TRANSFER_JOB_MOVE)
        interpretation = ZEITGEIST_ZG_MOVE_EVENT;
    else if (job->type == THUNAR_TRANSFER_JOB_TRASH)
        interpretation = ZEITGEIST_ZG_DELETE_EVENT;
    else if (job->type == THUNAR_TRANSFER_JOB_LINK)
        interpretation = ZEITGEIST_ZG_LINK_EVENT;
    else if (job->type == THUNAR_TRANSFER_JOB_COPY)
        interpretation = ZEITGEIST_ZG_COPY_EVENT;

    if (!interpretation)
        return;

		job->event = zeitgeist_event_new_full (
			interpretation,
			ZEITGEIST_ZG_USER_ACTIVITY,
			ZEITGEIST_THUNAR_ACTOR,
			NULL, NULL);

    return;
}

void log_transfer_add_node (ThunarTransferJob *job, ThunarTransferNode *node, GFile *real_target_file)
{
    if (!job || !job->event) return;
    if (!node || !real_target_file) return;

    GError *err = NULL;
    GFileInfo *info = g_file_query_info (node->source_file,
                              G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
                              G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                              exo_job_get_cancellable (EXO_JOB (job)),
                              &err);
    if (G_UNLIKELY (info == NULL))
    {
        if (err)
        {
            g_error_free (err);
            err = NULL;
        }

        info = g_file_query_info (real_target_file,
                              G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
                              G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                              exo_job_get_cancellable (EXO_JOB (job)),
                              &err);
    }

    if (err)
    {
        g_error_free (err);
        err = NULL;
        return;
    }

    gchar *source_path = g_file_get_uri (node->source_file);
    gchar *target_path = g_file_get_uri (real_target_file);
    gchar *target_name = g_file_get_basename (real_target_file);
    fprintf (stderr, "%sing %s to %s\n", (job->type == THUNAR_TRANSFER_JOB_MOVE ? "mov" : job->type == THUNAR_TRANSFER_JOB_COPY ? "copy": job->type == THUNAR_TRANSFER_JOB_TRASH ? "trash":"link"), source_path, target_path);

    gchar *origin = g_path_get_dirname (source_path);
    ZeitgeistSubject *subject = zeitgeist_subject_new_full (
      source_path,
      NULL, // subject interpretation - auto-guess
      NULL, // subject manifestation - auto-guess
      NULL, // content type - auto-guess
      origin,
      g_file_info_get_display_name (info),
      NULL // storage - auto-guess
    );

    // One subject for move events, a pair for other events
    if (job->type == THUNAR_TRANSFER_JOB_MOVE)
    {
        zeitgeist_subject_set_current_uri (subject, target_path);
		    zeitgeist_event_add_subject (job->event, subject);
    }
    else
    {
		    zeitgeist_event_add_subject (job->event, subject);

        ZeitgeistSubject *subject2 = zeitgeist_subject_new_full (
          target_path,
          NULL, // subject interpretation - auto-guess
          NULL, // subject manifestation - auto-guess
          NULL, // content type - auto-guess
          source_path,
          target_name,
          NULL // storage - auto-guess
        );
		    zeitgeist_event_add_subject (job->event, subject2);
    }

    g_free (origin);
    g_free (source_path);
    g_free (target_path);
    g_free (target_name);
}

void log_transfer_finish (ThunarTransferJob *job)
{
    if (!job || !job->event) return;
		// Add the UCL study metadata object
		char *study_uri = g_strdup_printf ("activity://null///pid://%d///winid://n/a///", getpid());
		ZeitgeistSubject *subject = zeitgeist_subject_new_full (study_uri,
			ZEITGEIST_FILE_MANAGING_ACTOR,
			ZEITGEIST_ZG_WORLD_ACTIVITY,
			"application/octet-stream",
			NULL,
			"ucl-study-metadata",
			NULL);
		g_free (study_uri);
    if (job->type == THUNAR_TRANSFER_JOB_MOVE)
        zeitgeist_subject_set_current_uri (subject, "activity://dummy-for-move-event");
		zeitgeist_event_add_subject (job->event, subject);

    GError *error = NULL;
    ZeitgeistLog *log = zeitgeist_log_get_default ();
    zeitgeist_log_insert_events_no_reply (log, job->event, NULL, &error);
    job->event = NULL;

    if (error)
    {
      g_warning ("Impossible to log event for file transfer: %s ", error->message);
      g_error_free (error);
    }
}

void log_transfer_cleanup (ThunarTransferJob *job)
{
    if (!job || !job->event) return;

    g_object_unref (job->event);
    job->event = NULL;
}
