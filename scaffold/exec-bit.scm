;;; GNU Mes --- Maxwell Equations of Software
;;; Copyright © 2026 Novavero AI Inc.
;;;
;;; This file is part of GNU Mes.
;;;
;;; GNU Mes is free software; you can redistribute it and/or modify it
;;; under the terms of the GNU General Public License as published by
;;; the Free Software Foundation; either version 3 of the License, or (at
;;; your option) any later version.
;;;
;;; GNU Mes is distributed in the hope that it will be useful, but
;;; WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;
;;; You should have received a copy of the GNU General Public License
;;; along with GNU Mes.  If not, see <http://www.gnu.org/licenses/>.

;;; Commentary:

;;; Make a file with a mode, or give a file a mode, through the C library,
;;; for a check outside Mes to read the result.  Run it from the top of the
;;; source tree, with MODE in octal:
;;;
;;;     bin/mes-m2.exe scaffold/exec-bit.scm open FILE MODE
;;;     bin/mes-m2.exe scaffold/exec-bit.scm chmod FILE MODE
;;;
;;; open makes FILE with open-output-file, and chmod calls chmod.  On
;;; Windows the owner execute bit is FILE's nova.exec stream.  This exits 1
;;; if open fails; chmod answers nothing to Scheme, so only FILE can say
;;; whether it worked.

;;; Code:

(define arguments (cdr (command-line)))
(define operation (car arguments))
(define file (cadr arguments))
(define mode (string->number (caddr arguments) 8))

(cond ((equal? operation "open")
       (if (< (open-output-file file mode) 0)
           (exit 1)))
      ((equal? operation "chmod")
       (chmod file mode))
      (else
       (exit 2)))
