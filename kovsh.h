#ifndef KOVSH_H
#define KOVSH_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>

#define STRV_LIT(lit) (StrView){ sizeof(lit)-1, (lit) }
#define STRV_FMT "%.*s"
#define STRV_ARG(sv) (int) (sv).len, (sv).items

#define KSH_ARR(T)    \
    typedef struct {  \
        T *items;     \
        size_t count; \
    } T##s            \

#define MAX_ERR_MSG 100
typedef char KshErr[MAX_ERR_MSG];

typedef struct {
    size_t len;
    const char *items;
} StrView;

typedef StrView Token;

StrView strv_from_str(const char *str);
StrView strv_new(const char *data, size_t data_len);
bool    strv_eq(StrView sv1, StrView sv2);

typedef struct {
    StrView text;
    StrView buf;
    bool cargs;
} KshLexer;

typedef struct {
    StrView name;
    const char *usage;
} KshArg;

typedef enum {
    KSH_PARAM_TYPE_STR,
    KSH_PARAM_TYPE_INT,
    KSH_PARAM_TYPE_FLOAT,
} KshParamType;

typedef struct {
    KshArg base;
    KshParamType type;
    size_t max;
    void *var;
    bool assigned;
} KshParam;

typedef enum {
    KSH_FLAG_TYPE_STORE_BOOL,
    KSH_FLAG_TYPE_HELP,
} KshFlagType;

typedef struct {
    KshArg base;
    bool *var;
} KshFlag;

struct KshParser;
typedef int (*KshCommandFn)(struct KshParser *p);
typedef struct {
    KshArg base;
    KshCommandFn fn;
} KshSubcmd;

KSH_ARR(KshParam);
KSH_ARR(KshFlag);
KSH_ARR(KshSubcmd);
typedef struct {
    KshParams params;
    KshFlags flags;
    KshSubcmds subcmds;
    const char *help;
} KshArgs;

typedef struct KshParser {
    KshLexer lex;
    KshErr err;
} KshParser;

#define KSH_PARAMS(...)  (KshParams){ (KshParam[]){__VA_ARGS__}, sizeof((KshParam[]){__VA_ARGS__})/sizeof(KshParam) }
#define KSH_FLAGS(...)   (KshFlags){ (KshFlag[]){__VA_ARGS__}, sizeof((KshFlag[]){__VA_ARGS__})/sizeof(KshFlag) }
#define KSH_SUBCMDS(...) (KshSubcmds){ (KshSubcmd[]){__VA_ARGS__}, sizeof((KshSubcmd[]){__VA_ARGS__})/sizeof(KshSubcmd) }

#define KSH_PARAM(var, usage)  { { STRV_LIT(#var), usage }, KSH_PARAM_TYPE(var), sizeof(var)/(KSH_TYPESIZE(var)), &var, false }
#define KSH_PARAM_O(var, usage) { { STRV_LIT(#var), usage }, KSH_PARAM_TYPE(var), sizeof(var)/(KSH_TYPESIZE(var)), &var, true }

#define KSH_FLAG(var, usage) { { STRV_LIT(#var), usage }, KSH_FLAG_TYPE_STORE_BOOL, &var }
#define KSH_HELP(descr) { { STRV_LIT("help"), "displays this help" }, KSH_FLAG_TYPE_HELP, descr }

#define KSH_SUBCMD(fn, descr) { { STRV_LIT(#fn), descr }, fn }

#define KSH_PARAM_TYPE(var) _Generic(var, \
    StrView:  KSH_PARAM_TYPE_STR,         \
    StrView*: KSH_PARAM_TYPE_STR,         \
    int:      KSH_PARAM_TYPE_INT,         \
    int*:     KSH_PARAM_TYPE_INT,         \
    float:    KSH_PARAM_TYPE_FLOAT,       \
    float*:   KSH_PARAM_TYPE_FLOAT)       \

#define KSH_TYPESIZE(var) _Generic(var, \
    int*: sizeof(int),                  \
    StrView*: sizeof(StrView),          \
    default: sizeof(var))               \

void ksh_parse_args(KshParser *p, KshArgs *args);

bool ksh_parse_strv(StrView strv, KshCommandFn root_fn);
bool ksh_parse_cargs(int argc, char **argv, KshCommandFn root_fn);

#endif
