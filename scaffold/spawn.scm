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

;;; system* where fork answers -1, as it does on Windows.  Run it from the
;;; top of the source tree, naming the mes to start:
;;;
;;;     bin/mes-m2.exe scaffold/spawn.scm bin/mes-m2.exe
;;;
;;; The child prints "spawned" and exits 7.  This exits 0 only if fork
;;; answered -1 and system* answered the child's status.

;;; Code:

(define child (cadr (command-line)))

(define pid (primitive-fork))
(display "primitive-fork: ")
(display pid)
(newline)
(if (not (= pid -1))
    (exit 1))

(define status (system* child "-c" "(display 'spawned) (newline) (exit 7)"))
(display "system*: ")
(display status)
(newline)
(exit (if (= (status:exit-val status) 7) 0 1))
