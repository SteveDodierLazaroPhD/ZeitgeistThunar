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

#ifndef INC_LOGGER_H
#define INC_LOGGER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <glib.h>
#include <sys/time.h>
#include <time.h>

#include "thunar-file.h"
#include "thunar-job.h"
#include "thunar-transfer-job.h"

#define ZEITGEIST_THUNAR_ACTOR "application://Thunar.desktop"
#define ZEITGEIST_FILE_MANAGING_ACTOR "activity://file-manager/Actor"
#define ZEITGEIST_THUNAR_APP_LAUNCH "activity://file-manager/Thunar/AppAccess"
#define ZEITGEIST_THUNAR_FILE_EXEC "activity://file-manager/Thunar/FileExec"
#define ZEITGEIST_THUNAR_FILE_EXEC_INSECURE_LAUNCH "activity://file-manager/Thunar/InsecureFileExec"
#define ZEITGEIST_THUNAR_FILE_EXEC_INSECURE_MARK_EXEC "activity://file-manager/Thunar/InsecureFileMarkExecutable"
#define ZEITGEIST_THUNAR_FILE_EXEC_INSECURE_DENY "activity://file-manager/Thunar/InsecureFileDeny"
#define ZEITGEIST_ZG_TRASH_EVENT "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#TrashEvent"
#define ZEITGEIST_ZG_COPY_EVENT "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#CopyEvent"
#define ZEITGEIST_ZG_LINK_EVENT "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#LinkEvent"
#define ZEITGEIST_ZG_RESTORE_EVENT "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#TrashRestoreEvent"

void log_rename (ThunarFile *, GFile *, GFile *);

void log_open_files (GAppInfo *app_info, GList *path_list);
void log_execute (ThunarFile *target, GFile *origin_file, GList *params);
void log_insecure_dialog (ThunarFile *target, int response);

void log_create (GList *files);
void log_mkdir (GList *files);
void log_trash (GList *files);
void log_unlink (GList *files);

void log_transfer_init (ThunarTransferJob *job);
void log_transfer_add_node (ThunarTransferJob *job, ThunarTransferNode *node, GFile *real_target_file);
void log_transfer_finish (ThunarTransferJob *job);
void log_transfer_cleanup (ThunarTransferJob *job);


#endif /* INC_LOGGER_H */
