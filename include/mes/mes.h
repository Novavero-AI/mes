/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2016,2017,2018,2019,2020,2021,2022 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
 * Copyright © 2022 Timothy Sample <samplet@ngyro.com>
 * Copyright © 2025 Ekaitz Zarraga <ekaitz@elenq.tech>
 *
 * This file is part of GNU Mes.
 *
 * GNU Mes is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * GNU Mes is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Mes.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __MES_MES_H
#define __MES_MES_H

#include <sys/types.h>

typedef struct scm* SCM;
#if __MESC__
typedef long FUNCTION;
typedef long function0_t;
typedef long function1_t;
typedef long function2_t;
typedef long function3_t;
typedef long functionn_t;
#else // !__MESC__
typedef SCM (*FUNCTION) (void);
typedef SCM (*function0_t) (void);
typedef SCM (*function1_t) (SCM);
typedef SCM (*function2_t) (SCM, SCM);
typedef SCM (*function3_t) (SCM, SCM, SCM);
typedef SCM (*functionn_t) (SCM);
#endif // !__MESC__

#include "mes/constants.h"

struct builtin {
  FUNCTION function;
  const char *name;
  signed char arity;
  /* TODO: add inlining information and all */
};

struct scm
{
  enum cell_type type;
  union
  {
    struct scm *car;
    long car_value;
    long port;              /* for TPORT */
    long length;            /* for TSTRING, TBYTES, TVECTOR and TSTRUCT */
    struct scm *name;       /* for TBINDING */
    struct scm *env;        /* for TCLOSURE */
    struct scm *state;      /* for TCONTINUATION */
    struct
      {
        unsigned short pc;
        unsigned short fp;
      } frame;              /* for TFRAME */
  };
  union
  {
    struct scm *cdr;
    long cdr_value;
    struct scm *binding;    /* for TBINDING */
    struct scm *stack;      /* for TCONTINUATION */
    struct scm *string;     /* for TSTRING */
    long value;             /* for TNUMBER and TCHAR */
    struct scm *variable;   /* for TVARIABLE */
    long builtin;           /* for TBUILTIN */
    struct scm *template;   /* for TCLOSURE */
    struct scm *closure;    /* for TFRAME */
  };
};

/* M2-Planet does not support pointer arithmetic.  Explicitly compensate
   for that by multiplying with M2_CELL_SIZE when using cell
   pointers. */
#if __M2__
#define M2_CELL_SIZE sizeof (struct scm)
#else
#define M2_CELL_SIZE 1
#endif

/* mes */
extern char *g_datadir;
extern int g_debug;
extern char *g_buf;
extern int g_continuations;
extern struct scm *g_symbols;
extern struct scm *g_symbol_max;
extern int g_mini;

/* evaluator */
extern int g_booted;
extern long g_stack;
extern struct scm **STACK;

#if (__riscv && (__GNUC__ || __TINYC__)) || (!__riscv && !__M2__)
#include <setjmp.h>
extern jmp_buf JMP_BUF;
#endif

/* builtins */
extern struct builtin BUILTINS [BUILTINS_SIZE];
/* initial module obarray */
extern struct scm *M0;
/* current module */
extern struct scm *M1;
extern struct scm *g_ports;

/* gc */
extern size_t ARENA_SIZE;
extern size_t STACK_SIZE;
extern size_t GC_SAFETY;
extern size_t MAX_STRING;
extern char *g_arena;

extern struct scm *g_free;
extern struct scm *g_symbol;

extern struct scm *g_cells;
extern struct scm *g_cells_end;
extern struct scm *g_news;
extern size_t gc_count;
extern struct timespec *gc_start_time;
extern struct timespec *gc_end_time;
extern size_t gc_time;

extern char **__execl_c_argv;
extern char **__execle_c_env;
extern char *__open_boot_buf;
extern char *__open_boot_file_name;
extern char *__setenv_buf;
extern char *__reader_read_char_buf;
extern struct timespec *g_start_time;
extern struct timeval *__gettimeofday_time;
extern struct timespec *__get_internal_run_time_ts;
extern struct utsname *__uts;

extern struct scm *scm_hash_table_type;
extern struct scm *scm_template_type;
extern struct scm *scm_module_type;

#define cell_bytes(x) ((char *)(x) + sizeof (struct scm))
#define vector_body(x) ((struct scm **)cell_bytes (x))
#define vector_bytes(x) (((x)->length) * sizeof (struct scm*))

struct scm *alloc (long n);
struct scm *cstring_to_list (char const *s);
struct scm *cstring_to_symbol (char const *s);
struct scm *cell_ref (struct scm *cell, long index);
struct scm *make_module_type ();
struct scm *fdisplay_ (struct scm *, int, int);
struct scm *init_symbols ();
struct scm *init_time (struct scm *a);
struct scm *make_bytes (size_t length);
struct scm *make_cell (long type, struct scm *car, struct scm *cdr);
struct scm *make_pointer_cell (long type, long car, void *cdr);
struct scm *make_value_cell (long type, long car, long cdr);
struct scm *make_frame (struct scm *closure, unsigned int pc, unsigned int fp);
struct scm *make_char (int n);
struct scm *make_hash_table_ (long size);
struct scm *make_hash_table_type ();
struct scm *make_initial_module (struct scm *a);
struct scm *make_closure (struct scm *env, struct scm *template);
struct scm *make_number (long n);
struct scm *make_string (char const *s, size_t length);
struct scm *make_string0 (char const *s);
struct scm *make_string_port (struct scm *x);
struct scm *make_variable_type ();
struct scm *make_vector_ (long k, struct scm *e);
struct scm *make_vector_uninit_ (long k);
struct scm *mes_builtins (struct scm *a);
struct scm *push_cc (struct scm *p1, struct scm *p2, struct scm *a, struct scm *c);
struct scm *set_x (struct scm *x, struct scm *e, int define_p);
struct scm *struct_ref_ (struct scm *x, long i);
struct scm *struct_set_x_ (struct scm *x, long i, struct scm *e);
struct scm *string_set_x_ (struct scm *str, long i, char c);
char string_ref_ (struct scm *str, long i);
struct scm *string_copy_x_ (struct scm *str, long start, struct scm *source, long begin, long end);
struct scm *make_string_init_ (long length, char c);
struct scm *vector_ref_ (struct scm *x, long i);
struct scm *vector_set_x_ (struct scm *x, long i, struct scm *e);
struct scm *vector_copy_x_ (struct scm *vec, long start, struct scm *source, long begin, long end);
struct scm *vector_fill_x_ (struct scm *vec, struct scm *fill, long start, long end);
struct scm *make_bytevector_ (long size, long value);
struct scm *make_bytevector_uninit_ (long size);
struct scm *bytevector_fill_x_ (struct scm *bv, long value, long start, long end);

struct scm *template_name (struct scm *template);
struct scm *template_arity (struct scm *template);
struct scm *template_source (struct scm *template);
struct scm *template_module (struct scm *template);
struct scm *template_code (struct scm *template);
struct scm *template_literals (struct scm *template);
struct scm *template_locals (struct scm *template);
struct scm *template_arguments (struct scm *template);
struct scm *compile_procedure (struct scm *exp, struct scm *name, struct scm *mod);
char *inst_to_string (enum opcode inst);
struct scm *eval_compiled ();
struct scm *apply (struct scm *fn, struct scm *x);

long bytevector_u8_ref_ (struct scm *bv, long k);
void bytevector_u8_set_x_ (struct scm *bv, long k, long value);
FUNCTION builtin_function (struct scm *builtin);
signed char builtin_arity_ (struct scm *builtin);
struct scm *list2 (struct scm *x, struct scm *y);
struct scm *list3 (struct scm *x, struct scm *y, struct scm *z);
#if 0
void assert_type (long type, char const *name_name, struct scm *x)
#endif
int peekchar ();
int readchar ();
int unreadchar ();
long gc_used ();
long length__ (struct scm *x);
size_t bytes_cells (size_t length);
void assert_max_string (size_t i, char const *msg, char const *string);
void assert_msg (int check, char *msg);
void assert_number (char const *name, struct scm *x);
void copy_cell (struct scm *to, struct scm *from);
void gc_ ();
void gc_dump_arena (struct scm *cells, long size);
void gc_init ();
void gc_stats_ (char const* where);
struct scm *gc_check ();
struct scm *gc ();
void init_symbols_ ();
long seconds_and_nanoseconds_to_long (long s, long ns);

#include "mes/builtins.h"
#include "mes/symbols.h"

#if !__M2__
#define stack_init()        (g_stack = 0)
#define stack_index(i)      (STACK [i])
#define stack_set(i, v)     (STACK [i] = v)
#define stack_push(val)     (STACK [g_stack++] = val)
#define stack_pop()         (STACK [--g_stack])
#define stack_drop(n)       (g_stack -= n)
#define stack_peek()        (STACK[g_stack-1])
#define stack_empty_p()     (g_stack == 0)
#define stack_rewind_to(to) (stack_drop (g_stack - (to)))
#else
struct scm *stack_index (long i);
void stack_set (long i, struct scm *v);
void stack_init ();
void stack_push (struct scm *x);
struct scm *stack_pop ();
void stack_drop (long n);
struct scm *stack_peek ();
int stack_empty_p ();
void stack_rewind_to (long to);
#endif

#endif /* __MES_MES_H */
