/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2025, 2026 Ekaitz Zarraga <ekaitz@elenq.tech>
 * Copyright © 2026 Timothy Sample <samplet@ngyro.com>
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
    This a scheme compiler. It takes Scheme code that is already expanded to a
    reduced set of Scheme constructs and creates bytecode from it.

    The entry point for the compiler are procedures and `template`s are
    returned.

    Procedures may have internal procedures. Those are compiled to templates
    and the template is registered as a literal inside of the parent
    procedure's template's literal list.

    Templates are never exposed to Scheme directly, but Closures are. Closures
    are Scheme values that hold an `environment` and a `template` together.

    TODO: We could make templates also contain original line numbers, the
    source code of the procedure and a few extra things.
 */

/** Code: */

#include "mes/mes.h"
#include "mes/lib.h"
#include <string.h>

/* Helper */
enum scope
{
  SCOPE_LOCAL,
  SCOPE_OUTER,
  SCOPE_GLOBAL,
};

struct scm *create_id (char *base);
struct scm *get_global_binding (struct scm *mod, struct scm *name);
struct scm *get_global_definition_binding (struct scm *mod, struct scm *name);
struct scm *get_defined_names (struct scm *body);

/* Template */
struct scm *make_template_type ();
struct scm *make_template_ (struct scm *name, struct scm *parent, struct scm* mod, struct scm* source);
struct scm *template_p (struct scm *template);
struct scm *template_name (struct scm *template);
struct scm *template_arity (struct scm *template);
struct scm *template_source (struct scm *template);
struct scm *template_module (struct scm *template);
struct scm *template_code (struct scm *template);
struct scm *template_literals (struct scm *template);
struct scm *template_locals (struct scm *template);
struct scm *template_arguments (struct scm *template);
void template_clear (struct scm *template); /* TODO: change */
void template_set_arity (struct scm *template, struct scm *args);

void template_set_locals (struct scm *template, struct scm *args, struct scm *defines);
size_t template_local_id (struct scm *template, struct scm *name);

size_t template_register_free_var (struct scm *template, struct scm *name);
size_t template_register_literal (struct scm *template, struct scm *literal);

struct scm *template_ref_literal (struct scm *template, size_t literal_id);
struct scm *template_ref_local (struct scm *template, size_t local_id);
struct scm *template_ref_free_var (struct scm *template, size_t free_id);

enum scope template_symbol_scope (struct scm *template, struct scm *name);
struct scm *template_lookup_keyword (struct scm *template, struct scm *exp);

size_t template_emit_ubyte (struct scm *template, unsigned char ubyte);
size_t template_emit_ushort (struct scm *template, unsigned short ushort);
void template_patch_target (struct scm *template, size_t where, unsigned short to);

#define template_emit_inst        template_emit_ubyte
#define template_emit_argno       template_emit_ushort
#define template_emit_literal     template_emit_ushort
#define template_emit_target      template_emit_ushort
#define template_emit_variable_id template_emit_ushort

/* Compilation */
size_t template_emit_special (struct scm *tpl, struct scm *exp);
size_t template_emit_sequence (struct scm *tpl, struct scm *exp, int tail);
size_t template_emit_lookup (struct scm *tpl, struct scm *exp);
size_t template_emit_definition_lookup (struct scm *tpl, struct scm *exp);
size_t template_emit_capture (struct scm *tpl, struct scm *internal);
size_t template_emit_exp (struct scm *tpl, struct scm *exp, int tail);
size_t template_emit_form (struct scm *tpl, struct scm *exp, int tail);

/* High level interface */
struct scm *compile_procedure_internal (struct scm *exp, struct scm *name,
                                        struct scm *parent, struct scm *mod,
                                        int top_level);
struct scm *compile_procedure (struct scm *exp, struct scm *name, struct scm *mod);
struct scm *compile_top_level_exp (struct scm *exp, struct scm *mod);

/* Disassembler */
void print_disassembly_line (int pc, unsigned char raw_byte, char *txt, char *suffix);
char *inst_to_string (enum opcode inst);
void eprint_ptr (void *p);
void template_disassemble (struct scm *templ);

/* Builtins */
struct scm *procedure_name (struct scm *closure);
struct scm *procedure_arguments (struct scm *closure);
struct scm *procedure_disassemble (struct scm *closure);


/* Template */
size_t template_id = 0;
char template_name_buf [128];

struct scm *
create_id (char *base)
{
  size_t len = strlen (base);
  assert_msg (len < 100, "create_id: Base name too long");
  memcpy (template_name_buf, base, strlen (base));
  char * num = ltoa (template_id);
  template_id += 1;
  char *t = template_name_buf + len;
  *t = '#';
  memcpy (template_name_buf + len + 1, num, strlen (num));
  return cstring_to_symbol (template_name_buf);
}

enum templ_fields
{
  TEMPL_TYPE = 0,
  TEMPL_NAME = 2,
  TEMPL_ARITY,
  TEMPL_LITERALS,
  TEMPL_LITERALS_USED,
  TEMPL_CODE,
  TEMPL_CODE_USED,
  TEMPL_LOCALS,
  TEMPL_FREE_VARS_USED,
  TEMPL_FREE_VARS,
  TEMPL_PARENT,
  TEMPL_SOURCE,
  TEMPL_MODULE
};

struct scm *
make_template_type ()     /*:((internal)) */
{
  if (scm_template_type == 0)
    {
      struct scm *fields = cell_nil;
      fields = cons (cstring_to_symbol ("module"), fields);
      fields = cons (cstring_to_symbol ("source"), fields);
      fields = cons (cstring_to_symbol ("parent"), fields);
      fields = cons (cstring_to_symbol ("free-vars-used"), fields);
      fields = cons (cstring_to_symbol ("free-vars"), fields);
      fields = cons (cstring_to_symbol ("locals"), fields);
      fields = cons (cstring_to_symbol ("code-used"), fields);
      fields = cons (cstring_to_symbol ("code"), fields);
      fields = cons (cstring_to_symbol ("literals-used"), fields);
      fields = cons (cstring_to_symbol ("literals"), fields);
      fields = cons (cstring_to_symbol ("arity"), fields);
      fields = cons (cstring_to_symbol ("name"), fields);
      fields = cons (fields, cell_nil);
      fields = cons (cstring_to_symbol ("template"), fields);
      scm_template_type = make_struct (cell_symbol_record_type, fields, cell_unspecified);
    }
  return scm_template_type;
}
struct scm *
make_template_ (struct scm *name, struct scm *parent, struct scm *mod, struct scm *source)  /*:((internal)) */
{
  struct scm *template_type = make_template_type ();
  struct scm *sym = cstring_to_symbol ("template");
  struct scm *values = cell_nil;
  values = cons (mod, values);                                    /* module */
  values = cons (source, values);                                 /* source */
  values = cons (parent, values);                                 /* parent */
  values = cons (make_vector_ (16, cell_unspecified), values);    /* free-vars */
  values = cons (make_number (0), values);                        /* free-vars-used */
  values = cons (cell_nil, values);                               /* locals */
  values = cons (make_number (0), values);                        /* code-used */
  values = cons (make_bytevector_ (128, OP_UNREACHABLE), values); /* code */
  values = cons (make_number (0), values);                        /* literals-used */
  values = cons (make_vector_ (16, cell_unspecified), values);    /* literals */
  values = cons (make_number (0), values);                        /* arity */
  values = cons (name, values);                                   /* name */
  return make_struct (template_type, values, sym);
}

struct scm *
template_p (struct scm *template)
{
  if (template->type == TSTRUCT)
    {
      if (struct_ref_ (template, TEMPL_TYPE) == scm_template_type)
        {
          return cell_t;
        }
    }
  return cell_f;
}
struct scm *
template_name (struct scm *template)
{
  struct_ref_ (template, TEMPL_NAME);
}
struct scm *
template_arity (struct scm *template)
{
  struct_ref_ (template, TEMPL_ARITY);
}
struct scm *
template_source (struct scm *template)
{
  struct_ref_ (template, TEMPL_SOURCE);
}
struct scm *
template_module (struct scm *template)
{
  struct_ref_ (template, TEMPL_MODULE);
}
struct scm *
template_code (struct scm *template)
{
  return struct_ref_ (template, TEMPL_CODE);
}
struct scm *
template_literals (struct scm *template)
{
  return struct_ref_ (template, TEMPL_LITERALS);
}

void
template_clear (struct scm *template)
{
  /* Clears all the unneeded values */
  /* TODO: We shouldn't use the same data structure for compilation and scheme
   * usage because we need some data at compilation that is not needed later.
   * This function just packs the data, but we should find a better way to do
   * this, so we can remove bunch of the fields and provide a faster data
   * structure.
   * As closure are a very specific case, we could try to write them in a C
   * struct that uses type punning to struct scm.
   */
  struct_set_x_ (template, TEMPL_PARENT, cell_nil);

  long lit_used = struct_ref_ (template, TEMPL_LITERALS_USED)->value;
  struct scm *literals = struct_ref_ (template, TEMPL_LITERALS);
  struct scm *new_literals = make_vector_uninit_(lit_used);
  vector_copy_x_ (new_literals, 0, literals, 0, lit_used);
  struct_set_x_ (template, TEMPL_LITERALS, new_literals);

  size_t code_used = struct_ref_ (template, TEMPL_CODE_USED)->value;
  struct scm *code = struct_ref_ (template, TEMPL_CODE);
  struct scm *new_code = make_bytevector_uninit_ (code_used);
  char *old = cell_bytes (code);
  char *new = cell_bytes (new_code);
  size_t i;
  for (i = 0; i < code_used; i++)
    new[i] = old[i];
  struct_set_x_ (template, TEMPL_CODE, new_code);

  long free_used = struct_ref_ (template, TEMPL_FREE_VARS_USED)->value;
  struct scm *free = struct_ref_ (template, TEMPL_FREE_VARS);
  struct scm *new_free = make_vector_uninit_(free_used);
  vector_copy_x_ (new_free, 0, free, 0, free_used);
  struct_set_x_ (template, TEMPL_FREE_VARS, new_free);
}

void
template_set_arity (struct scm *template, struct scm *args)   /*:((internal)) */
{
  /* Reverse, count and normalize the arguments */
  long arity = 0;
  struct scm *cur = args;
  for (args = cell_nil; cur != cell_nil; cur = cur->cdr)
    {
      arity++;
      if (cur->type != TPAIR) /* End of the improper list or args is just a symbol */
        {
          /* We represent N-arity with a negative number.
           * The magnitude represents the amount of formal arguments it has,
           * i.e. the amount of definitions the preamble makes.
           */
          arity = -arity;
          break;
        }
    }
  struct_set_x_ (template, TEMPL_ARITY, make_number (arity));
}

size_t
template_register_literal (struct scm *template, struct scm *literal)  /*:((internal)) */
{
  long lit_used = struct_ref_ (template, TEMPL_LITERALS_USED)->value;
  struct scm *literals = struct_ref_ (template, TEMPL_LITERALS);

  /* Avoid repetition */
  size_t i;
  for (i = 0; i < lit_used ; i = i + 1)
    if (literal == vector_ref_ (literals, i))
      return i;

  if (lit_used >= literals->length)
    {
      struct scm *new_literals = make_vector_ (literals->length * 2, cell_unspecified);
      /* TODO: optimize iterations with vector-fill! */
      vector_copy_x_ (new_literals, 0, literals, 0, literals->length);
      struct_set_x_ (template, TEMPL_LITERALS, new_literals);
      literals = new_literals;
    }
  vector_set_x_ (literals, lit_used, literal);
  struct_set_x_ (template, TEMPL_LITERALS_USED, make_number (lit_used + 1));
  return lit_used;
}

struct scm *
template_locals (struct scm *template)
{
  return struct_ref_ (template, TEMPL_LOCALS);
}

struct scm *
template_arguments (struct scm *template)
{
  struct scm *locals = template_locals (template);
  long arity = template_arity (template)->value;
  if (arity < 0)
    arity = -arity;

  struct scm *args = cell_nil;
  for (arity = arity - 1; arity >= 0; arity = arity - 1)
    args = cons (template_ref_local (template, arity), args);
  return args;
}

size_t
template_local_id (struct scm *template, struct scm *name)
{
  struct scm *locals = struct_ref_ (template, TEMPL_LOCALS);
  size_t i, l = locals->length;
  for (i = 0; i < l; i++)
    if (vector_ref_ (locals, i) == name)
      return i;
  assert_msg (0, "Unreachable");
  return 0;
}

struct scm *
template_ref_local (struct scm *template, size_t local_id)
{
  struct scm *locals = struct_ref_ (template, TEMPL_LOCALS);
  vector_ref_ (locals, local_id);
}

void
template_set_locals (struct scm *template, struct scm *args, struct scm *defines)
{
  /* This vector of symbols tracks the positions we use later to refer to each
   * variable.
   * It is also used when the procedure is called to create its environment in
   * the top of the stack. Arity is used to prepare the arguments that we
   * already have on the stack, and the locals are made pushing bindings on top
   * of the stack.
   */
  /* TODO: Repeated names? */
  /* TODO: This is repeated from set_arity but it's not an issue now */
  struct scm *cur = args;
  for (args = cell_nil; cur != cell_nil; cur = cur->cdr)
    {
      if (cur->type == TPAIR)
        args = cons (cur->car, args);
      else /* End of the improper list or args is just a symbol */
        {
          /* We represent N-arity with a negative number.
           * The magnitude represents the amount of formal arguments it has,
           * i.e. the amount of definitions the preamble makes.
           */
          args = cons (cur, args);
          break;
        }
    }
  /* Re-Reverse the args */
  /* TODO: Simplify me */
  cur = args;
  for (args = cell_nil; cur != cell_nil; cur = cur->cdr)
    args = cons (cur->car, args);
  struct scm *v = make_vector_ (length__ (args) + length__ (defines), cell_unspecified);
  long i = 0;
  for (cur = args; cur != cell_nil; cur = cur->cdr)
    {
      vector_set_x_ (v, i, cur->car);
      i = i + 1;
    }
  for (cur = defines; cur != cell_nil; cur = cur->cdr)
    {
      vector_set_x_ (v, i, cur->car);
      i = i + 1;
    }
  struct_set_x_ (template, TEMPL_LOCALS, v);
}

enum scope
template_symbol_scope (struct scm *template, struct scm *name)
{
  struct scm *locals = struct_ref_ (template, TEMPL_LOCALS);
  long i;
  for (i = 0; i < locals->length; i = i + 1)
    if (vector_ref_ (locals, i) == name)
      return SCOPE_LOCAL;

  struct scm * parent_template = struct_ref_ (template, TEMPL_PARENT);
  if (parent_template == cell_nil)
    return SCOPE_GLOBAL;

  enum scope s = template_symbol_scope (parent_template, name);
  if (SCOPE_LOCAL == s)
    return SCOPE_OUTER;
  return s;
}

struct scm *
template_lookup_keyword (struct scm *tpl, struct scm *exp)
{
  struct scm *keyword = cell_f;
  if (exp->type == TPAIR)
    {
      keyword = exp->car;
      if (keyword->type == TSYMBOL)
        {
          if (template_symbol_scope (tpl, keyword) == SCOPE_GLOBAL)
            {
              struct scm *variable = module_variable (template_module (tpl), keyword);
              if (variable != cell_f)
                keyword = variable_ref (variable);
            }
        }
    }
  return keyword;
}

struct scm *
template_ref_literal (struct scm *template, size_t literal_id)
{
  struct scm *literals = struct_ref_ (template, TEMPL_LITERALS);
  return vector_ref_ (literals, literal_id);
}

size_t
template_emit_ubyte (struct scm *template, unsigned char byte)  /*:((internal)) */
{
  size_t code_used = struct_ref_ (template, TEMPL_CODE_USED)->value;
  struct scm *code = struct_ref_ (template, TEMPL_CODE);

  if (code_used >= code->length)
    {
      size_t old_l = code->length;
      size_t new_l = old_l * 2;
      struct scm *new_code = make_bytevector_uninit_ (new_l);
      char *old = cell_bytes (code);
      char *new = cell_bytes (new_code);

      size_t i;
      for (i = 0; i < old_l; i++)
        new[i] = old[i];
      for (; i < new_l; i++)
        new[i] = OP_UNREACHABLE;

      struct_set_x_ (template, TEMPL_CODE, new_code);
      code = new_code;
    }
  unsigned char *c = cell_bytes (code);
  unsigned char *where = c + code_used;
  *where = byte;
  code_used = code_used + 1;
  struct_set_x_ (template, TEMPL_CODE_USED, make_number (code_used));
  return code_used;
}

size_t
template_emit_ushort (struct scm *template, unsigned short sh)  /*:((internal)) */
{
  size_t code_used = struct_ref_ (template, TEMPL_CODE_USED)->value;
  struct scm *code = struct_ref_ (template, TEMPL_CODE);

  if (code_used + 3 >= code->length)
    {
      size_t old_l = code->length;
      size_t new_l = old_l * 2;
      struct scm *new_code = make_bytevector_uninit_ (new_l);
      char *old = cell_bytes (code);
      char *new = cell_bytes (new_code);

      size_t i;
      for (i = 0; i < old_l; i++)
        new[i] = old[i];
      for (; i < new_l; i++)
        new[i] = OP_UNREACHABLE;

      struct_set_x_ (template, TEMPL_CODE, new_code);
      code = new_code;
    }
  unsigned char *c = cell_bytes (code);

  /* Align to short */
  unsigned char rem = code_used & 1;
  code_used += rem;
  unsigned char *where = c + code_used;
  unsigned short *where_short = (unsigned short *)where;
  *where_short = sh;
  code_used = code_used + 2;
  struct_set_x_ (template, TEMPL_CODE_USED, make_number (code_used));
  return code_used;
}

void
template_patch_target (struct scm *template, size_t wh, unsigned short to)  /*:((internal)) */
{
  struct scm *code = struct_ref_ (template, TEMPL_CODE);
  unsigned char *c = cell_bytes (code);
  unsigned char *where = c + wh;
  where += wh & 1; /* Align to short */
  unsigned short *where_short = (unsigned short *)where;
  *where_short = to;
}

/* Compilation */
size_t
template_emit_special (struct scm *tpl, struct scm *exp)  /*:((internal)) */
{
  char inst = OP_UNREACHABLE;
  if (exp == cell_f)
    inst = OP_FALSE;
  else if (exp == cell_t)
    inst = OP_TRUE;
  else if (exp == cell_nil)
    inst = OP_NIL;
  else if (exp == cell_unspecified)
    inst = OP_UNSPECIFIED;
  else
    {
      template_emit_inst (tpl, OP_LITERAL);
      return template_emit_literal (tpl, template_register_literal (tpl, exp));
    }
  return template_emit_inst (tpl, inst);
}

size_t
template_emit_sequence (struct scm *tpl, struct scm *exp, int tail)   /*:((internal)) */
{
  struct scm *cur;
  size_t end = 0;
  int is_last = 0;
  for (cur = exp; cur != cell_nil; cur = cur->cdr)
    {
      is_last = cur->cdr == cell_nil;
      end = template_emit_exp (tpl, cur->car, is_last && tail);
      if (is_last != 1)
        template_emit_inst (tpl, OP_DROP);
    }
  return end;
}

struct scm *
get_global_definition_binding (struct scm *mod, struct scm *name)
{
  struct scm *variable = module_ensure_local_variable_x (mod, name);
  return make_binding (name, variable);
}

struct scm *
get_global_binding (struct scm *mod, struct scm *name)
{
  /* Let's see if it's already defined or imported */
  struct scm *variable = module_variable (mod, name);
  /* If it's not defined anywhere yet, we hope it will be defined later in the
   * current module.
   */
  if (variable == cell_f)
    variable = module_ensure_local_variable_x (mod, name);
  return make_binding (name, variable);
}

size_t
template_emit_definition_lookup (struct scm *tpl, struct scm *exp)
{
  switch (template_symbol_scope (tpl, exp))
    {
    case SCOPE_LOCAL:
      template_emit_inst (tpl, OP_REF_LOCAL);
      return template_emit_variable_id (tpl, template_local_id (tpl, exp));
    case SCOPE_OUTER:
      /* NOTE: this means we are defining something that is not pre-processed
       * as a local define, meaning the (define ...) expression is not properly
       * placed.
       */
      assert_msg (0, "Unreachable");
    case SCOPE_GLOBAL:
      {
        struct scm *binding = get_global_definition_binding (template_module (tpl), exp);
        template_emit_inst (tpl, OP_LITERAL);
        return template_emit_literal (tpl, template_register_literal (tpl, binding));
      }
    }
}
size_t
template_emit_lookup (struct scm *tpl, struct scm *exp)
{
  switch (template_symbol_scope (tpl, exp))
    {
    case SCOPE_LOCAL:
      template_emit_inst (tpl, OP_REF_LOCAL);
      return template_emit_variable_id (tpl, template_local_id (tpl, exp));
    case SCOPE_OUTER:
      template_emit_inst (tpl, OP_REF_CLOSURE);
      return template_emit_variable_id (tpl, template_register_free_var (tpl, exp));
    case SCOPE_GLOBAL:
      {
        struct scm *binding = get_global_binding (template_module (tpl), exp);
        template_emit_inst (tpl, OP_LITERAL);
        return template_emit_literal (tpl, template_register_literal (tpl, binding));
      }
    }
}

size_t
template_register_free_var (struct scm *template, struct scm *name)
{
  /* TODO: The code is copy-pasted from the register_literal procedure, but
   * it's ok for now
   */
  long fv_used = struct_ref_ (template, TEMPL_FREE_VARS_USED)->value;
  struct scm *fv = struct_ref_ (template, TEMPL_FREE_VARS);

  /* Avoid repetition */
  size_t i;
  for (i = 0; i < fv_used; i = i + 1)
    if (name == vector_ref_ (fv, i))
      return i;

  if (fv_used >= fv->length)
    {
      struct scm *new_fv = make_vector_ (fv->length * 2, cell_unspecified);
      // TODO: optimize iterations with vector-fill!
      vector_copy_x_ (new_fv, 0, fv, 0, fv->length);
      struct_set_x_ (template, TEMPL_FREE_VARS, new_fv);
      fv = new_fv;
    }
  vector_set_x_ (fv, fv_used, name);
  struct_set_x_ (template, TEMPL_FREE_VARS_USED, make_number (fv_used + 1));
  return fv_used;
}

struct scm *
template_ref_free_var (struct scm *template, size_t free_id)
{
  /* TODO: Code copy-pasted from the literal's case, but it's ok for now */
  struct scm *fv = struct_ref_ (template, TEMPL_FREE_VARS);
  return vector_ref_ (fv, free_id);
}
size_t
template_emit_capture (struct scm *tpl, struct scm *internal)
{
  long fv_used = struct_ref_ (internal, TEMPL_FREE_VARS_USED)->value;
  struct scm *fv = struct_ref_ (internal, TEMPL_FREE_VARS);

  size_t i;
  for (i = 0; i < fv_used; i = i + 1)
    /* NOTE: This lookup can recursively produce closures in outer procedures */
    template_emit_lookup (tpl, vector_ref_ (fv, i));
  template_emit_inst (tpl, OP_CLOSURE);
  return template_emit_argno (tpl, fv_used);
}


size_t
template_emit_exp (struct scm *tpl, struct scm *exp, int tail)   /*:((internal)) */
{
  switch (exp->type)
    {
    case TSPECIAL:
      /* self-evaluating, but they have dedicated opcodes */
      return template_emit_special (tpl, exp);
    case TCHAR:
    case TKEYWORD:
    case TPORT:
    case TSTRING:
    case TVECTOR:
    case TNUMBER:
      template_emit_inst (tpl, OP_LITERAL);
      return template_emit_literal (tpl, template_register_literal (tpl, exp));
    case TSYMBOL:
      template_emit_lookup (tpl, exp);
      return template_emit_inst (tpl, OP_RESOLVE);
    case TBINDING:
      template_emit_inst (tpl, OP_LITERAL);
      template_emit_literal (tpl, template_register_literal (tpl, exp));
      return template_emit_inst (tpl, OP_RESOLVE);

    case TPAIR:
      return template_emit_form (tpl, exp, tail);
    default:
      template_emit_inst (tpl, OP_LITERAL);
      return template_emit_literal (tpl, template_register_literal (tpl, exp));
      break;
    }
}

size_t
template_emit_form (struct scm *tpl, struct scm *exp, int tail)   /*:((internal)) */
{
  struct scm *keyword = template_lookup_keyword (tpl, exp);
  /* (core:define x ...) */
  if (keyword == cell_symbol_core_define)
    {
      struct scm *sym = exp->cdr->car;
      struct scm *val;

      if (sym->type == TPAIR)
        {
          val = exp->cdr->cdr;
          val = cons (sym->cdr, val);
          val = cons (cell_symbol_core_lambda, val);
          sym = sym->car;
        }
      else
        val = exp->cdr->cdr->car;

      assert_msg (sym->type == TSYMBOL || sym == cell_unspecified,
                  "define: expected symbol.");
      /* NOTE: define-immutable-record-type expands to:
       *  (if #f (define *unspecified* ...) something-else)
       * We add that case just to support it.
       */

      /* (define x (lambda ...)) */
      if (template_lookup_keyword (tpl, val) == cell_symbol_core_lambda)
        {
          /* TODO: We repeat the code here just to give name to the function
           * Make this better.
           */
          /* NOTE: Self-recursion is resolved through the closure. Before, we
           * required special code for that case.
           */
          struct scm *proc, *lambda;
          lambda = cons (cell_symbol_core_lambda, val->cdr);
          proc = compile_procedure_internal (lambda, sym, tpl, template_module (tpl), 0);
          template_emit_inst (tpl, OP_LITERAL);
          template_emit_literal (tpl, template_register_literal (tpl, proc));
          template_emit_capture (tpl, proc);
        }
      else
        template_emit_exp (tpl, val, 0);
      template_emit_definition_lookup (tpl, sym);
      return template_emit_inst (tpl, OP_SET);
    }

  /* (core:set! x y) */
  if (keyword == cell_symbol_core_set_x)
    {
      struct scm *target = exp->cdr->car;
      struct scm *val = exp->cdr->cdr->car;
      template_emit_exp (tpl, val, 0);
      if (target->type == TBINDING)
        {
          template_emit_inst (tpl, OP_LITERAL);
          template_emit_literal (tpl, template_register_literal (tpl, target));
        }
      else
        template_emit_lookup (tpl, target);
      return template_emit_inst (tpl, OP_SET);
    }
  /* (core:quote ...) */
  if (keyword == cell_symbol_core_quote)
    {
      struct scm *val = exp->cdr->car;
      template_emit_inst (tpl, OP_LITERAL);
      return template_emit_literal (tpl, template_register_literal (tpl, val));
    }
  /* (core:if cond true false) */
  if (keyword == cell_symbol_core_if)
    {
      size_t end, branch_target, jump_target, after_jump_target;
      struct scm *cond  = exp->cdr->car;
      struct scm *true  = exp->cdr->cdr->car;
      struct scm *false = exp->cdr->cdr->cdr->car;

      template_emit_exp (tpl, cond, 0);
      branch_target = template_emit_inst (tpl, OP_BRANCH); /* Branch if false */
      template_emit_target (tpl, 0xCAFE);

      end = template_emit_exp (tpl, true, tail);

      jump_target = template_emit_inst (tpl, OP_JUMP);
      after_jump_target = template_emit_target (tpl, 0xCAFE);

      template_patch_target (tpl, branch_target, after_jump_target);

      end = template_emit_exp (tpl, false, tail);
      template_patch_target (tpl, jump_target, end);
      return end;
    }
  /* (core:lambda ...) */
  if (keyword == cell_symbol_core_lambda)
    {
      struct scm *proc, *lambda;
      lambda = cons (cell_symbol_core_lambda, exp->cdr);
      proc = compile_procedure_internal (lambda, create_id ("anonymous"),
                                         tpl, template_module (tpl), 0);
      template_emit_inst (tpl, OP_LITERAL);
      template_emit_literal (tpl, template_register_literal (tpl, proc));
      return template_emit_capture (tpl, proc);
    }

  /* (core:begin ...) */
  if (keyword == cell_symbol_core_begin)
    return template_emit_sequence (tpl, exp->cdr, tail);

  /* (core:gc) */
  if (exp->car == cell_symbol_core_gc)
    return template_emit_inst (tpl, OP_GC);

  /* (core:call/cc ...) */
  if (keyword == cell_symbol_core_call_cc)
    {
      size_t end;
      size_t target = template_emit_inst (tpl, OP_CC);
      template_emit_target (tpl, 0xCAFE);
      template_emit_exp (tpl, exp->cdr->car, 0);
      if (tail)
        template_emit_inst (tpl, OP_TAIL_CALL);
      else
        template_emit_inst (tpl, OP_CALL);
      end = template_emit_argno (tpl, 1);
      template_patch_target (tpl, target, end);
      return end;
    }

  /* (core:apply ...) */
  if (keyword == cell_symbol_core_apply)
    {
      struct scm *f = exp->cdr->car;
      struct scm *args = exp->cdr->cdr->car;
      template_emit_exp (tpl, args, 0);
      template_emit_exp (tpl, f, 0);
      if (tail)
        return template_emit_inst (tpl, OP_TAIL_APPLY);
      else
        return template_emit_inst (tpl, OP_APPLY);
    }

  /* (core:set-mark! mark) */
  if (exp->car == cell_symbol_core_set_mark_x)
    {
      template_emit_exp (tpl, exp->cdr->car, 0);
      return template_emit_inst (tpl, OP_SET_MARK);
    }

  /* (core:ref-mark) */
  if (exp->car == cell_symbol_core_ref_mark)
    {
      return template_emit_inst (tpl, OP_REF_MARK);
    }

  /* (<anything_else> ...) */
  {
    int argno = 0;
    struct scm *call = exp->car;
    struct scm *cur = cell_nil;
    for (cur = exp->cdr; cur != cell_nil; cur = cur->cdr)
      {
        template_emit_exp (tpl, cur->car, 0);
        argno += 1;
      }
    template_emit_exp (tpl, call, 0); /* puts it in the stack */
    if (tail)
      template_emit_inst (tpl, OP_TAIL_CALL);
    else
      template_emit_inst (tpl, OP_CALL);
    return template_emit_argno (tpl, argno);
  }
}

/* High-level interface */
struct scm *
get_defined_names (struct scm *body)
{
  struct scm *cur;
  struct scm *names = cell_nil;
  for (cur = body; cur != cell_nil; cur = cur->cdr)
    {
      if (cur->car->type != TPAIR)
        break;
      if (cur->car->car != cell_symbol_core_define)
        break;
      struct scm *name = cur->car->cdr->car;
      if (name->type == TPAIR)
        name = name->car;
      names = cons (name, names);
    }
  return names;
}

struct scm *
compile_procedure_internal (struct scm *exp,
                            struct scm *name,
                            struct scm *parent,
                            struct scm *mod,
                            int top_level)
{
  /* Expects:
   * (lambda ... ...)
   */
  struct scm *tpl = make_template_ (name, parent, mod, exp);

  assert_msg (exp->car == cell_symbol_core_lambda, "compile_procedure: expected `core:lambda`.");
  assert_msg (exp->cdr->type == TPAIR, "compile_procedure: malformed procedure.");
  assert_msg (exp->cdr->cdr->type == TPAIR, "compile_procedure: malformed procedure.");

  struct scm *args = exp->cdr->car;
  struct scm *body = exp->cdr->cdr;

  assert_msg (args == cell_nil || args->type == TPAIR || args->type == TSYMBOL,
              "compile_procedure: formals: expected symbol or list.");

  struct scm *defines = get_defined_names (body);
  if (top_level == 1)
    {
      /* TODO: Try to delay this until we emit the define itself, so we don't
       * shadow before than we do. This makes (define define ...) possible.
       */
      struct scm *cur;
      for (cur = defines; cur != cell_nil; cur = cur->cdr)
        module_ensure_local_variable_x (mod, cur->car);
      defines = cell_nil;
    }

  template_set_arity (tpl, args);
  template_set_locals (tpl, args, defines);

  template_emit_sequence (tpl, body, 1);
  template_emit_inst (tpl, OP_RETURN);

  template_clear (tpl); /* Remove a few fields to help with the GC */
  return tpl;
}


struct scm *
compile_procedure (struct scm *exp, struct scm *name, struct scm *mod)
{
  return compile_procedure_internal (exp, name, cell_nil, mod, 0);
}

struct scm *
compile_top_level_exp (struct scm *exp, struct scm *mod)
{
  /* Takes:
   *   <exp>
   * Expands to:
   *   (lambda () <exp>)
   * So it can be compiled to a _template_ and returned.
   * TODO: Leave that to the macro expander?
   */
  exp = cons (cell_symbol_core_lambda, cons (cell_nil, cons (exp, cell_nil)));
  exp = compile_procedure_internal (exp, create_id ("top-level"), cell_nil, mod, 1);
  return make_closure (make_vector_ (0, cell_nil), exp);
}

/* Disassembler */
void
print_disassembly_line (int pc, unsigned char raw_byte, char *txt, char *suffix)
{
  char *s = ntoab (pc, 10, 0);
  size_t len = strlen (s);
  int i;
  for (i = 0; i < 8 - len; i++)
    eputs (" ");
  eputs (s);
  eputs (":\t");

  s = ntoab (raw_byte, 16, 0);
  len = strlen (s);
  eputs ("0x");
  for (i = 0; i < 2 - len; i++)
    eputs ("0");
  eputs (s);

  len = strlen (txt);
  for (i = 0; i < 20 - len; i++)
    eputs (" ");
  eputs (txt);
  eputs (suffix);
}

char *
inst_to_string (enum opcode inst)
{
  switch (inst)
    {
    case OP_UNSPECIFIED:
      return "OP_UNSPECIFIED";
    case OP_NIL:
      return "OP_NIL";
    case OP_FALSE:
      return "OP_FALSE";
    case OP_TRUE:
      return "OP_TRUE";
    case OP_LITERAL:
      return "OP_LITERAL";
    case OP_DROP:
      return "OP_DROP";
    case OP_BRANCH:
      return "OP_BRANCH";
    case OP_JUMP:
      return "OP_JUMP";
    case OP_SET:
      return "OP_SET";
    case OP_REF_LOCAL:
      return "OP_REF_LOCAL";
    case OP_REF_CLOSURE:
      return "OP_REF_CLOSURE";
    case OP_RESOLVE:
      return "OP_RESOLVE";
    case OP_CLOSURE:
      return "OP_CLOSURE";
    case OP_CC:
      return "OP_CC";
    case OP_GC:
      return "OP_GC";
    case OP_CALL:
      return "OP_CALL";
    case OP_TAIL_CALL:
      return "OP_TAIL_CALL";
    case OP_RETURN:
      return "OP_RETURN";
    case OP_UNREACHABLE:
      return "OP_UNREACHABLE";
    case OP_APPLY:
      return "OP_APPLY";
    case OP_TAIL_APPLY:
      return "OP_TAIL_APPLY";
    case OP_REF_MARK:
      return "OP_REF_MARK";
    case OP_SET_MARK:
      return "OP_SET_MARK";
    default:
      return "??";
    }
}

void
eprint_ptr (void *p)
{
  eputs ("0x");
  eputs (ntoab ((unsigned long) p, 16, 0));
}

struct scm *
argno_number (struct scm *templ, size_t id)
{
  return make_number (id);
}

void
template_disassemble (struct scm *templ)
{
  char buf[24];
  unsigned int pc = 0;
  unsigned long id;
  unsigned int pc_old;

  struct scm *(*accessor) (struct scm *, size_t);

  unsigned char *code = cell_bytes (template_code (templ));
  size_t used = struct_ref_ (templ, TEMPL_CODE_USED)->value;

  struct scm *literals = template_literals (templ);

  eputs ("  ────────────────────────── "); eprint_ptr (templ); eputs ("\n");
  eputs ("  - name: "); display_error_ (template_name (templ)); eputs ("\n");
  eputs ("  - arity: "); display_error_ (template_arity (templ)); eputs ("\n");
  eputs ("  - source: "); display_error_ (template_source (templ)); eputs ("\n");


  while (pc < used)
    {
      unsigned char inst = code[pc++];
      print_disassembly_line (pc - 1, inst, inst_to_string (inst), "\n");
      switch (inst)
        {
        case OP_JUMP:
        case OP_BRANCH:
        case OP_CC:
          {
            pc_old = pc;
            pc += pc & 1; /* Align to half size */
            unsigned short *t = (unsigned short *)(code + pc);
            unsigned short target = *t;
            pc += 2;
            strcpy (buf, ltoa ((unsigned long) target));

            char byte = code[pc_old++];
            print_disassembly_line (pc_old - 1, byte, buf, "\n");

            byte = code[pc_old++];
            print_disassembly_line (pc_old - 1, byte, ".", "\n");
            break;
          }

        case OP_LITERAL:
            accessor = &template_ref_literal;
            goto accessor_set;
        case OP_REF_LOCAL:
            accessor = &template_ref_local;
            goto accessor_set;
        case OP_REF_CLOSURE:
            accessor = &template_ref_free_var;
        case OP_CALL:
        case OP_TAIL_CALL:
        case OP_CLOSURE:
          {
            accessor = &argno_number;
accessor_set:
            pc_old = pc;

            pc += pc & 1; /* Align to half size */
            unsigned short *t = (unsigned short *)(code + pc);
            id = *t;
            pc += 2;
            strcpy (buf, ltoa ((unsigned long) id));

            unsigned char byte = code[pc_old++];

            print_disassembly_line (pc_old - 1, byte, buf, " ");
            struct scm *val = accessor (templ, id);
            /* TODO: Temporary until our templates are C structs */
            if (template_p (val) == cell_t)
              {
                eputs ("template: ");
                display_error_ (template_name (val));
                eputs (" (");
                eprint_ptr (val);
                eputs (")");
              }
            else
              display_error_ (val);
            eputs ("\n");

            while (pc_old < pc)
              {
                byte = code[pc_old++];
                print_disassembly_line (pc_old - 1, byte, ".", "\n");
              }
            break;
          }
        }
    }
  eputs ("\n");
}

/* Builtins */
struct scm *
procedure_disassemble (struct scm *closure)
{
  if (closure->type != TCLOSURE)
    error (cell_symbol_wrong_type_arg,
           cons (closure, cstring_to_symbol ("procedure-disassemble")));
  template_disassemble (closure->template);
  return cell_unspecified;
}
struct scm *
procedure_name (struct scm *closure)
{
  if (closure->type != TCLOSURE)
    error (cell_symbol_wrong_type_arg,
           cons (closure, cstring_to_symbol ("procedure-name")));
  return template_name (closure->template);
}
struct scm *
procedure_arguments (struct scm *closure)
{
  if (closure->type != TCLOSURE)
    error (cell_symbol_wrong_type_arg,
           cons (closure, cstring_to_symbol ("procedure-arguments")));
  return template_arguments (closure->template);
}
/* TODO: Maybe add a procedure-captures to be able to inspect the closure? */
