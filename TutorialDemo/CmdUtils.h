#pragma once
#include "ffmpeg.h"

#include <stdarg.h>
class CmdUtils
{
};

extern const char program_name[];

/**
 * program birth year, defined by the program for show_banner()
 */
extern const int program_birth_year;

extern AVDictionary* sws_dict;
extern AVDictionary* swr_opts;
extern AVDictionary* format_opts, * codec_opts;
extern int hide_banner;

void init_dynload(void);

void uninit_opts(void);


void log_callback_help(void* ptr, int level, const char* fmt, va_list vl);

int opt_default(void* optctx, const char* opt, const char* arg);


int opt_timelimit(void* optctx, const char* opt, const char* arg);

int parse_number(const char* context, const char* numstr, int type, double min, double max, double* dst);

typedef struct SpecifierOpt {
    char* specifier;    /**< stream/chapter/program/... specifier */
    union {
        uint8_t* str;
        int        i;
        int64_t  i64;
        uint64_t ui64;
        float      f;
        double   dbl;
    } u;
} SpecifierOpt;

typedef struct OptionDef {
    const char* name;
    int flags;
#define HAS_ARG    0x0001
#define OPT_BOOL   0x0002
#define OPT_EXPERT 0x0004
#define OPT_STRING 0x0008
#define OPT_VIDEO  0x0010
#define OPT_AUDIO  0x0020
#define OPT_INT    0x0080
#define OPT_FLOAT  0x0100
#define OPT_SUBTITLE 0x0200
#define OPT_INT64  0x0400
#define OPT_EXIT   0x0800
#define OPT_DATA   0x1000
#define OPT_PERFILE  0x2000     /* the option is per-file (currently ffmpeg-only).
                                   implied by OPT_OFFSET or OPT_SPEC */
#define OPT_OFFSET 0x4000       /* option is specified as an offset in a passed optctx */
#define OPT_SPEC   0x8000       /* option is to be stored in an array of SpecifierOpt.
                                   Implies OPT_OFFSET. Next element after the offset is
                                   an int containing element count in the array. */
#define OPT_TIME  0x10000
#define OPT_DOUBLE 0x20000
#define OPT_INPUT  0x40000
#define OPT_OUTPUT 0x80000
    union {
        void* dst_ptr;
        int (*func_arg)(void*, const char*, const char*);
        size_t off;
    } u;
    const char* help;
    const char* argname;
} OptionDef;


void show_help_options(const OptionDef* options, const char* msg, int req_flags, int rej_flags, int alt_flags);

void show_help_children(const AVClass* pClass, int flags);


void show_help_default(const char* opt, const char* arg);

int parse_options(void* optctx, int argc, char** argv, const OptionDef* options, int (*parse_arg_function)(void* optctx, const char*));


int parse_option(void* optctx, const char* opt, const char* arg, const OptionDef* options);

typedef struct Option {
    const OptionDef* opt;
    const char* key;
    const char* val;
} Option;

typedef struct OptionGroupDef {
    /**< group name */
    const char* name;
    /**
     * Option to be used as group separator. Can be NULL for groups which
     * are terminated by a non-option argument (e.g. ffmpeg output files)
     */
    const char* sep;
    /**
     * Option flags that must be set on each option that is
     * applied to this group
     */
    int flags;
} OptionGroupDef;

typedef struct OptionGroup {
    const OptionGroupDef* group_def;
    const char* arg;

    Option* opts;
    int  nb_opts;

    AVDictionary* codec_opts;
    AVDictionary* format_opts;
    AVDictionary* sws_dict;
    AVDictionary* swr_opts;
} OptionGroup;

/**
 * A list of option groups that all have the same group type
 * (e.g. input files or output files)
 */
typedef struct OptionGroupList {
    const OptionGroupDef* group_def;

    OptionGroup* groups;
    int       nb_groups;
} OptionGroupList;

typedef struct OptionParseContext {
    OptionGroup global_opts;

    OptionGroupList* groups;
    int           nb_groups;

    /* parsing state */
    OptionGroup cur_group;
} OptionParseContext;

/**
 * Parse an options group and write results into optctx.
 *
 * @param optctx an app-specific options context. NULL for global options group
 */
int parse_optgroup(void* optctx, OptionGroup* g);


int split_commandline(OptionParseContext* octx, int argc, char* argv[],
    const OptionDef* options,
    const OptionGroupDef* groups, int nb_groups);


void uninit_parse_context(OptionParseContext* octx);

/**
 * Find the '-loglevel' option in the command line args and apply it.
 */
void parse_loglevel(int argc, char** argv, const OptionDef* options);

/**
 * Return index of option opt in argv or 0 if not found.
 */
int locate_option(int argc, char** argv, const OptionDef* options,
    const char* optname);

int check_stream_specifier(AVFormatContext* s, AVStream* st, const char* spec);

/**
 * Filter out options for given codec.
 *
 * Create a new options dictionary containing only the options from
 * opts which apply to the codec with ID codec_id.
 *
 * @param opts     dictionary to place options in
 * @param codec_id ID of the codec that should be filtered for
 * @param s Corresponding format context.
 * @param st A stream from s for which the options should be filtered.
 * @param codec The particular codec for which the options should be filtered.
 *              If null, the default one is looked up according to the codec id.
 * @param dst a pointer to the created dictionary
 * @return a non-negative number on success, a negative error code on failure
 */
int filter_codec_opts(const AVDictionary* opts, enum AVCodecID codec_id,
    AVFormatContext* s, AVStream* st, const AVCodec* codec,
    AVDictionary** dst);

/**
 * Setup AVCodecContext options for avformat_find_stream_info().
 *
 * Create an array of dictionaries, one dictionary for each stream
 * contained in s.
 * Each dictionary will contain the options from codec_opts which can
 * be applied to the corresponding stream codec context.
 */
int setup_find_stream_info_opts(AVFormatContext* s,
    AVDictionary* codec_opts,
    AVDictionary*** dst);

/**
 * Print an error message to stderr, indicating filename and a human
 * readable description of the error code err.
 *
 * If strerror_r() is not available the use of this function in a
 * multithreaded application may be unsafe.
 *
 * @see av_strerror()
 */
void print_error(const char* filename, int err);

/**
 * Print the program banner to stderr. The banner contents depend on the
 * current version of the repository and of the libav* libraries used by
 * the program.
 */
void show_banner(int argc, char** argv, const OptionDef* options);

/**
 * Return a positive value if a line read from standard input
 * starts with [yY], otherwise return 0.
 */
int read_yesno(void);

/**
 * Get a file corresponding to a preset file.
 *
 * If is_path is non-zero, look for the file in the path preset_name.
 * Otherwise search for a file named arg.ffpreset in the directories
 * $FFMPEG_DATADIR (if set), $HOME/.ffmpeg, and in the datadir defined
 * at configuration time or in a "ffpresets" folder along the executable
 * on win32, in that order. If no such file is found and
 * codec_name is defined, then search for a file named
 * codec_name-preset_name.avpreset in the above-mentioned directories.
 *
 * @param filename buffer where the name of the found filename is written
 * @param filename_size size in bytes of the filename buffer
 * @param preset_name name of the preset to search
 * @param is_path tell if preset_name is a filename path
 * @param codec_name name of the codec for which to look for the
 * preset, may be NULL
 */
FILE* get_preset_file(char* filename, size_t filename_size,
    const char* preset_name, int is_path, const char* codec_name);

/**
 * Realloc array to hold new_size elements of elem_size.
 *
 * @param array pointer to the array to reallocate, will be updated
 *              with a new pointer on success
 * @param elem_size size in bytes of each element
 * @param size new element count will be written here
 * @param new_size number of elements to place in reallocated array
 * @return a non-negative number on success, a negative error code on failure
 */
int grow_array(void** array, int elem_size, int* size, int new_size);

/**
 * Atomically add a new element to an array of pointers, i.e. allocate
 * a new entry, reallocate the array of pointers and make the new last
 * member of this array point to the newly allocated buffer.
 *
 * @param array     array of pointers to reallocate
 * @param elem_size size of the new element to allocate
 * @param nb_elems  pointer to the number of elements of the array array;
 *                  *nb_elems will be incremented by one by this function.
 * @return pointer to the newly allocated entry or NULL on failure
 */
void* allocate_array_elem(void* array, size_t elem_size, int* nb_elems);

#define GROW_ARRAY(array, nb_elems)\
    grow_array((void**)&array, sizeof(*array), &nb_elems, nb_elems + 1)

#define GET_PIX_FMT_NAME(pix_fmt)\
    const char *name = av_get_pix_fmt_name(pix_fmt);

#define GET_CODEC_NAME(id)\
    const char *name = avcodec_descriptor_get(id)->name;

#define GET_SAMPLE_FMT_NAME(sample_fmt)\
    const char *name = av_get_sample_fmt_name(sample_fmt)

#define GET_SAMPLE_RATE_NAME(rate)\
    char name[16];\
    snprintf(name, sizeof(name), "%d", rate);

double get_rotation(const int32_t* displaymatrix);


#if !defined(va_copy) && defined(_MSC_VER)
#define va_copy(dst, src) ((dst) = (src))
#endif
#if !defined(va_copy) && defined(__GNUC__) && __GNUC__ < 3
#define va_copy(dst, src) __va_copy(dst, src)
#endif

//--------------------------------------------------------------------------------------
/*
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */



static inline wchar_t* get_module_filename(HMODULE module)
{
    wchar_t* path = NULL, * new_path;
    DWORD path_size = 0, path_len;

    do {
        path_size = path_size ? FFMIN(2 * path_size, INT16_MAX + 1) : MAX_PATH;
        new_path = (wchar_t*)av_realloc_array(path, path_size, sizeof * path);
        if (!new_path) {
            av_free(path);
            return NULL;
        }
        path = new_path;
        // Returns path_size in case of insufficient buffer.
        // Whether the error is set or not and whether the output
        // is null-terminated or not depends on the version of Windows.
        path_len = GetModuleFileNameW(module, path, path_size);
    } while (path_len && path_size <= INT16_MAX && path_size <= path_len);

    if (!path_len) {
        av_free(path);
        return NULL;
    }
    return path;
}

/**
 * Safe function used to open dynamic libs. This attempts to improve program security
 * by removing the current directory from the dll search path. Only dll's found in the
 * executable or system directory are allowed to be loaded.
 * @param name  The dynamic lib name.
 * @return A handle to the opened lib.
 */
static inline HMODULE win32_dlopen(const char* name)
{
    wchar_t* name_w;
    HMODULE module = NULL;
    //if (utf8towchar(name, &name_w))
    //    name_w = NULL;
#if _WIN32_WINNT < 0x0602
    // On Win7 and earlier we check if KB2533623 is available
    if (!GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "SetDefaultDllDirectories")) {
        wchar_t* path = NULL, * new_path;
        DWORD pathlen, pathsize, namelen;
        if (!name_w)
            goto exit;
        namelen = wcslen(name_w);
        // Try local directory first
        path = get_module_filename(NULL);
        if (!path)
            goto exit;
        new_path = wcsrchr(path, '\\');
        if (!new_path)
            goto exit;
        pathlen = new_path - path;
        pathsize = pathlen + namelen + 2;
        new_path = av_realloc_array(path, pathsize, sizeof * path);
        if (!new_path)
            goto exit;
        path = new_path;
        wcscpy(path + pathlen + 1, name_w);
        module = LoadLibraryExW(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
        if (module == NULL) {
            // Next try System32 directory
            pathlen = GetSystemDirectoryW(path, pathsize);
            if (!pathlen)
                goto exit;
            // Buffer is not enough in two cases:
            // 1. system directory + \ + module name
            // 2. system directory even without the module name.
            if (pathlen + namelen + 2 > pathsize) {
                pathsize = pathlen + namelen + 2;
                new_path = av_realloc_array(path, pathsize, sizeof * path);
                if (!new_path)
                    goto exit;
                path = new_path;
                // Query again to handle the case #2.
                pathlen = GetSystemDirectoryW(path, pathsize);
                if (!pathlen)
                    goto exit;
            }
            path[pathlen] = L'\\';
            wcscpy(path + pathlen + 1, name_w);
            module = LoadLibraryExW(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
        }
    exit:
        av_free(path);
        av_free(name_w);
        return module;
    }
#endif
#ifndef LOAD_LIBRARY_SEARCH_APPLICATION_DIR
#   define LOAD_LIBRARY_SEARCH_APPLICATION_DIR 0x00000200
#endif
#ifndef LOAD_LIBRARY_SEARCH_SYSTEM32
#   define LOAD_LIBRARY_SEARCH_SYSTEM32        0x00000800
#endif
#if HAVE_WINRT
    if (!name_w)
        return NULL;
    module = LoadPackagedLibrary(name_w, 0);
#else
#define LOAD_FLAGS (LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32)
    /* filename may be be in CP_ACP */
    if (!name_w)
        return LoadLibraryExA(name, NULL, LOAD_FLAGS);
    module = LoadLibraryExW(name_w, NULL, LOAD_FLAGS);
#undef LOAD_FLAGS
#endif
    av_free(name_w);
    return module;
}
#define dlopen(name, flags) win32_dlopen(name)
#define dlclose FreeLibrary
#define dlsym GetProcAddress


