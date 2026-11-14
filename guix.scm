;;; guix.scm -- Guix package definition

;;; GNU Mes --- Maxwell Equations of Software
;;; Copyright © 2018-2026 Janneke Nieuwenhuizen <janneke@gnu.org>
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
;;
;; This file is used to setup a reproducible development environment
;; using the GNU Guix package manager.  It can be used implicitly by
;; envrc by adding
;;
;;    eval $($guix shell --search-paths)
;;
;; to your .envrc file.
;;
;; To build and install GNU Mes, run:
;;
;;    guix install -f guix.scm
;;
;; To build Mes, but not install it, run:
;;
;;    guix build -f guix.scm
;;
;; To use as to setup a development environment manually, run:
;;
;;    guix time-machine --commit=d5b0c1c2e3765191edb49257b66d5f99195a987b -- shell
;;
;; Substitutes may be available through: http://dezynen.org:8181
;;
;;; Code:

(define %source-dir (and=> (current-filename) dirname))
(add-to-load-path (in-vicinity (or %source-dir ".") ".guix/module"))
(use-modules (mes packages mes))

;; Return mes.git package for `guix build/install/shell'.
mes.git
