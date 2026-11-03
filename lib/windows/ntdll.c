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

/* What every other file in lib/windows/ stands on: how to reach an ntdll
 * routine, and how to hand it a filename.
 *
 * Windows has no syscall a program may make directly, so there is no
 * _sys_call here for lib/windows/ to be thin wrappers over the way
 * lib/linux/ is over int $0x80.  Instead, __ntdll_resolve (name) walks this
 * process's own PEB -> Ldr -> module list and ntdll's export table,
 * in-process, and hands back the routine's address.  It is ported from
 * stage0-pe32's M2libc fork (M2libc/x86/windows/ntdll.c), which measured it
 * byte-identical to lib/m2/x86/ntdll-i386.hex2's table for every routine
 * this port calls.
 *
 * That table is filled before main runs, and __ntdll (slot) in
 * lib/windows/<arch>-mes-<compiler>/ntlow.c hands back one of its entries
 * by index; include/windows/ntdll.h's NT_* constants name them.  Only
 * scaffold/ntdll-resolve-test.c calls __ntdll, to check __ntdll_resolve
 * against it.
 *
 * Every call goes through __ntcallN (see <windows/ntcall.h>), never to
 * ntdll directly: ntdll is stdcall, and what that costs depends on which
 * compiler is building, so it is settled in one place per compiler instead
 * of at every call site.  A caller writes the arguments in the order the
 * routine documents and nothing else.
 *
 * Filenames: ntdll takes UTF-16, inside an OBJECT_ATTRIBUTES around a
 * UNICODE_STRING, and for a file on disk it wants an NT path (\??\C:\...)
 * rather than the DOS path a program was given.
 * RtlDosPathNameToNtPathName_U does that conversion and __ntobject does the
 * rest.  Widening is a zero byte after each byte, which is right for ASCII
 * and wrong for everything else; a path with a character above 127 will not
 * survive, and this is not the layer that could say so.
 *
 * Nothing here frees anything.  Mes's malloc never gives memory back either.
 */

#include <windows/ntcall.h>
#include <windows/ntdll.h>
#include <mes/lib.h>

/* __ntdll_rd/__ntdll_rw/__ntdll_rb: read a dword/word/byte at an address
 * held as a plain int rather than a typed pointer, the same convention
 * brk.c's __alloc_addr/__alloc_size already use for addresses handed to
 * ntdll. */
int
__ntdll_rd (int addr)
{
  int *p;

  p = addr;
  return p[0];
}

int
__ntdll_rw (int addr)
{
  char *p;

  p = addr;
  return (255 & p[0]) + (256 * (255 & p[1]));
}

int
__ntdll_rb (int addr)
{
  char *p;

  p = addr;
  return 255 & p[0];
}

/* Walk this process's own PEB -> Ldr -> InMemoryOrderModuleList for a module
 * named (ASCII) `want', matched against its UTF-16 BaseDllName one code unit
 * at a time.  PEB->Ldr is at +0x0C and Ldr->InMemoryOrderModuleList's head
 * is at +0x14.  Walking the list keeps a pointer to each entry's
 * InMemoryOrderLinks field (LDR_DATA_TABLE_ENTRY +0x08) rather than the
 * entry's own base, which is why DllBase (real offset +0x18) reads back at
 * cur+0x10 and BaseDllName.Length/Buffer (real offsets +0x2C/+0x30) at
 * cur+0x24/+0x28 below.  Returns the module base, or 0 if the (circular)
 * list runs out first. */
int
__ntdll_find_module (char const *want)
{
  int peb;
  int ldr;
  int head;
  int cur;
  int base;
  int name_len;
  int name;
  int i;
  int match;
  int guard;

  peb = __peb ();
  ldr = __ntdll_rd (peb + 0x0C);
  head = ldr + 0x14;
  cur = __ntdll_rd (head);

  guard = 0;
  while (guard < 512)
    {
      if (cur == head)
        break;
      if (cur == 0)
        break;

      base = __ntdll_rd (cur + 0x10);
      name_len = __ntdll_rw (cur + 0x24);
      name = __ntdll_rd (cur + 0x28);

      match = 1;
      i = 0;
      while (want[i] != 0)
        {
          if (2 * i >= name_len)
            {
              match = 0;
              break;
            }
          if (__ntdll_rb (name + 2 * i) != want[i])
            {
              match = 0;
              break;
            }
          i = i + 1;
        }
      if (match != 0 && name_len == 2 * i)
        return base;

      cur = __ntdll_rd (cur);
      guard = guard + 1;
    }
  return 0;
}

/* Resolve one export by name out of a PE32 image already mapped at `base' in
 * this process.  IMAGE_NT_HEADERS32's export data directory is at +0x78;
 * the export directory's four arrays (NumberOfNames +24, AddressOfFunctions
 * +28, AddressOfNames +32, AddressOfNameOrdinals +36) are the same ordinary
 * 32-bit-RVA fields regardless of image bitness.  Returns 0 if `name' is not
 * among the exports. */
int
__ntdll_resolve_export (int base, char const *name)
{
  int e_lfanew;
  int nt;
  int export_rva;
  int export_dir;
  int num_names;
  int addr_funcs;
  int addr_names;
  int addr_ords;
  int i;
  int name_rva;
  int match;
  int j;
  int ord;
  int func_rva;

  e_lfanew = __ntdll_rd (base + 0x3C);
  nt = base + e_lfanew;
  export_rva = __ntdll_rd (nt + 0x78);
  export_dir = base + export_rva;

  num_names = __ntdll_rd (export_dir + 24);
  addr_funcs = __ntdll_rd (export_dir + 28);
  addr_names = __ntdll_rd (export_dir + 32);
  addr_ords = __ntdll_rd (export_dir + 36);

  i = 0;
  while (i < num_names)
    {
      name_rva = __ntdll_rd (base + addr_names + 4 * i);
      match = 1;
      j = 0;
      while (name[j] != 0)
        {
          if (__ntdll_rb (base + name_rva + j) != name[j])
            {
              match = 0;
              break;
            }
          j = j + 1;
        }
      if (match != 0 && __ntdll_rb (base + name_rva + j) == 0)
        {
          ord = __ntdll_rw (base + addr_ords + 2 * i);
          func_rva = __ntdll_rd (base + addr_funcs + 4 * ord);
          return base + func_rva;
        }
      i = i + 1;
    }
  return 0;
}

/* One ntdll routine, resolved by name rather than by resolve_all's fixed
 * index table -- see this file's own top-of-file comment.  ntdll.dll's base
 * is cached, since every caller wants a different export out of the same
 * module.  Returns 0 if ntdll.dll's export table has no such name. */
int __ntdll_base_cache;
int __ntdll_base_have;
void *
__ntdll_resolve (char const *name)
{
  if (__ntdll_base_have == 0)
    {
      __ntdll_base_cache = __ntdll_find_module ("ntdll.dll");
      __ntdll_base_have = 1;
    }
  if (__ntdll_base_cache == 0)
    return 0;
  return __ntdll_resolve_export (__ntdll_base_cache, name);
}

/* NtQueryInformationProcess with ProcessBasicInformation fills a
 * PROCESS_BASIC_INFORMATION: ExitStatus, PebBaseAddress, AffinityMask,
 * BasePriority, UniqueProcessId, InheritedFromUniqueProcessId -- six words
 * in a 32-bit process, so the process id is the word at index 4 and its
 * parent's at 5.  -1 is the pseudo-handle meaning this process.
 *
 * The id is in the TEB too, and is not read from there on purpose: that
 * would need an fs:0x18 read, which is one more instruction to write per
 * compiler, and this way needs none. */
int
__process_basic (int at)
{
  int NtQueryInformationProcess;
  int *info;
  int i;
  int rc;

  info = malloc (24);
  i = 0;
  while (i < 6)
    {
      info[i] = 0;
      i = i + 1;
    }

  NtQueryInformationProcess = __ntdll_resolve ("NtQueryInformationProcess");
  rc = __ntcall5 (NtQueryInformationProcess, -1, 0, info, 24, 0);
  if (rc < 0)
    return -1;
  return info[at];
}

/* Where one of the three standard handles is kept, so that dup2 can replace
 * it rather than only read it.  Redirection on Windows is exactly this: the
 * three words a child inherits are the three this points into. */
int *
__stdslot (int n)
{
  int params;

  /* PEB + 16 is ProcessParameters; its StandardInput is 24 in, and Output
     and Error follow it. */
  params = __ntdll_rd (__peb () + 16);
  return params + 24 + n * 4;
}

/* A file descriptor here is a Windows handle, with 0, 1 and 2 still meaning
 * the three standard streams as C says they do.  No real handle is that
 * small, so the two never collide and nothing above can tell. */
int
__handle (int filedes)
{
  int *slot;

  if (filedes > 2)
    return filedes;
  slot = __stdslot (filedes);
  return slot[0];
}

/* An ASCII string as the UTF-16 ntdll insists on. */
char *
__widen (char const *s)
{
  int n;
  int i;
  char *w;

  n = 0;
  while (s[n] != 0)
    n = n + 1;

  w = malloc (2 * n + 2);
  i = 0;
  while (i < 2 * n + 2)
    {
      w[i] = 0;
      i = i + 1;
    }
  i = 0;
  while (i < n)
    {
      w[2 * i] = s[i];
      i = i + 1;
    }
  return w;
}

/* A UNICODE_STRING over a DOS path: two words, the first holding Length and
 * MaximumLength as 16-bit halves, the second the widened text.  Both are byte
 * counts, and MaximumLength counts the terminator Length does not. */
int *
__dosustring (char const *path)
{
  int n;
  int *u;

  n = 0;
  while (path[n] != 0)
    n = n + 1;

  u = malloc (8);
  u[0] = 2 * n + 65536 * (2 * n + 2);
  u[1] = __widen (path);
  return u;
}

/* An OBJECT_ATTRIBUTES naming a file: six words, of which only the length,
 * the name and the attributes are ever anything but zero.  0 if the path
 * could not be made into an NT path at all. */
int *
__ntobject (char const *path)
{
  int RtlDosPathNameToNtPathName_U;
  char *wide;
  int *name;
  int *oa;
  int i;

  RtlDosPathNameToNtPathName_U = __ntdll_resolve ("RtlDosPathNameToNtPathName_U");

  name = malloc (8);
  name[0] = 0;
  name[1] = 0;
  wide = __widen (path);

  if (__ntcall4 (RtlDosPathNameToNtPathName_U, wide, name, 0, 0) == 0)
    return 0;

  oa = malloc (24);
  i = 0;
  while (i < 6)
    {
      oa[i] = 0;
      i = i + 1;
    }
  oa[0] = 24;                   /* Length: the size of this struct */
  oa[2] = name;                 /* ObjectName */
  oa[3] = 0x40;                 /* Attributes = OBJ_CASE_INSENSITIVE */
  return oa;
}

/* Copy a string into a fixed-size field of a struct, starting at `at`, and say
 * where the next piece would go.  Mes has strcpy, but not one that appends,
 * and lib/windows/uname.c has to build "10.0" out of two numbers. */
int
__strput (char *dst, int at, char const *src)
{
  int i;

  i = 0;
  while (src[i] != 0)
    {
      dst[at + i] = src[i];
      i = i + 1;
    }
  dst[at + i] = 0;
  return at + i;
}
