;;; GNU Mes --- Maxwell Equations of Software
;;; Copyright © 2016,2017,2018,2022 Janneke Nieuwenhuizen <janneke@gnu.org>
;;; Copyright © 2023 Ekaitz Zarraga <ekaitz@elenq.tech>
;;; Copyright © 2024,2025,2026 Timothy Sample <samplet@ngyro.com>
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

;; Define basic keywords.  This is only used to make the expander easier
;; to read.  Once the expander is running, it is in charge of inserting
;; the 'core:' forms.
(core:define lambda core:lambda)
(core:define quote core:quote)
(core:define begin core:begin)
(core:define if core:if)
(core:define define core:define)
(core:define set! core:set!)

;; Boot just enough type checking to run the loader and expander
(define (symbol? x)
  (eq? (core:type x) <cell:symbol>))

(define (number? x)
  (eq? (core:type x) <cell:number>))

(define (string? x)
  (eq? (core:type x) <cell:string>))

(define (port? x)
  (eq? (core:type x) <cell:port>))

(define (closure? x)
  (eq? (core:type x) <cell:closure>))

;; The expander
(define %macro-table (make-hash-table))

(define (define-macro! name proc)
  (if (closure? proc)
      (hashq-set! %macro-table name proc)
      (error "define-macro!: not a closure" proc)))

(define (expand form)
  (if (pair? form)
      (if (symbol? (car form))
          (expand-keyword-pair (car form) (cdr form))
          (cons (expand (car form))
                (expand-cdr (cdr form))))
      form))

(define (expand-keyword-pair keyword rest)
  (core:define xformer (core:hashq-ref %macro-table keyword #f))
  (if xformer
      (core:apply xformer rest)
      (cons keyword (expand-cdr rest))))

(define (expand-cdr form)
  (if (pair? form)
      (if (eq? (car form) *undefined*)
          (cons *undefined*
                (expand-cdr (cdr form)))
          (cons (expand (car form))
                (expand-cdr (cdr form))))
      (expand form)))

(define core:macro-expand expand)

;; Use the expander for 'eval'
(define (eval form)
  (core:apply (compile-top-level (expand form) (current-module)) '()))

;; System macros
(define (quote-expander form)
  (list core:quote form))

(define (lambda-expander formals . body)
  (cons core:lambda (cons formals (expand-cdr body))))

(define (if-expander test consequent . rest)
  (core:define alternate (if (pair? rest) (car rest) *unspecified*))
  (list core:if (expand test) (expand consequent) (expand alternate)))

(define (begin-expander . forms)
  (cons core:begin (expand-cdr forms)))

(define (set!-expander name exp)
  (list core:set! name (expand exp)))

(define (define-expander name+formals . body)
  (if (pair? name+formals)
      (cons core:define (cons name+formals (expand-cdr body)))
      (list core:define name+formals (expand (car body)))))

(define-macro! 'quote quote-expander)
(define-macro! 'lambda lambda-expander)
(define-macro! 'if if-expander)
(define-macro! 'begin begin-expander)
(define-macro! 'set! set!-expander)
(define-macro! 'define define-expander)

;; User macros
(define (wrap-user-macro proc)
  (lambda rest (expand (core:apply proc rest))))

(define (eval-and-define-macro! name form)
  (define-macro! name (wrap-user-macro (eval form)))
  *unspecified*)

(define (define-macro-expander name+formals . body)
  (if (pair? name+formals)
      (eval-and-define-macro! (car name+formals)
                              (cons 'lambda
                                    (cons (cdr name+formals) body)))
      (eval-and-define-macro! name+formals (car body))))

(define-macro! 'define-macro define-macro-expander)

;; The loader
(define (%read-and-eval on-last! form next-form)
  (if (null? next-form)
      (begin
        (on-last!)
        (eval form))
      (begin
        (eval form)
        (%read-and-eval on-last! next-form (reader-read)))))

(define (read-and-eval on-last! form)
  (if (null? form)
      (begin
        (on-last!)
        *unspecified*)
      (%read-and-eval on-last! form (reader-read))))

(define (load-from-current-input)
  (read-and-eval (lambda () *unspecified*) (reader-read)))

(define (load-from-port port)
  (core:define prev (set-current-input-port port))
  (read-and-eval (lambda ()
                   (set-current-input-port prev))
                 (reader-read)))

(define (check-close-port)
  (core:define v (module-variable (initial-module) 'close-port))
  (if v (if (eq? (variable-ref v) *undefined*) #f #t) #f))

(define has-close-port? (check-close-port))

(define (load-from-file filename)
  (core:define port (open-input-file filename))
  (core:define prev (set-current-input-port port))
  (read-and-eval (lambda ()
                   (set-current-input-port prev)
                   (if has-close-port?
                       (close-port port)
                       #t))
                 (reader-read)))

(define (primitive-load filename)
  (if (if (number? filename)
          (= filename 0)
          #f)
      (load-from-current-input)
      (if (port? filename)
          (load-from-port filename)
          (if (string? filename)
              (load-from-file filename)
              (error "primitive-load failed" filename)))))

;; Boot dispatch
(define (boot-file basename)
  (string-append %datadir "/module/mes/" basename))

(define boot-table
  (list (cons "boot-5.scm" (boot-file "boot-5.scm"))
        (cons "boot-0.scm" (boot-file "boot-0.scm"))
        (cons "boot-00.scm" (boot-file "boot-00.scm"))
        (cons "boot-01.scm" (boot-file "boot-01.scm"))
        (cons "boot-02.scm" (boot-file "boot-02.scm"))
        (cons "boot-03.scm" (boot-file "boot-03.scm"))
        (cons "boot.scm" 0)))

(define initial-mes-boot (getenv "MES_BOOT"))
(define mes-boot (if initial-mes-boot initial-mes-boot "boot-5.scm"))
(define mes-boot-handle (assoc mes-boot boot-table))
(define mes-boot-file
  (if mes-boot-handle
      (cdr mes-boot-handle)
      (error "boot: invalid MES_BOOT" mes-boot)))

(primitive-load mes-boot-file)
