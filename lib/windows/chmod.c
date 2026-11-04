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
#include <string.h>
#include <sys/stat.h>

/* Stands in for lib/linux/chmod.c.
 *
 * Windows has no execute bit: an image runs because its PE header says it is
 * one.  A Nix archive records one for each regular file all the same, from
 * the owner execute bit, and nova-nix, which builds Nix derivations on
 * Windows, keeps it as an NTFS alternate data stream named nova.exec beside
 * the file, whose presence is the bit.  chmod sets that stream from the owner
 * execute bit and removes it when the bit is clear, as M2libc's chmod does,
 * and _open3 does the same for a file it makes.  The rest of the mode is
 * ignored.  A directory is left as it is, since a Nix archive records no mode
 * for one.
 */

/* The NtCreateFile arguments that make the stream. */
#define GENERIC_WRITE                0x40000000
#define SYNCHRONIZE                  0x00100000
#define FILE_ATTRIBUTE_NORMAL        0x80
#define FILE_SHARE_READ              1
#define FILE_OVERWRITE_IF            5
#define FILE_NON_DIRECTORY_FILE      0x40
#define FILE_SYNCHRONOUS_IO_NONALERT 0x20

/* FileAttributes in a FILE_BASIC_INFORMATION. */
#define FILE_ATTRIBUTE_DIRECTORY     0x10

/* What NtDeleteFile answers for a stream that is not there. */
#define STATUS_OBJECT_NAME_NOT_FOUND ((int) 0xc0000034)

/* The OBJECT_ATTRIBUTES naming the nova.exec stream of FILE, an
 * OBJECT_ATTRIBUTES from __ntobject: the same NT path with ":nova.exec"
 * after it.  The name goes on the NT path rather than on the path chmod was
 * given because Win32 reads "a:nova.exec" as the file nova.exec on drive A. */
int *
__exec_stream (int *file)
{
  char const *suffix;
  int *path;
  char *from;
  char *to;
  int length;
  int n;
  int size;
  int i;
  int *name;
  int *oa;

  suffix = ":nova.exec";
  n = strlen (suffix);

  /* ObjectName is a UNICODE_STRING: Length and MaximumLength as 16-bit
     halves of its first word, then the text.  Length counts bytes and no
     terminator. */
  path = file[2];
  length = path[0] & 0xffff;
  from = path[1];
  size = length + 2 * n;

  to = malloc (size + 2);
  memset (to, 0, size + 2);
  i = 0;
  while (i < length)
    {
      to[i] = from[i];
      i = i + 1;
    }
  i = 0;
  while (i < n)
    {
      to[length + 2 * i] = suffix[i];
      i = i + 1;
    }

  name = malloc (8);
  name[0] = size + 65536 * (size + 2);
  name[1] = to;

  oa = malloc (24);
  memset (oa, 0, 24);
  oa[0] = 24;                   /* Length: the size of this struct */
  oa[2] = name;                 /* ObjectName */
  oa[3] = file[3];              /* Attributes */
  return oa;
}

/* Make the nova.exec stream of FILE, an OBJECT_ATTRIBUTES from __ntobject
 * for a regular file that is there, holding "1" as nova-nix writes it,
 * though only its presence is read.  FILE has to be there because opening a
 * stream with a disposition that creates makes the file it belongs to as
 * well.  NTFS refuses to write or delete any stream of a file with the read
 * only attribute, so this and chmod's removal fail on such a file. */
int
__exec_mark (int *file)
{
  int NtCreateFile;
  int NtWriteFile;
  int NtClose;
  int *stream;
  int *iosb;
  int *handle;
  int rc;

  stream = __exec_stream (file);
  iosb = malloc (8);
  iosb[0] = 0;
  iosb[1] = 0;
  handle = malloc (4);
  handle[0] = 0;

  NtCreateFile = __ntdll_resolve ("NtCreateFile");
  rc = __ntcall11 (NtCreateFile, handle, GENERIC_WRITE | SYNCHRONIZE, stream,
                   iosb, 0, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ,
                   FILE_OVERWRITE_IF,
                   FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                   0, 0);
  if (rc != 0)
    return -1;

  NtWriteFile = __ntdll_resolve ("NtWriteFile");
  rc = __ntcall9 (NtWriteFile, handle[0], 0, 0, 0, iosb, "1", 1, 0, 0);
  NtClose = __ntdll_resolve ("NtClose");
  __ntcall1 (NtClose, handle[0]);
  if (rc != 0)
    return -1;
  return 0;
}

/* Mark FILE, a file _open3 has just created, if MODE has the owner execute
 * bit.  A file created without it is left untouched.  This is here rather
 * than in _open3.c because kaem.windows.run has M2-Planet compile _open3.c
 * before <sys/stat.h> defines S_IXUSR. */
int
__exec_create (int *file, int mode)
{
  if ((mode & S_IXUSR) == 0)
    return 0;
  return __exec_mark (file);
}

int
chmod (char const *file_name, mode_t mask)
{
  int NtQueryAttributesFile;
  int NtDeleteFile;
  int *file;
  int *basic;
  int *stream;
  int rc;

  file = __ntobject (file_name);
  if (file == 0)
    return -1;

  /* FILE_BASIC_INFORMATION: four timestamps and then FileAttributes */
  basic = malloc (40);
  memset (basic, 0, 40);
  NtQueryAttributesFile = __ntdll_resolve ("NtQueryAttributesFile");
  if (__ntcall2 (NtQueryAttributesFile, file, basic) != 0)
    return -1;
  if ((basic[8] & FILE_ATTRIBUTE_DIRECTORY) != 0)
    return 0;

  if ((mask & S_IXUSR) != 0)
    return __exec_mark (file);

  stream = __exec_stream (file);
  NtDeleteFile = __ntdll_resolve ("NtDeleteFile");
  rc = __ntcall1 (NtDeleteFile, stream);
  if (rc == STATUS_OBJECT_NAME_NOT_FOUND)
    return 0;
  if (rc != 0)
    return -1;
  return 0;
}
