/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2016,2017,2018,2019,2020,2021,2022 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
 * Copyright © 2022 Gabriel Wicki <gabriel@erlikon.ch>
 * Copyright © 2023,2026 Timothy Sample <samplet@ngyro.com>
 * Copyright © 2025,2026 Ekaitz Zarraga <ekaitz@elenq.tech>
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

#include "mes/lib.h"
#include "mes/mes.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

int g_dump_filedes;

#if __M2__
#define U3 3
#else
#define U3 3U
#endif

size_t
arena_bytes_ ()
{
  return ARENA_SIZE * sizeof (struct scm);
}

void
gc_init ()
{
#if SYSTEM_LIBC
  ARENA_SIZE = 100000000;       /* 2.3GiB */
#elif ! __M2__
  ARENA_SIZE = 2400000;         /* 32b: 24MiB, 64b: 48 MiB */
#else
  ARENA_SIZE = 20000000;
#endif
  STACK_SIZE = 20000;

  GC_SAFETY = 2000;
  MAX_STRING = 524288;

  char *p;
  p = getenv ("MES_ARENA");
  if (p != 0)
    ARENA_SIZE = atoi (p);
  GC_SAFETY = ARENA_SIZE / U3;
  p = getenv ("MES_SAFETY");
  if (p != 0)
    GC_SAFETY = atoi (p);
  p = getenv ("MES_STACK");
  if (p != 0)
    STACK_SIZE = atoi (p);
  /* TODO: Our TFRAMEs only support unsigned shorts for frame pointers */
  assert_msg (STACK_SIZE < (2 << 16), "MES_STACK: Max allowed: 2 << 16");
  p = getenv ("MES_MAX_STRING");
  if (p != 0)
    MAX_STRING = atoi (p);

  /* Allocate the heap */
  long arena_bytes = arena_bytes_ ();
  g_arena = malloc (arena_bytes);
  if (g_arena == 0)
    assert_msg (0, "gc_init: out of memory");
  g_cells = (struct scm *) g_arena;
  g_cells_end = (struct scm *) (g_arena + arena_bytes);
  g_free = g_cells;

  /* Allocate stack */
  STACK = malloc (STACK_SIZE * sizeof (struct scm **));
  if (STACK == 0)
    assert_msg (0, "gc_init: out of memory");
  stack_init ();

  /* FIXME: remove MES_MAX_STRING, grow dynamically. */
  g_buf = malloc (MAX_STRING);
}

long
gc_used ()
{
  return (g_free - g_cells) / M2_CELL_SIZE;
}

void
gc_stats_ (char const* where)
{
  if (where)
    {
      eputs (where);
      eputs (": ");
    }
  eputs ("[");
  eputs (ltoa (gc_used ()));
  eputs ("]\n");
}

struct scm *
gc_stats ()
{
  gc_stats_ (0);
  size_t arena_used = g_free - g_cells;
  arena_used = arena_used / M2_CELL_SIZE;
  size_t arena_free = ARENA_SIZE - arena_used;
  struct scm *r = cell_nil;
  r =  acons (cstring_to_symbol ("gc-count"), make_number (gc_count), r);
  r =  acons (cstring_to_symbol ("gc-time"), make_number (gc_time), r);
  r =  acons (cstring_to_symbol ("arena-free"), make_number (arena_free), r);
  r =  acons (cstring_to_symbol ("arena-size"), make_number (ARENA_SIZE), r);
  return r;
}

struct scm *
alloc (long n)
{
  struct scm *x = g_free;
  g_free = g_free + (n * M2_CELL_SIZE);
  if (g_free >= g_cells_end)
    assert_msg (0, "alloc: out of memory");
  return x;
}

struct scm *
make_cell (long type, struct scm *car, struct scm *cdr)
{
  struct scm *x = g_free;
  g_free = g_free + M2_CELL_SIZE;
  if (g_free >= g_cells_end)
    assert_msg (0, "make_cell: out of memory");
  x->type = type;
  x->car = car;
  x->cdr = cdr;
  return x;
}

struct scm *
make_pointer_cell (long type, long car, void *cdr)
{
  struct scm *x = g_free;
  g_free = g_free + M2_CELL_SIZE;
  if (g_free >= g_cells_end)
    assert_msg (0, "make_pointer_cell: out of memory");
  x->type = type;
  x->length = car;
  x->cdr = cdr;
  return x;
}

struct scm *
make_value_cell (long type, long car, long cdr)
{
  struct scm *x = g_free;
  g_free = g_free + M2_CELL_SIZE;
  if (g_free >= g_cells_end)
    assert_msg (0, "make_value_cell: out of memory");
  x->type = type;
  x->length = car;
  x->value = cdr;
  return x;
}

void
copy_cell (struct scm *to, struct scm *from)
{
  to->type = from->type;
  to->car = from->car;
  to->cdr = from->cdr;
}

struct scm *
cell_ref (struct scm *cell, long index)
{
  return cell + (index * M2_CELL_SIZE);
}

struct scm *
cons (struct scm *x, struct scm *y)
{
  return make_cell (TPAIR, x, y);
}

size_t
bytes_cells (size_t length)
{
  int remainder_p = length % sizeof (struct scm);
  if (remainder_p > 0)
    remainder_p = 1;
  else
    remainder_p = 0;
  return 1 + (length / sizeof (struct scm)) + remainder_p;
}

struct scm *
make_bytes (size_t length)
{
  size_t size = bytes_cells (length);
  struct scm *x = alloc (size);
  x->type = TBYTES;
  x->length = length;
  return x;
}

struct scm *
make_char (int n)
{
  return make_value_cell (TCHAR, 0, n);
}

struct scm *
make_frame (struct scm *closure, unsigned int pc, unsigned int fp)
{
  struct scm *cell = make_cell (TFRAME, 0, closure);
  cell->frame.pc = pc;
  cell->frame.fp = fp;
  return cell;
}

struct scm *
make_closure (struct scm *env, struct scm *template)
{
  return make_cell (TCLOSURE, env, template);
}

struct scm *
make_number (long n)
{
  return make_value_cell (TNUMBER, 0, n);
}

struct scm *
make_string (char const *s, size_t length)
{
  if (length > MAX_STRING)
    assert_max_string (length, "make_string", s);
  struct scm *x = make_pointer_cell (TSTRING, length, 0);
  struct scm *v = make_bytes (length + 1);
  char *p = cell_bytes (v);
  memcpy (p, s, length + 1);
  x->cdr = v;
  return x;
}

struct scm *
make_string0 (char const *s)
{
  return make_string (s, strlen (s));
}

struct scm *
make_binding (struct scm *name, struct scm *variable)
{
  return make_cell (TBINDING, name, variable);
}

struct scm *
make_string_port (struct scm *x)        /*:((internal)) */
{
  return make_pointer_cell (TPORT, -length__ (g_ports) - 2, x);
}

/* A pointer relocating memcpy for pointer cells to avoid using only
   half of the allocated cells.

   For number based cells a simply memcpy could be used, as number
   references are relative.

   A simple stop and copy (SICP 5.3) garbage collector allocates twice
   the cell arena size only for the garbage collector.  The garbage
   collector switches back and forth between cells and news, thus
   utilizing only half the allocated memory. */
void
gc_cellcpy (struct scm *dest, struct scm *src, size_t n)
{
  void *p = src;
  void *q = dest;
  long dist = p - q;
  long t;
  long a;
  long d;
  int i;
  int c;
  while (n != 0)
    {
      t = src->type;
      a = src->car_value;
      d = src->cdr_value;
      dest->type = t;
      switch (t)
        {
        case TBROKEN_HEART:
          assert_msg (0, "gc_cellcpy: broken heart");
          break;
        case TBYTES:
        case TKEYWORD:
        case TPORT:
        case TSPECIAL:
        case TSTRING:
        case TSTRUCT:
        case TSYMBOL:
        case TVALUES:
        case TVARIABLE:
        case TVECTOR:
        case TFRAME:
          dest->car_value = a;
          dest->cdr_value = d - dist;
          break;
        case TPAIR:
        case TBINDING:
        case TCLOSURE:
        case TCONTINUATION:
          dest->car_value = a - dist;
          dest->cdr_value = d - dist;
          break;
        default:
          dest->car_value = a;
          dest->cdr_value = d;
          break;
        }
      if (t == TBYTES)
        {
          if (g_debug > 5)
            {
              eputs ("copying bytes[");
              eputs (ntoab ((long) cell_bytes (src), 16, 0));
              eputs (", ");
              eputs (ntoab (a, 10, 0));
              eputs ("]: ");
              eputs (cell_bytes (src));
              eputs ("\n to [");
              eputs (ntoab ((long) cell_bytes (dest), 16, 0));
            }
          memcpy (cell_bytes (dest), cell_bytes (src), a);
          if (g_debug > 5)
            {
              eputs ("]: ");
              eputs (cell_bytes (dest));
              eputs ("\n");
            }
          i = bytes_cells (a);
          n = n - i;
          c = i * M2_CELL_SIZE;
          dest = dest + c;
          src = src + c;
        }
      else if (t == TVECTOR || t == TSTRUCT)
        {
          size_t i = 0;
          struct scm **dest_b = vector_body (dest);
          struct scm **src_b = vector_body (src);
          for (i = 0; i < dest->length; i = i + 1)
            {
              struct scm **loc = src_b + i * M2_PTR_SIZE;
              struct scm *value = *loc;
              long later = (long)value;
              later = later - dist;

              loc = dest_b + i * M2_PTR_SIZE;
              *loc = (struct scm *)(later);
            }

          i = bytes_cells (vector_bytes (dest));
          n = n - i;
          c = i * M2_CELL_SIZE;
          dest = dest + c;
          src = src + c;
        }
      else
        {
          n = n - 1;
          dest = dest + M2_CELL_SIZE;
          src = src + M2_CELL_SIZE;
        }
    }
}

/* We do not actually flip cells and news, instead we move news back to
   cells. */
void
gc_flip ()
{
  gc_cellcpy (g_cells, g_news, (g_free - g_news) / M2_CELL_SIZE);

  long dist = g_news - g_cells;
  g_free = g_free - dist;
  long i;
  for (i = 0; i <= g_stack; i++)
    stack_set (i, stack_index (i) - dist);
  g_symbols = g_symbols - dist;
  g_ports = g_ports - dist;
  if (scm_hash_table_type)
    scm_hash_table_type = scm_hash_table_type - dist;
  if (scm_template_type)
    scm_template_type = scm_template_type - dist;
  if (scm_module_type)
    scm_module_type = scm_module_type - dist;
  M0 = M0 - dist;
  M1 = M1 - dist;
}

struct scm *
gc_copy (struct scm *old)               /*:((internal)) */
{
  if (old->type == TBROKEN_HEART)
    return old->car;
  struct scm *new = alloc (1);
  copy_cell (new, old);
  size_t length = 0;
  switch (new->type)
    {
    case TSTRUCT:
    case TVECTOR:
      length = vector_bytes (new);
      /* fallthrough */
    case TBYTES:
      {
        if (length == 0)
          length = new->length;
        char const *src = cell_bytes (old);
        char *dest = cell_bytes (new);
        struct scm *new_bytes = alloc (bytes_cells (length) - 1);
        if ((char *) new_bytes != cell_bytes (new))
          assert_msg (0, "gc_copy: could not copy bytes");
        memcpy (dest, src, length);

        if (g_debug > 4)
          {
            eputs ("gc copy bytes: ");
            eputs (src);
            eputs ("\n");
            eputs ("    length: ");
            eputs (ltoa (old->length));
            eputs ("\n");
            eputs ("    nlength: ");
            eputs (ltoa (new->length));
            eputs ("\n");
            eputs ("        ==> ");
            eputs (dest);
            eputs ("\n");
          }
        break;
      }
    default:
      break;
    }
  old->type = TBROKEN_HEART;
  old->car = new;
  return new;
}

struct scm *
gc_relocate_car (struct scm *new, struct scm *car)      /*:((internal)) */
{
  new->car = car;
  return cell_unspecified;
}

struct scm *
gc_relocate_cdr (struct scm *new, struct scm *cdr)      /*:((internal)) */
{
  new->cdr = cdr;
  return cell_unspecified;
}

void
gc_loop (struct scm *scan)
{
  struct scm *car;
  struct scm *cdr;
  long t;
  while (scan < g_free)
    {
      t = scan->type;
      switch (scan->type)
        {
        case TBROKEN_HEART:
          assert_msg (0, "gc_loop: broken heart");
          break;
        case TPAIR:
        case TBINDING:
        case TCLOSURE:
        case TCONTINUATION:
          car = gc_copy (scan->car);
          gc_relocate_car (scan, car);
          cdr = gc_copy (scan->cdr);
          gc_relocate_cdr (scan, cdr);
          break;
        case TKEYWORD:
        case TPORT:
        case TSPECIAL:
        case TSTRING:
        case TSYMBOL:
        case TVALUES:
        case TVARIABLE:
        case TFRAME:
          cdr = gc_copy (scan->cdr);
          gc_relocate_cdr (scan, cdr);
          break;
        case TVECTOR:
        case TSTRUCT:
            {
              size_t i = 0;
              size_t l = scan->length;
              struct scm *v = scan;
              struct scm **vb = vector_body (v);
              for (i = 0; i < l; i = i + 1)
                {
                  struct scm **loc = vb + i * M2_PTR_SIZE;
                  *loc = gc_copy (*loc);
                }
              scan = scan + ((bytes_cells (vector_bytes (v)) - 1) * M2_CELL_SIZE);
              break;
            }
        case TBYTES:
          scan = scan + ((bytes_cells (scan->length) - 1) * M2_CELL_SIZE);
          break;
        default:
          break;
        }
      scan = scan + M2_CELL_SIZE;
    }
  gc_flip ();
}

struct scm *
gc_check ()
{
  if (gc_used () + GC_SAFETY >= ARENA_SIZE)
    return gc ();
  return cell_unspecified;
}

void
gc_ ()
{
  /* The new semispace starts in the free space */
  g_news = g_free;

  if (g_debug == 2)
    eputs (".");
  if (g_debug > 2)
    {
      gc_stats_ (";;; gc");
      eputs (";;; free: [");
      eputs (ltoa (ARENA_SIZE - gc_used ()));
      eputs ("]...");
    }

  struct scm *new_cell_nil = g_free;
  struct scm *s;
  for (s = cell_nil; s < g_symbol_max; s = s + M2_CELL_SIZE)
    gc_copy (s);

  g_symbols = gc_copy (g_symbols);
  g_ports = gc_copy (g_ports);
  if (scm_hash_table_type)
    scm_hash_table_type = gc_copy (scm_hash_table_type);
  if (scm_template_type)
    scm_template_type = gc_copy (scm_template_type);
  if (scm_module_type)
    scm_module_type = gc_copy (scm_module_type);
  M0 = gc_copy (M0);
  M1 = gc_copy (M1);

  long i;
  for (i = 0; i <= g_stack; i++)
    stack_set (i, gc_copy (stack_index (i)));

  gc_loop (new_cell_nil);
}

struct scm *
gc ()
{
  if (getenv ("MES_DUMP") != 0)
    gc_dump_arena (g_cells, gc_used ());
  if (g_debug > 5)
    {
      eputs ("symbols: ");
      write_error_ (g_symbols);
      eputs ("\n");
    }
  clock_gettime (CLOCK_PROCESS_CPUTIME_ID, gc_start_time);
  gc_ ();
  clock_gettime (CLOCK_PROCESS_CPUTIME_ID, gc_end_time);
  long time = seconds_and_nanoseconds_to_long
    (gc_end_time->tv_sec - gc_start_time->tv_sec,
     gc_end_time->tv_nsec - gc_start_time->tv_nsec);
  gc_time = gc_time + time;
  gc_count = gc_count + 1;
  if (g_debug > 5)
    {
      eputs ("symbols: ");
      write_error_ (g_symbols);
      eputs ("\n");
    }
  if (getenv ("MES_DUMP") != 0)
    gc_dump_arena (g_cells, gc_used ());
  return cell_unspecified;
}

void
dumpc (char c)
{
  fdputc (c, g_dump_filedes);
}

void
dumps (char const *s)
{
  fdputs (s, g_dump_filedes);
}

void
gc_dump_register (char const* n, struct scm *r)
{
  dumps (n); dumps (": ");
  long i = (long) r;
  long a = (long) g_arena;
  i = i - a;
  i = i / M2_CELL_SIZE;
  dumps (ltoa (i));
  dumps ("\n");
}

void
gc_dump_state ()
{
  gc_dump_register ("M0", M0);
  gc_dump_register ("M1", M1);
  gc_dump_register ("g_symbols", g_symbols);
  gc_dump_register ("g_symbol_max", g_symbol_max);
  gc_dump_register ("g_ports", g_ports);
  gc_dump_register ("cell_nil", cell_nil);
}

void
gc_dump_arena (struct scm *cells, long size)
{
  struct scm *end = g_cells + (size * M2_CELL_SIZE);
  struct scm *dist = cells;
  int i;
  long t;
  long a;
  long d;
  int c;
  char* p;
  if (g_dump_filedes == 0)
    g_dump_filedes = mes_open ("dump.mo", O_CREAT|O_WRONLY, 0644);
  dumps ("stack="); dumps (ltoa (g_stack)); dumpc ('\n');
  dumps ("size="); dumps (ltoa (size)); dumpc ('\n');
  gc_dump_state ();
  while (end->type == 0 && end->car == 0 && end->cdr == 0)
    {
      end = end - M2_CELL_SIZE;
      size = size - 1;
    }
  while (size > 0)
    {
      for (i=0; i < 16; i = i + 1)
        {
          t = cells->type;
          a = cells->car_value;
          d = cells->cdr_value;
          if (size == 0)
            dumps ("0 0 0");
          else
            {
              dumps (ltoa (t));
              dumpc (' ');
              if (t == TPAIR
                  || t == TBINDING)
                {
                  dumps (ltoa ((cells->car - dist) / M2_CELL_SIZE));
                  /* dumps ("["); dumps (ltoa (a)); dumps ("]"); */
                }
              else
                dumps (ltoa (a));
              dumpc (' ');
              if (t == TBYTES
                  || t == TSTRUCT
                  || t == TVECTOR)
                {
                  if (t == TBYTES)
                    c = bytes_cells (a);
                  else
                    c = bytes_cells (vector_bytes (cells));
                  p = cell_bytes (cells);
                  size = size - c;
                  dumpc ('"');
                  while (a > 0)
                    {
                      if (p[0] != 0)
                        dumpc (p[0]);
                      p = p + 1;
                      a = a - 1;
                    }
                  dumpc ('"');
                  cells = cells + c * M2_CELL_SIZE;
                  size = size - c;
                }
              else if (t == TCONTINUATION
                       || t == TKEYWORD
                       || t == TPAIR
                       || t == TPORT
                       || t == TSPECIAL
                       || t == TSTRING
                       || t == TSYMBOL
                       || t == TVALUES)
                {
                  dumps (ltoa ((cells->cdr - dist) / M2_CELL_SIZE));
                  /* dumps ("["); dumps (ltoa (d)); dumps ("]"); */
                }
              else if (t == TNUMBER)
                {
                  if ( d > 1000 )
                    dumps (ltoa (1001));
                  else
                    dumps (ltoa (d));
                }
              else
                {
                  cells = cells + M2_CELL_SIZE;
                  size = size - 1;
                }
            }
          if (i != 15)
            dumps ("  ");
          else
            dumpc ('\n');
        }
      dumpc ('\n');
    }
}
