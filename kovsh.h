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

typedef enum {
    KSH_LEXER_TAG_CARGS,
    KSH_LEXER_TAG_STRV
} KshLexerTag;

typedef enum {
    KSH_LEXER_KIND_CSTR,
    KSH_LEXER_KIND_CARGS
} KshLexerKind;

typedef union {
    char *as_cstr;
    struct {
        int argc;
        char **argv;
    } as_cargs;
} KshLexerSource;

typedef struct {
    KshLexerSource src;
    KshLexerKind kind;
} KshLexer;

typedef struct {
    StrView name;
    const char *usage;
} KshArgBase;

typedef enum {
    KSH_PARAM_TYPE_STR,
    KSH_PARAM_TYPE_CSTR,
    KSH_PARAM_TYPE_INT,
    KSH_PARAM_TYPE_FLOAT,
} KshParamType;

typedef struct {
    KshArgBase base;
    KshParamType type;
    size_t max;
    void *var;
} KshParam;

typedef struct {
    KshArgBase base;
    bool *var;
} KshFlag;

struct KshParser;
typedef int (*KshCommandFn)(struct KshParser *p);
typedef struct {
    KshArgBase base;
    KshCommandFn fn;
} KshSubcmd;

// TODO: think about
typedef struct {
    const char *usage;
    void *var;
    const char **names; // null terminated
} KshChoice;

// Flag kind args => `-`
// Param kind args => `+`
KSH_ARR(KshChoice);
KSH_ARR(KshParam);
KSH_ARR(KshFlag);
KSH_ARR(KshSubcmd);
typedef struct {
    KshParams params;
    KshParams opt_params;
    KshFlags flags;
    KshChoices choices;
    KshSubcmds subcmds;
    const char *help;
} KshArgs;

typedef struct KshParser {
    KshLexer lex;
    KshErr err;
    int cmd_exit_code;
} KshParser;

#define KSH_PARAMS(...)  (KshParams){ (KshParam[]){__VA_ARGS__}, sizeof((KshParam[]){__VA_ARGS__})/sizeof(KshParam) }
#define KSH_FLAGS(...)   (KshFlags){ (KshFlag[]){__VA_ARGS__}, sizeof((KshFlag[]){__VA_ARGS__})/sizeof(KshFlag) }
#define KSH_SUBCMDS(...) (KshSubcmds){ (KshSubcmd[]){__VA_ARGS__}, sizeof((KshSubcmd[]){__VA_ARGS__})/sizeof(KshSubcmd) }
#define KSH_CHOICES(...) (KshChoices){ (KshChoice[]){__VA_ARGS__}, sizeof((KshChoice[]){__VA_ARGS__})/sizeof(KshChoice) }

#define KSH_PARAM(var, usage)  { { STRV_LIT(#var), usage }, KSH_PARAM_TYPE(var), sizeof(var)/(KSH_TYPESIZE(var)), &var }
#define KSH_PARAM_O(var, usage) { { STRV_LIT(#var), usage }, KSH_PARAM_TYPE(var), sizeof(var)/(KSH_TYPESIZE(var)), &var }
#define KSH_FLAG(var, usage) { { STRV_LIT(#var), usage }, &var }
#define KSH_CHOICE(var, usage, ...) { usage, &var, (const char *[]){__VA_ARGS__, NULL} }
#define KSH_SUBCMD(fn, name,  descr) { { STRV_LIT(name), descr }, fn }

#define KSH_PARAM_TYPE(var) _Generic(var, \
    StrView:  KSH_PARAM_TYPE_STR,         \
    StrView*: KSH_PARAM_TYPE_STR,         \
    char*:    KSH_PARAM_TYPE_CSTR,        \
    int:      KSH_PARAM_TYPE_INT,         \
    int*:     KSH_PARAM_TYPE_INT,         \
    float:    KSH_PARAM_TYPE_FLOAT,       \
    float*:   KSH_PARAM_TYPE_FLOAT)       \

#define KSH_TYPESIZE(var) _Generic(var, \
    int*: sizeof(int),                  \
    StrView*: sizeof(StrView),          \
    char*: sizeof(char),                \
    default: sizeof(var))               \

void ksh_parse_args(KshParser *p, KshArgs *args);

void ksh_init_from_cstr(KshParser *p, char *cstr);
void ksh_init_from_cargs(KshParser *p, int argc, char **argv);
bool ksh_parse(KshParser *p, KshCommandFn root_cmd);

#endif

// TODO: user error handling
