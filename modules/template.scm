;;;; This file is part of GNU Dico.
;;;; Copyright (C) 2008, 2010, 2012 Sergey Poznyakoff
;;;; 
;;;; GNU Dico is free software; you can redistribute it and/or modify
;;;; it under the terms of the GNU General Public License as published by
;;;; the Free Software Foundation; either version 3, or (at your option)
;;;; any later version.
;;;; 
;;;; GNU Dico is distributed in the hope that it will be useful,
;;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;;; GNU General Public License for more details.
;;;; 
;;;; You should have received a copy of the GNU General Public License
;;;; along with GNU Dico.  If not, see <http://www.gnu.org/licenses/>. */

(define-module (dico)
  #:use-module (guile-user))

(define (open-module name . rest)
  #f)

(define (close-module dbh)
  #f)

(define (descr dbh)
  #f)

(define (info dbh)
  #f)

(define (lang dbh)
  (cons (list) (list)))

(define (define-word dbh word)
  #f)

(define (match-word dbh strat key)
  #f)

(define (output rh n)
  #f)

(define (result-count rh)
  #f)

(define (result-headers rh hdr)
  (list))

(define-public (dico-init arg)
  (list (cons "open" open-module)
        (cons "close" close-module)
        (cons "descr" descr)
        (cons "info" info)
	(cons "lang" lang)
        (cons "define" define-word)
        (cons "match" match-word)
        (cons "output" output)
        (cons "result-count" result-count)
	(cons "result-headers" result-headers)))
