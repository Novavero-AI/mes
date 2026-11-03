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
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Start a program, wait for it, and answer with what it exited with.
 *
 * The counterpart of lib/posix/spawn.c, which does this with fork, execve and
 * waitpid.  There is no fork here, and lib/stub/fork.c answers -1:
 * RtlCloneUserProcess is missing from Wine, and under WOW64 the child it
 * makes of a 32-bit process runs only after a 64-bit trampoline and a write
 * into the child's copy of ntdll, which is not what starting a program
 * should require.
 *
 * __spawn, which execve already stands on, starts the program directly; this
 * is execve without the _exit at the end, because a caller that wants to go
 * on running is the whole point.  See lib/windows/execve.c for how a child is
 * made and how argv survives being flattened into one command line.
 *
 * The status is the whole POSIX word, exit code in the second byte up, the
 * way waitpid here reports it.
 */

int
spawn (char const *file_name, char *const argv[])
{
  int pid;
  int *status;

  pid = __spawn (file_name, argv, environ);
  if (pid <= 0)
    return -1;

  status = malloc (4);
  status[0] = 0;
  if (waitpid (pid, status, 0) < 0)
    return -1;
  return status[0];
}
