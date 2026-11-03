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

/* Checks lib/windows/x86-mes-m2/ntdll.c's __ntdll_resolve -- a native,
 * in-process PEB/export-table walk that finds an ntdll routine by name --
 * against every routine resolve_all already resolves by fixed index
 * (lib/m2/x86/ntdll-i386.hex2, include/windows/ntdll.h), by
 * comparing the two addresses for each.  If they always agree,
 * __ntdll_resolve is a safe replacement for the index table in anything
 * built with a C compiler; the hand-assembled crt1.M1/pe-end.M1 stage has
 * none and must go on using __ntdll (slot) regardless.  Also checks that
 * resolving a name ntdll does not export returns 0 rather than some other
 * module's address or garbage.
 *
 * Ported from stage0-pe32's own x86/Development/ntdll-resolve-test.c, which
 * checks the identical mechanism in the M2libc fork this was ported from. */

#include <stdio.h>
#include <windows/ntdll.h>

int
main (int argc, char **argv)
{
  int ok;
  void *a;
  void *b;

  ok = 1;

  a = __ntdll (NT_CREATE);     b = __ntdll_resolve ("NtCreateFile");
  if (a != b) { eputs ("MISMATCH NtCreateFile\n"); ok = 0; }

  a = __ntdll (NT_READ);       b = __ntdll_resolve ("NtReadFile");
  if (a != b) { eputs ("MISMATCH NtReadFile\n"); ok = 0; }

  a = __ntdll (NT_WRITE);      b = __ntdll_resolve ("NtWriteFile");
  if (a != b) { eputs ("MISMATCH NtWriteFile\n"); ok = 0; }

  a = __ntdll (NT_CLOSE);      b = __ntdll_resolve ("NtClose");
  if (a != b) { eputs ("MISMATCH NtClose\n"); ok = 0; }

  a = __ntdll (NT_EXIT);       b = __ntdll_resolve ("NtTerminateProcess");
  if (a != b) { eputs ("MISMATCH NtTerminateProcess\n"); ok = 0; }

  a = __ntdll (NT_RTLPATH);    b = __ntdll_resolve ("RtlDosPathNameToNtPathName_U");
  if (a != b) { eputs ("MISMATCH RtlDosPathNameToNtPathName_U\n"); ok = 0; }

  a = __ntdll (NT_QUERYATTR);  b = __ntdll_resolve ("NtQueryAttributesFile");
  if (a != b) { eputs ("MISMATCH NtQueryAttributesFile\n"); ok = 0; }

  a = __ntdll (NT_GETCWD);     b = __ntdll_resolve ("RtlGetCurrentDirectory_U");
  if (a != b) { eputs ("MISMATCH RtlGetCurrentDirectory_U\n"); ok = 0; }

  a = __ntdll (NT_DUP);        b = __ntdll_resolve ("NtDuplicateObject");
  if (a != b) { eputs ("MISMATCH NtDuplicateObject\n"); ok = 0; }

  a = __ntdll (NT_TIME);       b = __ntdll_resolve ("NtQuerySystemTime");
  if (a != b) { eputs ("MISMATCH NtQuerySystemTime\n"); ok = 0; }

  a = __ntdll (NT_QUERYVOL);   b = __ntdll_resolve ("NtQueryVolumeInformationFile");
  if (a != b) { eputs ("MISMATCH NtQueryVolumeInformationFile\n"); ok = 0; }

  a = __ntdll (NT_WAIT);       b = __ntdll_resolve ("NtWaitForSingleObject");
  if (a != b) { eputs ("MISMATCH NtWaitForSingleObject\n"); ok = 0; }

  a = __ntdll (NT_QUERYPROC);  b = __ntdll_resolve ("NtQueryInformationProcess");
  if (a != b) { eputs ("MISMATCH NtQueryInformationProcess\n"); ok = 0; }

  a = __ntdll (NT_DELETE);     b = __ntdll_resolve ("NtDeleteFile");
  if (a != b) { eputs ("MISMATCH NtDeleteFile\n"); ok = 0; }

  a = __ntdll (NT_VERSION);    b = __ntdll_resolve ("RtlGetVersion");
  if (a != b) { eputs ("MISMATCH RtlGetVersion\n"); ok = 0; }

  a = __ntdll (NT_MAKEPARAMS); b = __ntdll_resolve ("RtlCreateProcessParameters");
  if (a != b) { eputs ("MISMATCH RtlCreateProcessParameters\n"); ok = 0; }

  a = __ntdll (NT_CREATEPROC); b = __ntdll_resolve ("RtlCreateUserProcess");
  if (a != b) { eputs ("MISMATCH RtlCreateUserProcess\n"); ok = 0; }

  a = __ntdll (NT_RESUME);     b = __ntdll_resolve ("NtResumeThread");
  if (a != b) { eputs ("MISMATCH NtResumeThread\n"); ok = 0; }

  a = __ntdll (NT_ALLOC);      b = __ntdll_resolve ("NtAllocateVirtualMemory");
  if (a != b) { eputs ("MISMATCH NtAllocateVirtualMemory\n"); ok = 0; }

  a = __ntdll (NT_GETCONTEXT); b = __ntdll_resolve ("NtGetContextThread");
  if (a != b) { eputs ("MISMATCH NtGetContextThread\n"); ok = 0; }

  a = __ntdll (NT_SETCONTEXT); b = __ntdll_resolve ("NtSetContextThread");
  if (a != b) { eputs ("MISMATCH NtSetContextThread\n"); ok = 0; }

  a = __ntdll (NT_WRITEVM);    b = __ntdll_resolve ("NtWriteVirtualMemory");
  if (a != b) { eputs ("MISMATCH NtWriteVirtualMemory\n"); ok = 0; }

  a = __ntdll (NT_WOW64QINFO); b = __ntdll_resolve ("NtWow64QueryInformationProcess64");
  if (a != b) { eputs ("MISMATCH NtWow64QueryInformationProcess64\n"); ok = 0; }

  a = __ntdll (NT_WOW64READVM); b = __ntdll_resolve ("NtWow64ReadVirtualMemory64");
  if (a != b) { eputs ("MISMATCH NtWow64ReadVirtualMemory64\n"); ok = 0; }

  /* NT_CLONE (RtlCloneUserProcess) is not checked above: nothing in this
   * port calls it, and Wine's ntdll has no such export.  A genuinely
   * made-up name is checked instead, to confirm a miss returns 0 rather
   * than the last thing found. */
  b = __ntdll_resolve ("NtThisExportDoesNotExist");
  if (b != 0) { eputs ("MISMATCH: bogus name resolved to something\n"); ok = 0; }

  if (ok)
    eputs ("ALL RESOLVED OK\n");
  else
    eputs ("FAILED\n");
  if (ok == 0)
    return 1;
  return 0;
}
