/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2016,2017,2018,2019,2020,2022 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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

#include "mes/lib.h"
#include "mes/mes.h"

struct scm *
make_vector_uninit_ (long k)
{
  size_t size = bytes_cells (sizeof (struct scm *) * k);
  struct scm *v = alloc (size);
  v->type = TVECTOR;
  v->length = k;
  return v;
}

struct scm *
make_vector_ (long k, struct scm *e)
{
  struct scm *x = make_vector_uninit_ (k);
  vector_fill_x_ (x, e, 0, k);
  return x;
}

struct scm *
make_vector (struct scm *x)               /*:((arity . n)) */
{
  struct scm *k = x->car;
  if (k->type != TNUMBER)
    error (cell_symbol_wrong_type_arg,
           cons (k, cstring_to_symbol ("make-vector")));
  long n = k->value;
  struct scm *e = cell_unspecified;
  if (x->cdr != cell_nil)
    e = x->cdr->car;

  return make_vector_ (n, e);
}

struct scm *
vector_length (struct scm *x)
{
  if (x->type != TVECTOR)
    error (cell_symbol_wrong_type_arg,
           cons (x, cstring_to_symbol ("vector-length")));
  return make_number (x->length);
}

struct scm *
vector_ref_ (struct scm *x, long i)
{
  if (x->type != TVECTOR)
    error (cell_symbol_wrong_type_arg,
           cons (x, cstring_to_symbol ("vector-ref")));
  if (i >= x->length)
    error (cell_symbol_out_of_range,
           cons (make_number (i), cstring_to_symbol ("vector-ref")));
  struct scm **loc = vector_body (x) + i * M2_PTR_SIZE;
  return *loc;
}

struct scm *
vector_ref (struct scm *x, struct scm *i)
{
  return vector_ref_ (x, i->value);
}

struct scm *
vector_set_x_ (struct scm *x, long i, struct scm *e)
{
  if (x->type != TVECTOR)
    error (cell_symbol_wrong_type_arg,
           cons (x, cstring_to_symbol ("vector-set!")));
  if (i >= x->length)
    error (cell_symbol_out_of_range,
           cons (make_number (i), cstring_to_symbol ("vector-set!")));
  struct scm **loc = vector_body (x) + i * M2_PTR_SIZE;
  *loc = e;
  return cell_unspecified;
}

struct scm *
vector_set_x (struct scm *x, struct scm *i, struct scm *e)
{
  return vector_set_x_ (x, i->value, e);
}

struct scm *
list_to_vector (struct scm *x)
{
  struct scm *v = make_vector_ (length__ (x), cell_unspecified);
  size_t i = 0;
  while (x != cell_nil)
    {
      vector_set_x_ (v, i, car (x));
      i = i + 1;
      x = cdr (x);
    }
  return v;
}

struct scm *
vector_to_list (struct scm *v)
{
  struct scm *x = cell_nil;
  long i;
  for (i = v->length; i; i = i - 1)
    x = cons (vector_ref_ (v, i - 1), x);
  return x;
}

struct scm *
vector_fill_x_ (struct scm *vec, struct scm *fill, long start, long end)
{
  if (start > end)
    error (cell_symbol_out_of_range,
           cons (make_number (start), cstring_to_symbol ("vector-fill!")));
  if (vec->length < end)
    error (cell_symbol_out_of_range,
           cons (make_number (end), cstring_to_symbol ("vector-fill!")));
  long i;
  for (i = start; i < end; i = i + 1)
    vector_set_x_ (vec, i, fill);
  return cell_unspecified;
}

struct scm *
vector_fill_x (struct scm *x)               /*:((arity . n)) */
{
  /* Arguments */
  struct scm *vec;
  struct scm *fill;
  struct scm *start; /* optional */
  struct scm *end;   /* optional */

  vec = x->car;
  if (vec->type != TVECTOR)
    error (cell_symbol_wrong_type_arg,
           cons (vec, cstring_to_symbol ("vector-fill!")));
  x = x->cdr;

  if (x->type != TPAIR)
    error (cell_symbol_wrong_number_of_args,
           cons (make_number (length__(x)), cstring_to_symbol ("vector-fill!")));
  fill = x->car;
  x = x->cdr;

  if (x->type != TPAIR)
    return vector_fill_x_ (vec, fill, 0, vec->length);
  start = x->car;
  if (start->type != TNUMBER)
    error (cell_symbol_wrong_type_arg,
           cons (start, cstring_to_symbol ("vector-fill!")));
  x = x->cdr;

  if (x->type != TPAIR)
    return vector_fill_x_ (vec, fill, start->value, vec->length);
  end = x->car;
  if (end->type != TNUMBER)
    error (cell_symbol_wrong_type_arg,
           cons (end, cstring_to_symbol ("vector-fill!")));

  return vector_fill_x_ (vec, fill, start->value, end->value);
}

struct scm *
vector_copy_x_ (struct scm *vec, long start, struct scm *source, long begin,
              long end)
{
  if (vec->length - start < end - begin)
    error (cell_symbol_out_of_range,
           cons (list3 (make_number (start), make_number (begin), make_number (end)),
                 cstring_to_symbol ("vector-copy!")));

  long i;
  for (i = begin; i < end; i = i + 1)
    vector_set_x_ (vec, i - begin + start, vector_ref_ (source, i));
  return cell_unspecified;
}

struct scm *
vector_copy_x (struct scm *x)               /*:((arity . n)) */
{
  /* Arguments */
  struct scm *vec;
  struct scm *start;
  struct scm *source;
  struct scm *begin; /* optional */
  struct scm *end;   /* optional */

  if (x->type != TPAIR)
    error (cell_symbol_wrong_number_of_args,
           cons (make_number (length__(x)), cstring_to_symbol ("vector-copy!")));

  vec = x->car;
  if (vec->type != TVECTOR)
    error (cell_symbol_wrong_type_arg, cons (vec, cstring_to_symbol ("vector-copy!")));

  x = x->cdr;
  if (x->type != TPAIR)
    error (cell_symbol_wrong_number_of_args,
           cons (make_number (length__(x)), cstring_to_symbol ("vector-copy!")));

  start = x->car;
  if (start->type != TNUMBER)
    error (cell_symbol_wrong_type_arg, cons (start, cstring_to_symbol ("vector-copy!")));

  x = x->cdr;
  if (x->type != TPAIR)
    error (cell_symbol_wrong_number_of_args,
           cons (make_number (length__(x)), cstring_to_symbol ("vector-copy!")));

  source = x->car;
  if (source->type != TVECTOR)
    error (cell_symbol_wrong_type_arg, cons (source, cstring_to_symbol ("vector-copy!")));

  x = x->cdr;
  if (x->type != TPAIR)
    return vector_copy_x_ (vec, start->value, source, 0, source->length);

  begin = x->car;
  if (begin->type != TNUMBER)
    error (cell_symbol_wrong_type_arg, cons (begin, cstring_to_symbol ("vector-copy!")));

  x = x->cdr;
  if (x->type != TPAIR)
    return vector_copy_x_ (vec, start->value, source, begin->value, source->length);

  end = x->car;
  if (end->type != TNUMBER)
    error (cell_symbol_wrong_type_arg, cons (end, cstring_to_symbol ("vector-copy!")));

  return vector_copy_x_ (vec, start->value, source, begin->value, end->value);
}
