;;;; This file is part of GNU Dico
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
;;;; along with GNU Dico.  If not, see <http://www.gnu.org/licenses/>.

(define-module (glossary)
  #:use-module (guile-user)
  #:use-module (srfi srfi-13)
  #:use-module (ice-9 getopt-long)
  #:use-module (ice-9 rdelim)
  #:use-module (ice-9 regex))

(define glossary-descr #f)

(define (ndict-error err . rest)
  (with-output-to-port
      (current-error-port)
    (lambda ()
      (display err)
      (for-each
       display
       rest)
      (newline))))

(define empty-line-re (make-regexp "^[ \t]*$" regexp/extended))
(define comment-re (make-regexp "#.*$" regexp/extended))
(define entry-re (make-regexp "([a-z][a-z])[ \t]*(.*)" regexp/extended))

(define (prep-line line)
  (cond
   ((not (string? line))
    line)
   ((regexp-exec comment-re line) =>
    (lambda (match)
      (regexp-substitute #f match 'pre)))
   (else
    line)))

(define (empty-line? line)
  (regexp-exec empty-line-re line))

(define (skip-newlines)
  (call-with-current-continuation
   (lambda (return)
     (do ((line (prep-line (read-line)) (prep-line (read-line))))
	 ((eof-object? line) #f)
       (if (not (empty-line? line))
	   (return line))))))

(define (read-entry)
  (let ((x (skip-newlines)))
    (if x
	(let ((entry '()))
	  (do ((line x (prep-line (read-line))))
	      ((or (eof-object? line)
		   (empty-line? line)) entry)
	    (cond
	     ((regexp-exec entry-re line) =>
	      (lambda (match)
		(set! entry (cons
			     (cons
			      (match:substring match 1)
			      (match:substring match 2))
			     entry))))
	     (else
	      (ndict-error "bad entry: " line)))))
	'())))

(define (read-file file)
  (let ((lst (list)))
    (with-input-from-file
	file
      (lambda ()
	(do ((entry (read-entry) (read-entry)))
	    ((null? entry))
	  (set! lst (cons entry lst)))))
    lst))

;;

(define (mapcan fun list)
  (apply (lambda ( . slist)
           (let loop ((elt '())
                      (slist slist))
             (cond
              ((null? slist)
               (reverse elt))
              ((not (car slist))
               (loop elt (cdr slist)))
              (else
               (loop (cons (car slist) elt) (cdr slist))))))
         (map fun list)))

(define (make-dict-article ent)
  (cond
   ((null? (cdr ent))
    (car ent))
   (else
    (let ((str ""))
      (do ((lst ent (cdr lst))
	   (n 1 (1+ n)))
	  ((null? lst))
	(set! str (string-append str "\n"
				 (number->string n) ". "
				 (car lst))))
      str))))

(define (make-dict data from to)
  (let ((dict '()))
    (for-each
     (lambda (entry)
       (let ((src (mapcan (lambda (x)
			    (if (string=? (car x) from)
				(cdr x)
				#f))
			  entry))
	     (dst (mapcan (lambda (x)
			    (if (string=? (car x) to)
				(cdr x)
				#f))
			  entry)))
	 (for-each (lambda (x)
		     (if (not (null? dst))
			 (set! dict (cons
				     (cons x (make-dict-article dst))
				     dict))
			 #f))
		   src)))
     data)
    dict))
	      
;; Callbacks

(define glossary-dict '())

(define (open-module name . rest)
  (let ((from #f)
	(to #f))
    (for-each (lambda (arg)
		(let ((av (string-split arg #\=)))
		  (case (length av)
		    ((2) (cond
			  ((string=? (car av) "from")
			   (set! from (cadr av)))
			  ((string=? (car av) "to")
			   (set! to (cadr av)))
			  (else
			   (glossary-error "Unknown option " (car av)))))
		    (else
		     (glossary-error "Unknown option " (car av))))))
	      (cdr rest))
    (cond
     ((not from)
      (glossary-error "from option is not given")
      #f)
     ((not to)
      (glossary-error "to option is not given")
      #f)
     (else
      (let ((dict (make-dict glossary-dict from to)))
	(cond
	 ((= (length dict) 0)
	  (glossary-error "No dictionary for " from "-" to " pair")
	  #f)
	 (else
;	  (write dict)
	  (list from to name dict))))))))

(defmacro glossary:src (dbh)
  `(list-ref ,dbh 0))

(defmacro glossary:dst (dbh)
  `(list-ref ,dbh 1))

(defmacro glossary:name (dbh)
  `(list-ref ,dbh 2))

(defmacro glossary:db (dbh)
  `(list-ref ,dbh 3))

(define (descr dbh)
  glossary-descr)

(define (info dbh)
  (string-append
   (glossary:name dbh)
   "\n"
   (or (descr dbh) "")
   " Headwords: "
   (number->string (length (glossary:db dbh))) "."
   "\n"
   "Copyright © 2008 Sergey Poznyakoff\n\
\n\
Permission is granted to copy, distribute and/or modify this document\n\
under the terms of the GNU Free Documentation License, Version 1.2 or\n\
any later version published by the Free Software Foundation; with no\n\
Invariant Sections, no Front-Cover and Back-Cover Texts"))

(defmacro rh:define? (rh)
  `(car ,rh))

(defmacro rh:key (rh)
  `(list-ref ,rh 1))

(defmacro rh:result (rh)
  `(list-ref ,rh 2))

(define (match-exact dbh strat key)
  (mapcan (lambda (word)
	    (and (string-ci=? (car word) key)
		 (car word)))
	  (glossary:db dbh)))

(define (match-prefix dbh strat key)
  (mapcan (lambda (word)
	    (and (string-prefix-ci? key (car word))
		 (car word)))
	  (glossary:db dbh)))

(define (match-suffix dbh strat key)
  (mapcan (lambda (word)
	    (and (string-suffix-ci? key (car word))
		 (car word)))
	  (glossary:db dbh)))

(define (match-selector dbh strat key)
  (mapcan (lambda (word)
	    (and (dico-strat-select? strat (car word) key)
		 (car word)))
	  (glossary:db dbh)))

(define strategy-list
  (list (cons "exact"  match-exact)
	(cons "prefix"  match-prefix)
	(cons "suffix"  match-suffix)))

(define (match-default dbh strat word)
  (match-prefix dbh strat word))

(define (match-word dbh strat key)
  (let ((sp (assoc (dico-strat-name strat) strategy-list)))
    (let ((res (cond
		(sp
		 ((cdr sp) dbh strat key))
		((dico-strat-selector? strat)
		 (match-selector dbh strat key))
		(else
		 (match-default dbh strat key)))))
      (if res
	  (list #f key res)
	  #f))))

(define (define-word dbh key)
  (let ((res (mapcan
	      (lambda (word)
		(if (string-ci=? (car word) key)
		    (cdr word)
		    #f))
	      (glossary:db dbh))))
    (if (null? res)
	#f
	(list #t key res))))

(define (output rh n)
  (let ((res (rh:result rh)))
    (cond
     ((rh:define? rh)
      (display (rh:key rh))
      (newline)
      (display (list-ref (rh:result rh) n)))
     (else
      (display (list-ref (rh:result rh) n))))))

(define (result-count rh)
  (length (rh:result rh)))

(define-public (glossary-init name)
  (list (cons "open" open-module)
	(cons "descr" descr)
	(cons "info" info)
	(cons "define" define-word)
	(cons "match" match-word)
	(cons "output" output)
	(cons "result-count" result-count)))

;; Main
(let ((grammar `((info (value #t)))))
  (for-each 
   (lambda (x)
     (cond
      ((pair? x)
       (case (car x)
	 ((info)
	  (set! glossary-descr (cdr x)))
	 ('()
	  (let ((arglist (cdr x)))
	    (cond
	     ((null? (cdr arglist))
	      (set! glossary-dict (read-file (car arglist))))
	     (else
	      (ndict-error "Not enough arguments")
	      (exit 1)))))))))
   (getopt-long (command-line) grammar)))

(dico-register-strat "suffix" "Match word suffixes")
      

