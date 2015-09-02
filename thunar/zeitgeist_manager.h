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

        xfwm4-zg - (c) 2015 Steve Dodier-Lazaro
 */

#ifndef UNITY_ZEITGEIST_MANAGER_H
#define UNITY_ZEITGEIST_MANAGER_H

#include <glib.h>
#include <sys/types.h>
#include <unistd.h>
#include <zeitgeist.h>
#include "logger.h"

ZeitgeistSubject *zeitgeist_get_ucl_subject (const pid_t pid);
char *zeitgeist_get_actor_name_from_pid (const pid_t pid);

#endif //UNITY_ZEITGEIST_MANAGER_H
