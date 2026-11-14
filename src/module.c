/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2018,2019,2022 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
 * Copyright © 2022 Timothy Sample <samplet@ngyro.com>
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

/** Commentary:
    Essential module system interface and initial module definition for
    bootstrapping. See: mes/module/mes/guile-module.mes
 */

/** Code: */

#include "mes/lib.h"
#include "mes/mes.h"

enum module_fields
{
  MODULE_OBARRAY = 3,
  MODULE_USES,
  MODULE_BINDER,
  MODULE_EVAL_CLOSURE,
  MODULE_TRANSFORMER,
  MODULE_NAME,
  MODULE_KIND,
  MODULE_DUPLICATES_HANDLERS,
  MODULE_DUPLICATES_INTERFACE,
  MODULE_OBSERVERS,
  MODULE_WEAK_OBSERVERS,
  MODULE_OBSERVER_ID
};

struct scm *
make_module_type ()     /*:((internal)) */
{
  if (scm_module_type == 0)
    {
      struct scm *type_name = cstring_to_symbol ("<module>");
      struct scm *fields = cell_nil;
      fields = cons (cstring_to_symbol ("observer-id"), fields);
      fields = cons (cstring_to_symbol ("weak-observers"), fields);
      fields = cons (cstring_to_symbol ("observers"), fields);
      fields = cons (cstring_to_symbol ("duplicates-interface"), fields);
      fields = cons (cstring_to_symbol ("duplicates-handlers"), fields);
      fields = cons (cstring_to_symbol ("kind"), fields);
      fields = cons (cstring_to_symbol ("name"), fields);
      fields = cons (cstring_to_symbol ("transformer"), fields);
      fields = cons (cstring_to_symbol ("eval-closure"), fields);
      fields = cons (cstring_to_symbol ("binder"), fields);
      fields = cons (cstring_to_symbol ("uses"), fields);
      fields = cons (cstring_to_symbol ("obarray"), fields);

      fields = cons (fields, cell_nil);     /* type-field */
      fields = cons (cell_f, fields);       /* type-printer */
      fields = cons (type_name, fields);    /* type-name */
      scm_module_type = make_struct (cell_symbol_record_type, fields, cell_unspecified);
    }
  return scm_module_type;
}


struct scm *
make_initial_module (struct scm *a)     /*:((internal)) */
{
  struct scm *var;
  struct scm *obarray = make_hash_table_ (100);
  while (a->type == TPAIR)
    {
      var = make_variable (a->car->cdr);
      hashq_set_x (obarray, a->car->car, var);
      a = a->cdr;
    }

  struct scm *name = cstring_to_symbol ("initial-module");

  struct scm *values = cell_nil;
  values = cons (make_number (0),  values);       /* observer-id */
  values = cons (make_hash_table_ (1), values);   /* weak-observers */
  values = cons (cell_f,  values);                /* observers */
  values = cons (cell_f,  values);                /* duplicates-interface */
  values = cons (cell_f,  values);                /* duplicates-handlers */
  values = cons (cell_f,  values);                /* kind */
  values = cons (name,    values);                /* name */
  values = cons (cell_f,  values);                /* transformer */
  values = cons (cell_f,  values);                /* eval-closure */
  values = cons (cell_f,  values);                /* binder */
  values = cons (cell_nil,values);                /* uses */
  values = cons (obarray, values);                /* obarray */
  values = cons (cstring_to_symbol ("module"), values);

  return make_struct (scm_module_type, values, cell_nil);
}

struct scm *
initial_module ()
{
  return M0;
}

struct scm *
current_module ()
{
  if (M1 == cell_f)
    return initial_module ();
  else
    return M1;
}

struct scm *
set_current_module (struct scm *module)
{
  struct scm *previous = M1;
  M1 = module;
  return previous;
}

struct scm *
module_local_variable (struct scm *module, struct scm *name)
{
  struct scm *obarray = struct_ref_ (module, MODULE_OBARRAY);
  return hashq_ref_ (obarray, name, cell_f);
}

struct scm *
module_ensure_local_variable_x (struct scm *module, struct scm *name)
{
  /* Ensure that there is a local variable in MODULE for SYMBOL.  If
   * there is no binding for SYMBOL, create a new uninitialized
   * variable.  Return the local variable.
   */
  struct scm *var = module_local_variable (module, name);
  if (var != cell_f)
    return var;

  struct scm *obarray = struct_ref_ (module, MODULE_OBARRAY);
  var = make_variable (cell_undefined);
  struct scm *handle = hashq_create_handle_x (obarray, name, var);
  return handle->cdr;
}

struct scm *
module_variable (struct scm *module, struct scm *name)
{
  struct scm *modules = cons (module, cell_nil);
  struct scm *variable;
  struct scm *uses;
  while (modules->type == TPAIR)
    {
      module = modules->car;
      variable = module_local_variable (module, name);
      if (variable != cell_f)
          return variable;

      uses = struct_ref_ (module, MODULE_USES);
      modules = append2 (uses, modules->cdr);
    }
  return cell_f;
}

struct scm *
module_define_x (struct scm *module, struct scm *name, struct scm *value)
{
  struct scm *variable = module_ensure_local_variable_x (module, name);
  return variable_set_x (variable, value);
}
