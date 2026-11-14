/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2016,2017,2018,2019,2020,2022,2023 Janneke Nieuwenhuizen <janneke@gnu.org>
 * Copyright © 2022,2023 Timothy Sample <samplet@ngyro.com>
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

#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>

const char *
builtin_name_ (struct scm *builtin)
{
  if (builtin->type != TBUILTIN)
    error (cell_symbol_wrong_type_arg,
           cons (builtin, cstring_to_symbol ("builtin-name")));
  return BUILTINS [builtin->builtin].name;
}
struct scm *
builtin_name (struct scm *builtin)
{
  return make_string0 (builtin_name_ (builtin));
}

signed char
builtin_arity_ (struct scm *builtin)
{
  if (builtin->type != TBUILTIN)
    error (cell_symbol_wrong_type_arg,
           cons (builtin, cstring_to_symbol ("builtin-arity")));
  return BUILTINS [builtin->builtin].arity;
}
struct scm *
builtin_arity (struct scm *builtin)
{
  return make_number ((long)builtin_arity_ (builtin));
}

FUNCTION
builtin_function (struct scm *builtin)
{
  if (builtin->type != TBUILTIN)
    error (cell_symbol_wrong_type_arg,
           cons (builtin, cstring_to_symbol ("builtin-function")));
  return BUILTINS [builtin->builtin].function;
}

struct scm *
builtin_p (struct scm *x)
{
  if (x->type == TBUILTIN)
      return cell_t;
  return cell_f;
}
struct scm *
make_builtin (unsigned short id)
{
  return make_value_cell (TBUILTIN, 0, (long)id);
}

struct scm *
init_builtin (unsigned short *index, char const *name, int arity, void* function, struct scm *a)
{
  unsigned short i = *index;
  assert_msg (i < BUILTINS_SIZE, "index < BUILTINS_SIZE");
  BUILTINS [i].function = function;
  BUILTINS [i].name = name;
  BUILTINS [i].arity = arity;
  *index = i + 1;
  return acons (cstring_to_symbol (name), make_builtin (i), a);
}

struct scm *
mes_builtins (struct scm *a)            /*:((internal)) */
{
  unsigned short index = 0;
  if (g_mini != 0)
    {
      a = init_builtin (&index, "cons", 2, &cons, a);
      a = init_builtin (&index, "car", 1, &car, a);
      a = init_builtin (&index, "list", -1, &list, a);
      a = init_builtin (&index, "eq?", 2, &eq_p, a);
      a = init_builtin (&index, "-", -1, &minus, a);
      a = init_builtin (&index, "+", -1, &plus, a);
      a = init_builtin (&index, "core:display", 1, &display_, a);
      a = init_builtin (&index, "core:write", 1, &write_, a);
      a = init_builtin (&index, "core:display-error", 1, &display_error_, a);
      a = init_builtin (&index, "getenv", 1, &getenv_, a);
      a = init_builtin (&index, ">", -1, &greater_p, a);
      a = init_builtin (&index, "<", -1, &less_p, a);
      a = init_builtin (&index, "make-vector", -1, &make_vector, a);
      return a;
    }

  /* src/builtins.c */
  a = init_builtin (&index, "builtin-name", 1, &builtin_name, a);
  a = init_builtin (&index, "builtin-arity", 1, &builtin_arity, a);
  a = init_builtin (&index, "builtin?", 1, &builtin_p, a);
  /* src/core.c */
  a = init_builtin (&index, "car", 1, &car, a);
  a = init_builtin (&index, "cdr", 1, &cdr, a);
  a = init_builtin (&index, "list", -1, &list, a);
  a = init_builtin (&index, "null?", 1, &null_p, a);
  a = init_builtin (&index, "eq?", 2, &eq_p, a);
  a = init_builtin (&index, "%values", -1, &values, a);
  a = init_builtin (&index, "acons", 3, &acons, a);
  a = init_builtin (&index, "length", 1, &length, a);
  a = init_builtin (&index, "error", 2, &error, a);
  a = init_builtin (&index, "append2", 2, &append2, a);
  a = init_builtin (&index, "append-reverse", 2, &append_reverse, a);
  a = init_builtin (&index, "core:reverse!", 2, &reverse_x_, a);
  a = init_builtin (&index, "assq", 2, &assq, a);
  a = init_builtin (&index, "assoc", 2, &assoc, a);
  /* src/display.c */
  a = init_builtin (&index, "core:display", 1, &display_, a);
  a = init_builtin (&index, "core:display-error", 1, &display_error_, a);
  a = init_builtin (&index, "core:display-port", 2, &display_port_, a);
  a = init_builtin (&index, "core:write", 1, &write_, a);
  a = init_builtin (&index, "core:write-error", 1, &write_error_, a);
  a = init_builtin (&index, "core:write-port", 2, &write_port_, a);
  /* src/eval-apply.c */
  a = init_builtin (&index, "display-backtrace", 1, &display_backtrace, a);
  /* src/gc.c */
  a = init_builtin (&index, "make-binding", 2, &make_binding, a);
  a = init_builtin (&index, "gc-stats", 0, &gc_stats, a);
  a = init_builtin (&index, "cons", 2, &cons, a);
  /* src/hash.c */
  a = init_builtin (&index, "hashq", 2, &hashq, a);
  a = init_builtin (&index, "hash", 2, &hash, a);
  a = init_builtin (&index, "hashq-get-handle", 2, &hashq_get_handle, a);
  a = init_builtin (&index, "core:hashq-ref", 3, &hashq_ref_, a);
  a = init_builtin (&index, "core:hash-ref", 3, &hash_ref_, a);
  a = init_builtin (&index, "hashq-create-handle!", 3, &hashq_create_handle_x, a);
  a = init_builtin (&index, "hash-create-handle!", 3, &hash_create_handle_x, a);
  a = init_builtin (&index, "hashq-set!", 3, &hashq_set_x, a);
  a = init_builtin (&index, "hash-set!", 3, &hash_set_x, a);
  a = init_builtin (&index, "hash-remove!", 2, &hash_remove_x, a);
  a = init_builtin (&index, "hash-table-printer", 1, &hash_table_printer, a);
  a = init_builtin (&index, "hash-table?", 1, &hash_table_p, a);
  a = init_builtin (&index, "make-hash-table", -1, &make_hash_table, a);
  a = init_builtin (&index, "hash-buckets", 1, &hash_buckets, a);
  a = init_builtin (&index, "hash-clear!", 1, &hash_clear_x, a);
  /* src/lib.c */
  a = init_builtin (&index, "pairlis", 3, &pairlis, a);
  a = init_builtin (&index, "set-car!", 2, &set_car_x, a);
  a = init_builtin (&index, "set-cdr!", 2, &set_cdr_x, a);
  a = init_builtin (&index, "core:type", 1, &type_, a);
  a = init_builtin (&index, "core:car", 1, &car_, a);
  a = init_builtin (&index, "core:cdr", 1, &cdr_, a);
  a = init_builtin (&index, "xassq", 2, &xassq, a);
  a = init_builtin (&index, "memq", 2, &memq, a);
  a = init_builtin (&index, "equal2?", 2, &equal2_p, a);
  a = init_builtin (&index, "last-pair", 1, &last_pair, a);
  a = init_builtin (&index, "pair?", 1, &pair_p, a);
  a = init_builtin (&index, "char->integer", 1, &char_to_integer, a);
  a = init_builtin (&index, "integer->char", 1, &integer_to_char, a);
  a = init_builtin (&index, "make-bytevector", -1, &make_bytevector, a);
  a = init_builtin (&index, "bytevector-fill!", -1, &bytevector_fill_x, a);
  a = init_builtin (&index, "bytevector-u8-ref", 2, &bytevector_u8_ref, a);
  a = init_builtin (&index, "bytevector-u8-set!", 3, &bytevector_u8_set_x, a);
  /* src/math.c */
  a = init_builtin (&index, ">", -1, &greater_p, a);
  a = init_builtin (&index, "<", -1, &less_p, a);
  a = init_builtin (&index, "=", -1, &is_p, a);
  a = init_builtin (&index, "-", -1, &minus, a);
  a = init_builtin (&index, "+", -1, &plus, a);
  a = init_builtin (&index, "/", -1, &divide, a);
  a = init_builtin (&index, "modulo", 2, &modulo, a);
  a = init_builtin (&index, "*", -1, &multiply, a);
  a = init_builtin (&index, "logand", -1, &logand, a);
  a = init_builtin (&index, "logior", -1, &logior, a);
  a = init_builtin (&index, "lognot", 1, &lognot, a);
  a = init_builtin (&index, "logxor", -1, &logxor, a);
  a = init_builtin (&index, "ash", 2, &ash, a);
  /* src/module.c */
  a = init_builtin (&index, "initial-module", 0, &initial_module, a);
  a = init_builtin (&index, "current-module", 0, &current_module, a);
  a = init_builtin (&index, "set-current-module", 1, &set_current_module, a);
  a = init_builtin (&index, "module-variable", 2, &module_variable, a);
  a = init_builtin (&index, "module-local-variable", 2, &module_local_variable, a);
  a = init_builtin (&index, "module-ensure-local-variable!", 2, &module_ensure_local_variable_x, a);
  a = init_builtin (&index, "module-define!", 3, &module_define_x, a);
  /* src/posix.c */
  a = init_builtin (&index, "abort", 0, &abort_, a);
  a = init_builtin (&index, "exit", 1, &exit_, a);
  a = init_builtin (&index, "peek-byte", 0, &peek_byte, a);
  a = init_builtin (&index, "read-byte", 0, &read_byte, a);
  a = init_builtin (&index, "unread-byte", 1, &unread_byte, a);
  a = init_builtin (&index, "peek-char", 0, &peek_char, a);
  a = init_builtin (&index, "read-char", -1, &read_char, a);
  a = init_builtin (&index, "unread-char", 1, &unread_char, a);
  a = init_builtin (&index, "write-char", -1, &write_char, a);
  a = init_builtin (&index, "write-byte", -1, &write_byte, a);
  a = init_builtin (&index, "getenv", 1, &getenv_, a);
  a = init_builtin (&index, "setenv", 2, &setenv_, a);
  a = init_builtin (&index, "access?", 2, &access_p, a);
  a = init_builtin (&index, "current-input-port", 0, &current_input_port, a);
  a = init_builtin (&index, "open", 2, &open_, a);
  a = init_builtin (&index, "open-input-file", 1, &open_input_file, a);
  a = init_builtin (&index, "open-input-string", 1, &open_input_string, a);
  a = init_builtin (&index, "set-current-input-port", 1, &set_current_input_port, a);
  a = init_builtin (&index, "current-output-port", 0, &current_output_port, a);
  a = init_builtin (&index, "current-error-port", 0, &current_error_port, a);
  a = init_builtin (&index, "open-output-file", -1, &open_output_file, a);
  a = init_builtin (&index, "set-current-output-port", 1, &set_current_output_port, a);
  a = init_builtin (&index, "set-current-error-port", 1, &set_current_error_port, a);
  a = init_builtin (&index, "chmod", 2, &chmod_, a);
  a = init_builtin (&index, "isatty?", 1, &isatty_p, a);
  a = init_builtin (&index, "primitive-fork", 0, &primitive_fork, a);
  a = init_builtin (&index, "core:spawn", 2, &spawn_, a);
  a = init_builtin (&index, "primitive-exit", 1, &primitive_exit, a);
  a = init_builtin (&index, "core:execl", 2, &execl_, a);
  a = init_builtin (&index, "core:execle", 3, &execle_, a);

  a = init_builtin (&index, "core:waitpid", 2, &waitpid_, a);

  a = init_builtin (&index, "current-time", 0, &current_time, a);
  a = init_builtin (&index, "gettimeofday", 0, &gettimeofday_, a);
  a = init_builtin (&index, "get-internal-run-time", 0, &get_internal_run_time, a);
  a = init_builtin (&index, "getcwd", 0, &getcwd_, a);
  a = init_builtin (&index, "dup", 1, &dup_, a);
  a = init_builtin (&index, "dup2", 2, &dup2_, a);
  a = init_builtin (&index, "delete-file", 1, &delete_file, a);
  a = init_builtin (&index, "uname", 0, &uname_, a);

  a = acons (cstring_to_symbol ("O_RDONLY"), make_number (O_RDONLY), a);
  a = acons (cstring_to_symbol ("O_WRONLY"), make_number (O_WRONLY), a);
  a = acons (cstring_to_symbol ("O_CREAT"), make_number (O_CREAT), a);
  a = acons (cstring_to_symbol ("O_TRUNC"), make_number (O_TRUNC), a);

  /* src/reader.c */
  a = init_builtin (&index, "core:read-input-file", 1, &read_input_file_, a);
  a = init_builtin (&index, "read-input-file", 0, &read_input_file, a);
  a = init_builtin (&index, "reader-read", 0, &reader_read, a);
  a = init_builtin (&index, "reader-read-sexp", 3, &reader_read_sexp, a);
  a = init_builtin (&index, "reader-read-character", 0, &reader_read_character, a);
  a = init_builtin (&index, "reader-read-binary", 0, &reader_read_binary, a);
  a = init_builtin (&index, "reader-read-octal", 0, &reader_read_octal, a);
  a = init_builtin (&index, "reader-read-hex", 0, &reader_read_hex, a);
  a = init_builtin (&index, "reader-read-string", 0, &reader_read_string, a);
  /* src/string.c */
  a = init_builtin (&index, "string=?", 2, &string_equal_p, a);
  a = init_builtin (&index, "symbol->string", 1, &symbol_to_string, a);
  a = init_builtin (&index, "symbol->keyword", 1, &symbol_to_keyword, a);
  a = init_builtin (&index, "keyword->string", 1, &keyword_to_string, a);
  a = init_builtin (&index, "string->symbol", 1, &string_to_symbol, a);
  a = init_builtin (&index, "make-symbol", 1, &make_symbol, a);
  a = init_builtin (&index, "string->list", 1, &string_to_list, a);
  a = init_builtin (&index, "list->string", 1, &list_to_string, a);
  a = init_builtin (&index, "read-string", -1, &read_string, a);
  a = init_builtin (&index, "string-append", -1, &string_append, a);
  a = init_builtin (&index, "string-length", 1, &string_length, a);
  a = init_builtin (&index, "string-ref", 2, &string_ref, a);
  a = init_builtin (&index, "string-set!", 3, &string_set_x, a);
  a = init_builtin (&index, "string-copy!", -1, &string_copy_x, a);
  a = init_builtin (&index, "make-string", -1, &make_string_init, a);
  /* src/struct.c */
  a = init_builtin (&index, "make-struct", 3, &make_struct, a);
  a = init_builtin (&index, "struct-length", 1, &struct_length, a);
  a = init_builtin (&index, "struct-ref", 2, &struct_ref, a);
  a = init_builtin (&index, "struct-set!", 3, &struct_set_x, a);
  /* src/variable.c */
  a = init_builtin (&index, "make-variable", 1, &make_variable, a);
  a = init_builtin (&index, "variable?", 1, &variable_p, a);
  a = init_builtin (&index, "variable-ref", 1, &variable_ref, a);
  a = init_builtin (&index, "variable-set!", 2, &variable_set_x, a);
  /* src/vector.c */
  a = init_builtin (&index, "make-vector", -1, &make_vector, a);
  a = init_builtin (&index, "vector-length", 1, &vector_length, a);
  a = init_builtin (&index, "vector-ref", 2, &vector_ref, a);
  a = init_builtin (&index, "vector-set!", 3, &vector_set_x, a);
  a = init_builtin (&index, "list->vector", 1, &list_to_vector, a);
  a = init_builtin (&index, "vector->list", 1, &vector_to_list, a);
  a = init_builtin (&index, "vector-copy!", -1, &vector_copy_x, a);
  a = init_builtin (&index, "vector-fill!", -1, &vector_fill_x, a);
  /* src/compiler.c */
  a = init_builtin (&index, "compile-top-level", 2, &compile_top_level_exp, a);
  a = init_builtin (&index, "procedure-disassemble", 1, &procedure_disassemble, a);
  a = init_builtin (&index, "procedure-arguments", 1, &procedure_arguments, a);
  a = init_builtin (&index, "procedure-name", 1, &procedure_name, a);

  /* This is a conditional compilation hack for M2-Planet in bootstrap
     mode.  The following file will not be included in the M2-Planet
     build of Mes, as M2-Planet will ignore the preprocessor directive.
     Other builds of Mes will include it.  The code in the file relies
     on system interfaces supported by MesCC but not M2-Planet. */
#include "mescc-builtins.c"

  return a;
}
