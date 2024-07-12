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
    size_t cursor;
    Token cur_tok;
    bool is_peek;
} Lexer;

typedef struct {
    StrView name;
    const char *usage;
} KshArg;

typedef enum {
    KSH_PARAM_TYPE_STR,
    KSH_PARAM_TYPE_INT
} KshParamType;

typedef struct {
    KshArg base;
    KshParamType type;
    size_t max;
    void *var;
} KshParam;

typedef struct {
    KshParam *items;
    size_t count;
} KshParams;

typedef struct {
    KshArg base;
    bool *var;
} KshFlag;

typedef struct {
    KshFlag *items;
    size_t count;
} KshFlags;

struct KshParser;
typedef int (*KshCommandFn)(struct KshParser *p);
typedef struct {
    KshArg base;
    KshCommandFn fn;
} KshSubcmd;

typedef struct {
    KshSubcmd *items;
    size_t count;
} KshSubcmds;

typedef struct KshParser {
    Lexer lex;
    KshParams params;
    KshFlags flags;
    KshSubcmds subcmds;
    KshErr err;
    KshCommandFn root;
} KshParser;

#define KSH_PARAMS(p, ...) p->params = (KshParams){ (KshParam[]){__VA_ARGS__}, sizeof((KshParam[]){__VA_ARGS__})/sizeof(KshParam) }
#define KSH_FLAGS(p, ...) p->flags = (KshFlags){ (KshFlag[]){__VA_ARGS__}, sizeof((KshFlag[]){__VA_ARGS__})/sizeof(KshFlag) }
#define KSH_SUBCMDS(p, ...) p->subcmds = (KshSubcmds){ (KshSubcmd[]){__VA_ARGS__}, sizeof((KshSubcmd[]){__VA_ARGS__})/sizeof(KshSubcmd) }

#define KSH_PARAM(var, usage)  { { STRV_LIT(#var), usage }, KSH_PARAM_TYPE(var), sizeof(var)/(KSH_TYPESIZE(var)), &var }
#define KSH_FLAG(var, usage)   { { STRV_LIT(#var), usage }, &var }
#define KSH_SUBCMD(var, usage) { { STRV_LIT(#var), usage }, var }

#define KSH_PARAM_TYPE(var) _Generic(var, \
    StrView:  KSH_PARAM_TYPE_STR,         \
    StrView*: KSH_PARAM_TYPE_STR,         \
    int:      KSH_PARAM_TYPE_INT,         \
    int*:     KSH_PARAM_TYPE_INT)         \

#define KSH_TYPESIZE(var) _Generic(var, \
    int*: sizeof(int),                  \
    StrView*: sizeof(StrView),          \
    default: sizeof(var))               \

bool ksh_parse(KshParser *p);
bool ksh_parse_cmd(KshParser *p, StrView cmd);

#endif
