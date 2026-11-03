/* GNU Mes --- Maxwell Equations of Software
 * Copyright © 2026 Gavin John
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
#include <windows/ntcall.h>
#include <windows/ntdll.h>
#include <mes/lib.h>
#include <fcntl.h>

/* Stands in for lib/linux/_read.c: the bare read, with nothing above it.
 * configure-lib.sh's lists build read from lib/posix/buffered-read.c, whose
 * __buffered_read fills its buffer with this, so this must not call read.
 * NtReadFile is NtWriteFile's twin, and an NTSTATUS below zero is how the end
 * of the file arrives as much as how a failure does -- both mean nothing more
 * was read, which is what a caller of read wants to hear as 0. */

ssize_t
_read (int filedes, void *buffer, size_t size)
{
  int NtReadFile;
  int *iosb;
  int handle;
  int rc;

  NtReadFile = __ntdll_resolve ("NtReadFile");
  iosb = __iosb ();
  iosb[0] = 0;
  iosb[1] = 0;
  handle = __handle (filedes);

  rc = __ntcall9 (NtReadFile, handle, 0, 0, 0, iosb, buffer, size, 0, 0);
  if (rc < 0)
    return 0;
  return iosb[1];
}
