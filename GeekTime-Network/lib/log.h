#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include    <stdarg.h>        /* ANSI C header file */
#include    <syslog.h>        /* for syslog() */

#define LOG_DEBUG_TYPE 0
#define LOG_MSG_TYPE   1
#define LOG_WARN_TYPE  2
#define LOG_ERR_TYPE   3

/**
 * error - print a diagnostic and optionally exit.
 *
 * @status the status of the current program. if status != 0, the program will exit.
 * @err error no.
 */
void error(int status, int err, char *fmt, ...);

void yolanda_log(int severity, const char *msg);

void yolanda_logx(int severity, const char *errstr, const char *fmt, va_list ap);
void yolanda_debugx(const char *fmt, ...);
void yolanda_msgx(const char *fmt, ...);
void yolanda_warnx(const char *fmt, ...);
void yolanda_errorx(const char *fmt, ...);

#endif