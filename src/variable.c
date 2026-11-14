/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © Timothy Sample <samplet@ngyro.com>
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

/* TODO:
 * Remove this file and make it all become just a base type.
 */

struct scm *
make_variable (struct scm *value)
{
  return make_cell (TVARIABLE, cell_nil, value);
}

struct scm *
variable_p (struct scm *x)
{
  if (x->type == TVARIABLE)
    return cell_t;
  return cell_f;
}

struct scm *
variable_ref (struct scm *var)
{
  return var->variable;
}

struct scm *
variable_set_x (struct scm *var, struct scm *val)
{
  var->variable = val;
  return cell_unspecified;
}
