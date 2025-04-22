/*
 * Copyright (C) 2025 Alessandro Scotti
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef GLIB_SHIM_H
#define GLIB_SHIM_H

#include <alloca.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FALSE 0
#define TRUE 1
#define G_PI 3.14159265358979323846

#define GLIB_CHECK_VERSION(major, minor, micro) TRUE

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef int64_t gint64;
typedef uint64_t guint64;
typedef int gint;
typedef unsigned guint;
typedef char gchar;
typedef float gfloat;
typedef size_t gsize;

typedef void *gpointer;

#define GMutex int
#define GCond int

#define gchar char
#define gboolean int
#define GError int

// Memory mapped file
typedef struct {
    char *data;
    size_t size;
} GMappedFile;

extern GMappedFile *g_mapped_file_new(const gchar *filename, gboolean writable, GError **error);
extern gchar *g_mapped_file_get_contents(GMappedFile *file);
extern void g_mapped_file_unref(GMappedFile *file);

// List
typedef struct GList {
    void *data;
    struct GList *next;
    struct GList *prev;
} GList;

extern GList *g_list_first(GList *list);
extern GList *g_list_append(GList *list, void *data);
extern GList *g_list_prepend(GList *list, void *data);
extern GList *g_list_nth(GList *list, unsigned int n);
extern void *g_list_nth_data(GList *list, unsigned int n);
extern int g_list_length(GList *list);
extern void g_list_free(GList *list);

// Memory management
#define g_malloc(size) malloc(size)
#define g_malloc0(size) calloc(1, size)
#define g_free(ptr) free(ptr)
#define g_new(type, count) ((type *)malloc(sizeof(type) * (count)))
#define g_new0(type, count) ((type *)calloc((count), sizeof(type)))
#define g_try_new0(type, count) g_new0(type, count)
#define g_realloc(ptr, size) realloc(ptr, size)
#define g_memdup(ptr, size) memcpy(malloc(size), ptr, size)
#define g_memdup2(ptr, size) g_memdup(ptr, size)
#define g_alloca(size) alloca(size)

// Strings
#define g_strdup(str) strdup(str)
#define g_strndup(str, n) strndup(str, n)
#define g_strlcpy(dest, src, size) snprintf(dest, size, "%s", src)
#define g_strdup_printf(...) g_strdup_printf_impl(__VA_ARGS__)
#define g_strdup_vprintf(fmt, ap) g_strdup_vprintf_impl(fmt, ap)

extern char *g_strdup_printf_impl(const char *fmt, ...);

typedef gboolean (*GSourceFunc)(void *data);

static inline unsigned int g_timeout_add(unsigned int interval, GSourceFunc func, void *data) {
    (void)interval;
    func(data);
    return 1;
}

#define g_source_remove(s) ((void)s)

#define g_printerr(...) fprintf(stderr, __VA_ARGS__)
#define g_strerror(errnum) strerror(errnum)
#define g_print(...) printf(__VA_ARGS__)
#define g_printf(...) printf(__VA_ARGS__)

// Asserts
#define g_assert(expr) assert(expr)
#define g_assert_not_reached() assert(0 && "g_assert_not_reached")
#define g_return_if_fail(expr) \
    if (!(expr)) return;
#define g_return_val_if_fail(expr, val) \
    if (!(expr)) return (val);
#define g_warning(...) ((void)0)
#define g_message(...) ((void)0)
#define g_error(...) ((void)0)
#define g_error_free(err) free(err)

// I/O
#define g_fopen fopen
#define g_rename rename
#define g_mkstemp mkstemp
#define g_mkdir(path, mode) mkdir(path, mode)
#define g_file_test(...) 1
#define g_file_get_contents(...) 0
#define g_file_open_tmp(...) NULL
#define g_path_get_basename(path) (path)
#define g_path_get_dirname(path) (path)
#define g_path_is_absolute(path) 1
#define g_unlink unlink

// System
#define g_get_home_dir() "."
#define g_get_current_dir() "."
#define g_get_user_name() "user"
#define g_getenv(x) getenv(x)
#define g_setenv(k, v, o) setenv(k, v, o)
#define g_get_real_time() time(NULL)

// ASCII
#define g_ascii_isalnum(c) isalnum(c)
#define g_ascii_isdigit(c) isdigit(c)
#define g_ascii_tolower(c) tolower(c)
#define g_ascii_strcasecmp strcasecmp
#define g_ascii_strncasecmp strncasecmp
#define g_ascii_formatd(buf, len, fmt, val) snprintf(buf, len, fmt, val)
#define g_ascii_strtod strtod
#define g_ascii_strtoll strtoll
#define g_ascii_strtoull strtoull

// i18n
const char *gettext(const char *msgid);

#define N_(s) s
#define _(s) s

#endif // GLIB_SHIM_H
