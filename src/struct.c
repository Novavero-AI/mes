/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2018,2019,2020,2022 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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

struct scm *
make_struct (struct scm *type, struct scm *fields, struct scm *printer)
{
  /* Structs are just a vector, with a different type */
  size_t size = length__ (fields) + 2;
  struct scm *x = make_vector_uninit_ (size);
  x->type = TSTRUCT;

  struct scm **vb= vector_body (x);
  struct scm **loc;
  loc = vb + 0 * M2_PTR_SIZE; *loc = type;    /* `0` is the type */
  loc = vb + 1 * M2_PTR_SIZE; *loc = printer; /* `1` is the printer */
  long i;
  struct scm *e;
  for (i = 2; i < size; i = i + 1)            /* `i` are the actual values */
    {
      e = cell_unspecified;
      if (fields != cell_nil)
        {
          e = fields->car;
          fields = fields->cdr;
        }
      loc = vb + i * M2_PTR_SIZE; *loc = e;
    }
  return x;
}

struct scm *
struct_length (struct scm *x)
{
  if (x->type != TSTRUCT)
    error (cell_symbol_wrong_type_arg, cons (x, cstring_to_symbol ("struct-length")));
  return make_number (x->length);
}

struct scm *
struct_ref_ (struct scm *x, long i)
{
  if (x->type != TSTRUCT)
    error (cell_symbol_wrong_type_arg, cons (x, cstring_to_symbol ("struct-ref")));
  if (i >= x->length)
    error (cell_symbol_out_of_range, cons (make_number (i), cstring_to_symbol ("struct-ref")));
  struct scm **loc = vector_body (x) + i * M2_PTR_SIZE;
  return *loc;
}

struct scm *
struct_set_x_ (struct scm *x, long i, struct scm *e)
{
  if (x->type != TSTRUCT)
    error (cell_symbol_wrong_type_arg, cons (x, cstring_to_symbol ("struct-set!")));
  if (i >= x->length)
    error (cell_symbol_out_of_range, cons (make_number (i), cstring_to_symbol ("struct-set!")));
  struct scm **loc = vector_body (x) + i * M2_PTR_SIZE;
  *loc = e;
  return cell_unspecified;
}

struct scm *
struct_ref (struct scm *x, struct scm *i)
{
  return struct_ref_ (x, i->value);
}

struct scm *
struct_set_x (struct scm *x, struct scm *i, struct scm *e)
{
  return struct_set_x_ (x, i->value, e);
}
