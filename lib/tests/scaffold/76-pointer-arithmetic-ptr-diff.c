/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2026 Ekaitz Zarraga <ekaitz@elenq.tech>
 * Copyright © 2026 Janneke Nieuwenhuizen <janneke@gnu.org>
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

#include <stdio.h>
struct scm
{
  int type;
  struct scm *car;
  struct scm *cdr;
};

int
main ()
{
  struct scm* cells = 0;
  struct scm* free = cells + 1;

  free  = cells + 1;
  printf ("size of scm: %d\n", sizeof (struct scm));
  if ((free - cells) != 1)
    return 1;

  long GC_SAFETY = 2;
  long used = (free - cells) + GC_SAFETY;
  printf ("used: %ld\n", used);
  if (used != 3)
    return 2;

  unsigned char foo = (free - cells) - GC_SAFETY;
  printf ("foo: %u\n", foo);
  if (foo != 255)
    return 3;

  return 0;
}
