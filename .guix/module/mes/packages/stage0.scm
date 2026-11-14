;;; GNU Mes --- Maxwell Equations of Software
;;; Copyright © 2016-2026 Janneke Nieuwenhuizen <janneke@gnu.org>
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

(define-module (mes packages stage0)
  #:use-module (gnu packages base)
  #:use-module (guix build-system gnu)
  #:use-module (guix gexp)
  #:use-module (guix download)
  #:use-module (guix licenses)
  #:use-module (guix packages)
  #:use-module (guix utils))

(define-public mescc-tools
  (package
    (name "mescc-tools")
    (version "1.7.0")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/" name "/"
                                  name "-" version ".tar.gz"))
              (sha256
               (base32
                "0p2pnvci9vglmf7zas12ay2v5gnrwafqsqqw1dfyb2bgayzzg0mn"))))
    (build-system gnu-build-system)
    (supported-systems '("i686-linux" "x86_64-linux"
                         "armhf-linux" "aarch64-linux"
                         "riscv32-linux" "riscv64-linux"))
    (native-inputs (list which))
    (arguments
     (list
      #:make-flags #~(list (string-append "PREFIX=" #$output))
      #:test-target "test"
      #:phases #~(modify-phases %standard-phases
                   (delete 'configure))))
    (synopsis "Tools for the full source bootstrapping process")
    (description
     "Mescc-tools is a collection of tools for use in a full source
bootstrapping process.  It consists of the M1 macro assembler, the hex2
linker, the blood-elf symbol table generator, the kaem shell, exec_enable and
get_machine.")
    (home-page "https://savannah.nongnu.org/projects/mescc-tools")
    (license gpl3+)))

(define-public m2-planet
  (package
    (name "m2-planet")
    (version "1.13.1")
    (source (origin
              (method url-fetch)
              (uri (string-append
                    "https://github.com/oriansj/M2-Planet/releases/download/"
                    "Release_" version "/" name "-" version ".tar.gz"))
              (sha256
               (base32
                "0wv1l9ngf86wmh9pliik6r9j8yibq6dcm0bh0jlqn5cffbi33igx"))))
    (native-inputs (list mescc-tools))
    (build-system gnu-build-system)
    (supported-systems '("i686-linux" "x86_64-linux"
                         "armhf-linux" "aarch64-linux"
                         "riscv32-linux" "riscv64-linux"))
    (arguments
     (list
      #:make-flags #~(list (string-append "PREFIX=" (assoc-ref %outputs "out"))
                           (string-append "CC=" #$(cc-for-target)))
      #:tests? #f
      #:phases #~(modify-phases %standard-phases
                   (delete 'bootstrap)
                   (delete 'configure))))
    (synopsis "The PLAtform NEutral Transpiler")
    (description
     "M2-Planet, The PLAtform NEutral Transpiler, when combined with
mescc-tools compiles a subset of the C language into working binaries
with introspective steps inbetween.")
    (home-page "https://github.com/oriansj/m2-planet")
    (license gpl3+)))
