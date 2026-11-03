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

/* Stands in for lib/linux/ioctl3.c, which exists in this build for exactly
 * one caller: lib/m2/isatty.c asks TCGETS and takes success to mean the
 * descriptor is a terminal.
 *
 * Windows answers the same question differently.  NtQueryVolumeInformationFile
 * with FileFsDeviceInformation reports what kind of device is behind a handle,
 * and FILE_DEVICE_CONSOLE is 0x50.  Any other command has no counterpart and
 * fails, which is what ioctl does on Unix for a command a device does not
 * know.
 */

/* lib/m2/isatty.c defines this for itself; ioctl3 is the other half of the
 * same conversation, so it needs the number too. */
#define TCGETS 0x5401

int
ioctl3 (int filedes, unsigned long command, long data)
{
  int NtQueryVolumeInformationFile;
  int *iosb;
  int *info;
  int handle;
  int rc;

  if (command != TCGETS)
    return -1;

  iosb = malloc (8);
  iosb[0] = 0;
  iosb[1] = 0;
  /* FILE_FS_DEVICE_INFORMATION: DeviceType and Characteristics */
  info = malloc (8);
  info[0] = 0;
  info[1] = 0;
  handle = __handle (filedes);

  NtQueryVolumeInformationFile = __ntdll_resolve ("NtQueryVolumeInformationFile");
  rc = __ntcall5 (NtQueryVolumeInformationFile, handle, iosb, info, 8, 4);
  if (rc != 0)
    return -1;

  if (info[0] == 0x50)          /* FILE_DEVICE_CONSOLE */
    return 0;
  return -1;
}
