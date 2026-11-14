;;; GNU Mes --- Maxwell Equations of Software
;;; Copyright © 2016-2026 Janneke Nieuwenhuizen <janneke@gnu.org>
;;; Copyright © 2023,2025 Ekaitz Zarraga <ekaitz@elenq.tech>
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

(use-modules (guix utils))
(eval-when (expand load)
  (define %guix-modules (and=> (or (current-source-directory)
                                   (and=> (current-filename) dirname))
                               (compose dirname dirname)))
  (define %source-dir (and=> %guix-modules (compose dirname dirname))))
(add-to-load-path %guix-modules)

(define-module (mes packages mes)
  #:use-module (ice-9 popen)
  #:use-module (ice-9 rdelim)

  #:use-module (gnu packages build-tools)
  #:use-module (gnu packages cross-base)
  #:use-module (gnu packages gdb)
  #:use-module (gnu packages graphviz)
  #:use-module (gnu packages guile)
  #:use-module (gnu packages man)
  #:use-module (gnu packages perl)
  #:use-module (gnu packages texinfo)
  #:use-module (gnu packages time)

  #:use-module (guix build-system gnu)
  #:use-module (guix gexp)
  #:use-module (guix download)
  #:use-module (guix git-download)
  #:use-module (guix licenses)
  #:use-module (guix packages)
  #:use-module (guix utils)

  #:use-module (mes packages nyacc)
  #:use-module (mes packages stage0))

(define %guix-modules (and=> (or (current-source-directory)
                                 (and=> (current-filename) dirname))
                             (compose dirname dirname)))
(define %source-dir (and=> %guix-modules (compose dirname dirname)))

(define-public mes
  (package
    (name "mes")
    (version #!mes!# "0.28")
    (source (origin
              (method url-fetch)
              (uri (string-append
                    "https://ftp.gnu.org/pub/gnu/mes/mes-" version ".tar.gz"))
              (sha256
               (base32 #!mes!# "0hkqy0pllpam8yj1b9cs708z29ir15jbhvda5v6xl4ylfq5jxn92"))))
    (build-system gnu-build-system)
    (supported-systems '("aarch64-linux" "armhf-linux" "i686-linux"
                         "x86_64-linux" "riscv64-linux"))
    (propagated-inputs (list mescc-tools nyacc-3.00.0))
    (native-inputs
     (append
      (list guile-3.0)
      (let ((target-system (or (%current-target-system)
                               (%current-system))))
        (cond
         ((string-prefix? "x86_64-linux" target-system)
          ;; Use cross-compiler rather than #:system "i686-linux" to get
          ;; MesCC 64 bit .go files installed ready for use with Guile.
          (list (cross-binutils "i686-unknown-linux-gnu")
                (cross-gcc "i686-unknown-linux-gnu")))
         ((string-prefix? "aarch64-linux" target-system)
          ;; Use cross-compiler rather than #:system "armhf-linux" to get
          ;; MesCC 64 bit .go files installed ready for use with Guile.
          (let ((triplet "arm-linux-gnueabihf"))
            (list (cross-binutils triplet) (cross-gcc triplet))))
         (else
          '())))
      (list graphviz help2man m2-planet
            perl                        ;build-aux/gitlog-to-changelog
            texinfo)))
    (arguments
     `(#:strip-binaries? #f)) ; binutil's strip b0rkes MesCC/M1/hex2 binaries
    (native-search-paths
     (list (search-path-specification
             (variable "C_INCLUDE_PATH")
             (files '("include")))
           (search-path-specification
             (variable "LIBRARY_PATH")
             (files '("lib")))
           (search-path-specification
             (variable "MES_PREFIX")
             (separator #f)
             (files '("")))))
    (synopsis "Scheme interpreter and C compiler for full source bootstrapping")
    (description
     "GNU Mes--Maxwell Equations of Software--brings the Reduced Binary Seed
bootstrap to Guix and aims to help create full source bootstrapping for
GNU/Linux distributions.  It consists of a mutual self-hosting Scheme
interpreter in C and a Nyacc-based C compiler in Scheme and is compatible with
Guile.")
    (home-page "https://www.gnu.org/software/mes")
    (license gpl3+)))

(define vcs-file?
  (or (git-predicate %source-dir)
      (const #t)))

(define-public mes.git
  (package
    (inherit mes)
    (version "git")
    (source (local-file %source-dir
                        #:recursive? #t
                        #:select? vcs-file?))
    (native-inputs
     `(("gdb" ,gdb)
       ("gnulib" ,gnulib)
       ("time" ,time)
       ,@(package-native-inputs mes)))))
