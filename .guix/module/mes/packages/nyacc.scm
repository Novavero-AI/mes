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

(define-module (mes packages nyacc)
  #:use-module (gnu packages guile)
  #:use-module (gnu packages pkg-config)
  #:use-module (guix build-system gnu)
  #:use-module (guix download)
  #:use-module (guix licenses)
  #:use-module (guix packages)
  #:use-module (guix utils))

(define-public nyacc-0.99
  (package
    (name "nyacc")
    (version "0.99.0")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/nyacc/nyacc-"
                                  version ".tar.gz"))
              (sha256
               (base32
                "0hl5qxx19i4x1r0839sxm19ziqq65g4hy97yik81cc2yb9yvgyv3"))
              (modules '((guix build utils)))
              (snippet
               '(begin
                  (substitute* (find-files "." "^Makefile\\.in$")
                    (("^SITE_SCM_DIR =.*")
                     "SITE_SCM_DIR = \
@prefix@/share/guile/site/@GUILE_EFFECTIVE_VERSION@\n")
                    (("^SITE_SCM_GO_DIR =.*")
                     "SITE_SCM_GO_DIR = \
@prefix@/lib/guile/@GUILE_EFFECTIVE_VERSION@/site-ccache\n")
                    (("^INFODIR =.*")
                     "INFODIR = @prefix@/share/info\n")
                    (("^DOCDIR =.*")
                     "DOCDIR = @prefix@/share/doc/$(PACKAGE_TARNAME)\n"))
                  #t))))
    (build-system gnu-build-system)
    (native-inputs (list pkg-config))
    (inputs (list guile-2.2))
    (synopsis "LALR(1) Parser Generator in Guile")
    (description
     "NYACC is an LALR(1) parser generator implemented in Guile.
The syntax and nomenclature should be considered not stable.  It comes with
extensive examples, including parsers for the Javascript and C99 languages.")
    (home-page "https://savannah.nongnu.org/projects/nyacc")
    (license (list gpl3+ lgpl3+))))

(define-public nyacc-1.00.2
  (package
    (inherit nyacc-0.99)
    (version "1.00.2")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/nyacc/nyacc-"
                                  version ".tar.gz"))
              (modules '((guix build utils)))
              (snippet
               '(begin
                  (substitute* (find-files "." "^Makefile\\.in$")
                    (("^SITE_SCM_DIR =.*")
                     "SITE_SCM_DIR = \
@prefix@/share/guile/site/@GUILE_EFFECTIVE_VERSION@\n")
                    (("^SITE_SCM_GO_DIR =.*")
                     "SITE_SCM_GO_DIR = \
@prefix@/lib/guile/@GUILE_EFFECTIVE_VERSION@/site-ccache\n")
                    (("^INFODIR =.*")
                     "INFODIR = @prefix@/share/info\n")
                    (("^DOCDIR =.*")
                     "DOCDIR = @prefix@/share/doc/$(PACKAGE_TARNAME)\n"))
                  #t))
              (sha256
               (base32
                "065ksalfllbdrzl12dz9d9dcxrv97wqxblslngsc6kajvnvlyvpk"))))
    (inputs (list guile-3.0))))

(define-public nyacc-1.02.0
  (package
    (inherit nyacc-0.99)
    (version "1.02.0")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/nyacc/nyacc-"
                                  version ".tar.gz"))
              (sha256
               (base32
                "1a0iffvi0h2y8rlkg9vkzgvc4afywmf3kkmjdnhq9r7m4crjpcyl"))
              (modules '((guix build utils)))
              (snippet
               '(begin
                  (substitute* "configure"
                    (("GUILE_GLOBAL_SITE=\\$prefix.*")
                     "GUILE_GLOBAL_SITE=\
$prefix/share/guile/site/$GUILE_EFFECTIVE_VERSION\n"))
                  #t))))
    (inputs (list guile-3.0))))

(define-public nyacc-1.03.0
  (package
    (inherit nyacc-0.99)
    (version "1.03.0")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/nyacc/nyacc-"
                                  version ".tar.gz"))
              (sha256
               (base32
                "1vdiqpm3p0ndmpmkzcpkpjvgklfsk4wxrhkixdxbczpafdfl635p"))
              (modules '((guix build utils)))
              (snippet
               '(begin
                  (substitute* "configure"
                    (("GUILE_GLOBAL_SITE=\\$prefix.*")
                     "GUILE_GLOBAL_SITE=\
$prefix/share/guile/site/$GUILE_EFFECTIVE_VERSION\n"))
                  #t))))
    (inputs (list guile-3.0))))

(define-public nyacc-1.04.0
  (package
    (inherit nyacc-0.99)
    (version "1.04.0")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/nyacc/nyacc-"
                                  version ".tar.gz"))
              (sha256
               (base32
                "0m3mcdaslvvr2iav8ga146hzsja2hdj4656pszljb4q2q7h25ip5"))
              (modules '((guix build utils)))
              (snippet
               '(begin
                  (substitute* "configure"
                    (("GUILE_GLOBAL_SITE=\\$prefix.*")
                     "GUILE_GLOBAL_SITE=\
$prefix/share/guile/site/$GUILE_EFFECTIVE_VERSION\n"))
                  #t))))
    (inputs (list guile-3.0))))

(define-public nyacc-1.05.1
  (package
    (inherit nyacc-0.99)
    (version "1.05.1")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/nyacc/nyacc-"
                                  version ".tar.gz"))
              (sha256
               (base32
                "1ck3gyzln5dhamp317nv3waych12mczj05dm4wdblij6ab0l4863"))
              (modules '((guix build utils)))
              (snippet
               '(begin
                  (substitute* "configure"
                    (("GUILE_GLOBAL_SITE=\\$prefix.*")
                     "GUILE_GLOBAL_SITE=\
$prefix/share/guile/site/$GUILE_EFFECTIVE_VERSION\n"))
                  #t))))
    (inputs (list guile-3.0))))

(define-public nyacc-1.06.5
  (package
    (inherit nyacc-0.99)
    (version "1.06.5")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/nyacc/nyacc-"
                                  version ".tar.gz"))
              (sha256
               (base32
                "1fbzz9bm4mkz4j40l2z02zjlbqj82dmv2ayz83zl3j8gj6z3lpdg"))
              (modules '((guix build utils)))
              (snippet
               '(substitute* "configure"
                  (("GUILE_GLOBAL_SITE=\\$prefix.*")
                   "GUILE_GLOBAL_SITE=\
$prefix/share/guile/site/$GUILE_EFFECTIVE_VERSION\n")))))
    (inputs (list guile-3.0))))

(define-public nyacc-1.07.0
  (package
    (inherit nyacc-0.99)
    (version "1.07.0")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/nyacc/nyacc-"
                                  version ".tar.gz"))
              (sha256
               (base32
                "01qb6h6bk684z8xda5d71q5f5l2z3q9jjz36if3jbpjc7b8dxjap"))
              (modules '((guix build utils)))
              (snippet
               '(substitute* "configure"
                  (("GUILE_GLOBAL_SITE=\\$prefix.*")
                   "GUILE_GLOBAL_SITE=\
$prefix/share/guile/site/$GUILE_EFFECTIVE_VERSION\n")))))
    (inputs (list guile-3.0))))

(define-public nyacc-1.08.1
  (package
    (inherit nyacc-0.99)
    (version "1.08.1")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/nyacc/nyacc-"
                                  version ".tar.gz"))
              (sha256
               (base32
                "1vrz3pnlr3njwk6ksz85slcwawi8ngiqbw94wd9x3mgv85vsfmys"))
              (modules '((guix build utils)))
              (snippet
               '(substitute* "configure"
                  (("GUILE_GLOBAL_SITE=\\$prefix.*")
                   "GUILE_GLOBAL_SITE=\
$prefix/share/guile/site/$GUILE_EFFECTIVE_VERSION\n")))))
    (inputs (list guile-3.0))))

(define-public nyacc-1.08.7
  (package
    (inherit nyacc-0.99)
    (version "1.08.7")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/nyacc/nyacc-"
                                  version ".tar.gz"))
              (sha256
               (base32
                "0kfkazwchlpbrcqin5c02adwzf9vn76x7ix3hq272dw21nczavii"))
              (modules '((guix build utils)))
              (snippet
               '(substitute* "configure"
                  (("GUILE_GLOBAL_SITE=\\$prefix.*")
                   "GUILE_GLOBAL_SITE=\
$prefix/share/guile/site/$GUILE_EFFECTIVE_VERSION\n")))))
    (inputs (list guile-3.0))))

(define-public nyacc-1.09.0
  (package
    (inherit nyacc-0.99)
    (version "1.09.0")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/nyacc/nyacc-"
                                  version ".tar.gz"))
              (sha256
               (base32
                "0psbdpirz3b91qfk5as4rp7pf75940wsvpk2xfvvlbqcpicvn8p2"))
              (modules '((guix build utils)))
              (snippet
               '(substitute* "configure"
                  (("GUILE_GLOBAL_SITE=\\$prefix.*")
                   "GUILE_GLOBAL_SITE=\
$prefix/share/guile/site/$GUILE_EFFECTIVE_VERSION\n")))))
    (inputs (list guile-3.0))))

(define-public nyacc-1.09.4
  (package
    (inherit nyacc-0.99)
    (version "1.09.4")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/nyacc/nyacc-"
                                  version ".tar.gz"))
              (sha256
               (base32
                "0h81sywxbkhwcx2gcax8ymag832aq51bxaqnnxcnpz0qx7a4crgf"))
              (modules '((guix build utils)))
              (snippet
               '(substitute* "configure"
                  (("GUILE_GLOBAL_SITE=\\$prefix.*")
                   "GUILE_GLOBAL_SITE=\
$prefix/share/guile/site/$GUILE_EFFECTIVE_VERSION\n")))))
    (inputs (list guile-3.0))))

(define-public nyacc-1.09.5
  (package
    (inherit nyacc-0.99)
    (version "1.09.5")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/nyacc/nyacc-"
                                  version ".tar.gz"))
              (sha256
               (base32
                "12l6xmbvvmam55jzdqisrd7cnb4qfxk2g1aa3gxmk32r3sb57kb0"))
              (modules '((guix build utils)))
              (snippet
               '(substitute* "configure"
                  (("GUILE_GLOBAL_SITE=\\$prefix.*")
                   "GUILE_GLOBAL_SITE=\
$prefix/share/guile/site/$GUILE_EFFECTIVE_VERSION\n")))))
    (inputs (list guile-3.0))))

(define-public nyacc-2.00.0
  (package
    (inherit nyacc-0.99)
    (version "2.00.0")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/nyacc/nyacc-"
                                  version ".tar.gz"))
              (sha256
               (base32
                "08ssi79dpwhvkfbll02z6bx67r3xgnxvlkrnqir4d08m4fbid6h8"))
              (modules '((guix build utils)))
              (snippet
               '(substitute* "configure"
                  (("GUILE_GLOBAL_SITE=\\$prefix.*")
                   "GUILE_GLOBAL_SITE=\
$prefix/share/guile/site/$GUILE_EFFECTIVE_VERSION\n")))))
    (inputs (list guile-3.0))))

(define-public nyacc-2.00.6
  (package
    (inherit nyacc-0.99)
    (version "2.00.6")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/nyacc/nyacc-"
                                  version ".tar.gz"))
              (sha256
               (base32
                "05397dqxv6sargkmz8kbgbrjvwcjrqcqc91bkmfxgmwm4a9sdg6i"))
              (modules '((guix build utils)))
              (snippet
               '(substitute* "configure"
                  (("GUILE_GLOBAL_SITE=\\$prefix.*")
                   "GUILE_GLOBAL_SITE=\
$prefix/share/guile/site/$GUILE_EFFECTIVE_VERSION\n")))))
    (inputs (list guile-3.0))))

(define-public nyacc-2.02.2
  (package
    (inherit nyacc-0.99)
    (version "2.02.2")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/nyacc/nyacc-"
                                  version ".tar.gz"))
              (sha256
               (base32
                "01829c24v531036rj8grcwx4hmiy3f0jznc9zbfa4wrslmq566k9"))
              (modules '((guix build utils)))
              (snippet
               '(substitute* "configure"
                  (("GUILE_GLOBAL_SITE=\\$prefix.*")
                   "GUILE_GLOBAL_SITE=\
$prefix/share/guile/site/$GUILE_EFFECTIVE_VERSION\n")))))
    (inputs (list guile-3.0))))

(define-public nyacc-2.02.4
  (package
    (inherit nyacc-0.99)
    (version "2.02.4")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/nyacc/nyacc-"
                                  version ".tar.gz"))
              (sha256
               (base32
                "0ml38lwhdd49b90mmx4aggccrkp80vxy13g4z53ma5d11gw767hy"))
              (modules '((guix build utils)))
              (snippet
               '(substitute* "configure"
                  (("GUILE_GLOBAL_SITE=\\$prefix.*")
                   "GUILE_GLOBAL_SITE=\
$prefix/share/guile/site/$GUILE_EFFECTIVE_VERSION\n")))))
    (inputs (list guile-3.0))))

(define-public nyacc-3.00.0
  (package
    (inherit nyacc-0.99)
    (version "3.00.0")
    (source (origin
              (method url-fetch)
              (uri (string-append
                     "https://github.com/mwette/nyacc/archive/refs/tags/V"
                     version ".tar.gz"))
              (sha256
               (base32
                "17ymkgxxzhdn64h01jp964svzx6hdqr2a1p0csinsnyvb6qhghnv"))
              (modules '((guix build utils)))
              (snippet
               '(substitute* "configure"
                  (("GUILE_GLOBAL_SITE=\\$prefix.*")
                   "GUILE_GLOBAL_SITE=\
$prefix/share/guile/site/$GUILE_EFFECTIVE_VERSION\n")))))
    (inputs (list guile-3.0))))
