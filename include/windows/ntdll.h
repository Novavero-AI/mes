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

#ifndef __MES_WINDOWS_NTDLL_H
#define __MES_WINDOWS_NTDLL_H

/* The interface every file in lib/windows/ is written against, whichever
 * compiler is building it.  What cannot be said in C -- the PEB read, and
 * the slot table that only a hand-assembled stage has -- lives in
 * lib/windows/<arch>-mes-<compiler>/ntlow.c; everything else is here.
 *
 * Which ntdll routine lib/m2/x86/ntdll-i386.hex2 resolved into which slot.
 * The numbering is decided there and nowhere else, so this half is copied
 * from stage0-pe32 fcc842d (x86/M2libc-windows/ntdll-slots.h), which that
 * project generates from the same list it generates the .hex2 from.  If the
 * .hex2 beside it is updated, this must be too.
 *
 * Nothing compiled from C in this port calls __ntdll (slot) by one of these
 * any more -- see ntdll.c's own top-of-file comment for __ntdll_resolve,
 * which replaced it everywhere above the hand-assembled stages.  These
 * stay, because lib/m2/x86/ntdll-i386.hex2 still fills the same table for
 * those stages, which have no C compiler yet to call __ntdll_resolve with.
 */
#define NT_CREATE      0   /* NtCreateFile: open a file, or make one */
#define NT_READ        1   /* NtReadFile: read */
#define NT_WRITE       2   /* NtWriteFile: write */
#define NT_CLOSE       3   /* NtClose: close, and any other handle */
#define NT_EXIT        4   /* NtTerminateProcess: _exit */
#define NT_RTLPATH     5   /* RtlDosPathNameToNtPathName_U: a DOS path becomes an NT path */
#define NT_SETINFO     6   /* NtSetInformationFile: lseek, and unlink */
#define NT_QUERYINFO   7   /* NtQueryInformationFile: lseek, and fstat */
#define NT_QUERYATTR   8   /* NtQueryAttributesFile: access */
#define NT_QUERYFULL   9   /* NtQueryFullAttributesFile: stat */
#define NT_SETCWD      10  /* RtlSetCurrentDirectory_U: chdir */
#define NT_GETCWD      11  /* RtlGetCurrentDirectory_U: getcwd */
#define NT_DUP         12  /* NtDuplicateObject: dup and dup2 */
#define NT_TIME        13  /* NtQuerySystemTime: time, gettimeofday, clock_gettime */
#define NT_QUERYVOL    14  /* NtQueryVolumeInformationFile: isatty */
#define NT_WAIT        15  /* NtWaitForSingleObject: waitpid */
#define NT_QUERYPROC   16  /* NtQueryInformationProcess: waitpid, for the exit status */
#define NT_DELETE      17  /* NtDeleteFile: unlink and rmdir */
#define NT_VERSION     18  /* RtlGetVersion: uname */
#define NT_MAKEPARAMS  19  /* RtlCreateProcessParameters: __spawn, for the child's parameter block */
#define NT_CREATEPROC  20  /* RtlCreateUserProcess: __spawn */
#define NT_RESUME      21  /* NtResumeThread: __spawn: a new process starts suspended */
#define NT_ALLOC       22  /* NtAllocateVirtualMemory: a real brk, for a program too big to live inside the image */
#define NT_CLONE       23  /* RtlCloneUserProcess: fork; absent on wine, where the slot stays 0 */
#define NT_GETCONTEXT  24  /* NtGetContextThread: fork: the context a suspended child would start with */
#define NT_SETCONTEXT  25  /* NtSetContextThread: fork: point that child at where its parent was instead */
#define NT_WRITEVM     26  /* NtWriteVirtualMemory: fork: copy this process's memory into that child */
#define NT_READVM      27  /* NtReadVirtualMemory: fork: watch for the child to say it has parked */
#define NT_SUSPEND     28  /* NtSuspendThread: fork: stop that child before overwriting it */
#define NT_WOW64QINFO  29  /* NtWow64QueryInformationProcess64: wow64 clone fix: this process's own 64-bit PEB address */
#define NT_WOW64READVM 30  /* NtWow64ReadVirtualMemory64: wow64 clone fix: read the 64-bit ntdll/wow64cpu images to resolve BTCpuSimulate */

int *__iosb ();
void *__ntdll (int slot);
int __peb (void);
void *__ntdll_resolve (char const *name);
int *__stdslot (int n);
int __handle (int filedes);
int __process_basic (int at);
char *__widen (char const *s);
int *__dosustring (char const *path);
int *__ntobject (char const *path);
int __strput (char *dst, int at, char const *src);
int __spawn (char const *file_name, char **argv, char **env);

#endif /* __MES_WINDOWS_NTDLL_H */
