/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2016,2017,2018,2019,2021,2022 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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

#ifndef __MES_SYMBOLS_H
#define __MES_SYMBOLS_H

extern struct scm *cell_nil;
extern struct scm *cell_f;
extern struct scm *cell_t;
extern struct scm *cell_dot;
extern struct scm *cell_arrow;
extern struct scm *cell_undefined;
extern struct scm *cell_unspecified;
extern struct scm *cell_closure;
extern struct scm *cell_circular;

extern struct scm *cell_symbol_lambda;
extern struct scm *cell_symbol_begin;
extern struct scm *cell_symbol_if;
extern struct scm *cell_symbol_quote;
extern struct scm *cell_symbol_define;
extern struct scm *cell_symbol_quasiquote;
extern struct scm *cell_symbol_unquote;
extern struct scm *cell_symbol_unquote_splicing;
extern struct scm *cell_symbol_syntax;
extern struct scm *cell_symbol_quasisyntax;
extern struct scm *cell_symbol_unsyntax;
extern struct scm *cell_symbol_unsyntax_splicing;
extern struct scm *cell_symbol_set_x;
extern struct scm *cell_symbol_car;
extern struct scm *cell_symbol_cdr;
extern struct scm *cell_symbol_not_a_number;
extern struct scm *cell_symbol_not_a_pair;
extern struct scm *cell_symbol_system_error;
extern struct scm *cell_symbol_throw;
extern struct scm *cell_symbol_unbound_variable;
extern struct scm *cell_symbol_wrong_number_of_args;
extern struct scm *cell_symbol_wrong_type_arg;
extern struct scm *cell_symbol_out_of_range;
extern struct scm *cell_symbol_buckets;
extern struct scm *cell_symbol_builtin;
extern struct scm *cell_symbol_frame;
extern struct scm *cell_symbol_hashq_table;
extern struct scm *cell_symbol_procedure;
extern struct scm *cell_symbol_record_type;
extern struct scm *cell_symbol_size;
extern struct scm *cell_symbol_stack;
extern struct scm *cell_symbol_argv;
extern struct scm *cell_symbol_mes_datadir;
extern struct scm *cell_symbol_mes_version;
extern struct scm *cell_symbol_internal_time_units_per_second;
extern struct scm *cell_symbol_compiler;
extern struct scm *cell_symbol_arch;
extern struct scm *cell_symbol_variable;
extern struct scm *cell_type_bytes;
extern struct scm *cell_type_char;
extern struct scm *cell_type_closure;
extern struct scm *cell_type_continuation;
extern struct scm *cell_type_function;
extern struct scm *cell_type_keyword;
extern struct scm *cell_type_number;
extern struct scm *cell_type_pair;
extern struct scm *cell_type_port;
extern struct scm *cell_type_ref;
extern struct scm *cell_type_special;
extern struct scm *cell_type_string;
extern struct scm *cell_type_struct;
extern struct scm *cell_type_symbol;
extern struct scm *cell_type_values;
extern struct scm *cell_type_binding;
extern struct scm *cell_type_vector;
extern struct scm *cell_type_broken_heart;
extern struct scm *cell_symbol_program;
extern struct scm *cell_symbol_test;

extern struct scm *cell_symbol_core_apply;
extern struct scm *cell_symbol_core_lambda;
extern struct scm *cell_symbol_core_begin;
extern struct scm *cell_symbol_core_if;
extern struct scm *cell_symbol_core_quote;
extern struct scm *cell_symbol_core_define;
extern struct scm *cell_symbol_core_set_x;
extern struct scm *cell_symbol_core_call_cc;
extern struct scm *cell_symbol_core_gc;
extern struct scm *cell_symbol_core_ref_mark;
extern struct scm *cell_symbol_core_set_mark_x;

#endif /* __MES_SYMBOLS_H */
