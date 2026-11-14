/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2016,2017,2018,2019,2022 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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

#ifndef __MES_CONSTANTS_H
#define __MES_CONSTANTS_H

/* Cell types */

enum cell_type
{
  TCHAR,
  TBYTES,
  TCLOSURE,
  TCONTINUATION,
  TBUILTIN,
  TKEYWORD,
  TNUMBER,
  TPAIR,
  TPORT,
  TSPECIAL,
  TSTRING,
  TSTRUCT,
  TSYMBOL,
  TVALUES,
  TBINDING,
  TVECTOR,
  TVARIABLE,
  TFRAME,
  TBROKEN_HEART,
};

/* VM opcodes */
enum opcode
{
  /* These 4 could be just an OP_LITERAL */
  OP_UNSPECIFIED,
  OP_NIL,
  OP_FALSE,
  OP_TRUE,

  OP_LITERAL,
  OP_DROP,
  OP_BRANCH,
  OP_JUMP,
  OP_SET,
  OP_REF_LOCAL,
  OP_REF_CLOSURE,
  OP_RESOLVE,
  OP_CLOSURE,
  OP_GC,
  OP_CC,
  OP_CALL,
  OP_TAIL_CALL,
  OP_RETURN,
  OP_APPLY,
  OP_TAIL_APPLY,
  OP_REF_MARK,
  OP_SET_MARK,
  OP_UNREACHABLE = 0xAA, /* Value is easy to read */
};

/* Sizes */

#define BUILTINS_SIZE 255

/* Struct types */
#define STRUCT_TYPE 0
#define STRUCT_PRINTER 1

#endif /* __MES_CONSTANTS_H */
