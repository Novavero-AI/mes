;;; -*-scheme-*-

;;; GNU Mes --- Maxwell Equations of Software
;;; Copyright © 2017,2018 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
;;; Copyright © 2023 Ekaitz Zarraga <ekaitz@elenq.tech>
;;; Copyright © 2026 Timothy Sample <samplet@ngyro.com>
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

;;; Code:

(define-module (mes guile)
  #:export (<cell:char>
            <cell:keyword>
            <cell:number>
            <cell:pair>
            <cell:string>
            <cell:symbol>
            <cell:vector>

            %arch
            %compiler
            %program

            append2
            core:apply
            core:car
            core:display
            core:display-error
            core:display-port
            core:exit
            core:macro-expand
            core:write
            core:write-error
            core:write-port
            core:type
            initial-module
            equal2?
            keyword->string
            pmatch-car
            pmatch-cdr
            search-path-quote
            search-path-split))

(cond-expand
 (guile-2)
 (guile
  (define %host-type (string-append (utsname:machine (uname)) "linux-gnu")))
 (else))

(cond-expand
 (guile
  (define pmatch-car car)
  (define pmatch-cdr cdr)
  (define core:exit exit)
  (define core:display display)
  (define core:display-port display)
  (define (core:display-error o) (display o (current-error-port)))
  (define core:write write)
  (define (core:write-error o) (write o (current-error-port)))
  (define core:write-port write)
  (define core:macro-expand identity)
  (define core:apply apply)
  (define (core:car f a . m) (apply f a))
  (define append2 append)
  (define equal2? equal?)

  ;; A search path splits on colons, except inside a pair of double quotes:
  ;; "C:\windows\absolute\path" is one entry, drive-letter colon and all.
  ;; Mes defines these while booting; Guile needs its own copy.
  (define (search-path-split path)
    (let loop ((lst (string->list path)) (field '()) (result '()) (quoted? #f))
      (cond ((null? lst)
             (append result (list (list->string (reverse field)))))
            ((eq? (car lst) #\")
             (loop (cdr lst) field result (not quoted?)))
            ((and (not quoted?) (eq? (car lst) #\:))
             (loop (cdr lst) '() (append result (list (list->string (reverse field)))) #f))
            (else
             (loop (cdr lst) (cons (car lst) field) result quoted?)))))

  (define (search-path-quote path)
    (if (memq #\: (string->list path)) (string-append "\"" path "\"") path))

  (define guile:keyword? keyword?)
  (define guile:number? number?)
  (define guile:pair? pair?)
  (define guile:string? string?)
  (define guile:symbol? symbol?)

  (define <cell:char> 0)
  (define <cell:keyword> 4)
  (define <cell:number> 6)
  (define <cell:pair> 7)
  (define <cell:string> 10)
  (define <cell:symbol> 11)
  (define <cell:vector> 15)

  (define %arch (car (string-split %host-type #\-)))
  (define %compiler "gnuc")
  (define %program #f)

  (define keyword->string (compose symbol->string keyword->symbol))

  (define (core:type x)
    (cond ((guile:keyword? x) <cell:keyword>)
          ((guile:number? x) <cell:number>)
          ((guile:pair? x) <cell:pair>)
          ((guile:string? x) <cell:string>)
          ((guile:symbol? x) <cell:symbol>)))
  (define (core:car x)
    (cond ((guile:string? x) (string->list x))))

  (define initial-module
    (let ((m (current-module)))
      (lambda () m))))
 (mes))

(cond-expand
 (guile-2.2)
 (guile-2)
 (guile
  (use-modules (ice-9 syncase))
  (define (compose proc . rest)
  (if (null? rest) proc
      (lambda args
        (proc (apply (apply compose rest) args)))))
  (export compose))
 (mes))
