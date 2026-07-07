(library (ad-libitum repl (1))
  (export start-repl-server process-pending!)
  (import (chezscheme)
          (prefix (bsd-sockets) sock:))

  (define listen-address "127.0.0.1:37146")
  (define polling-nanoseconds 5000000)
  (define polling-duration (make-time 'time-duration polling-nanoseconds 0))
  (define max-chunk-length 4096)
  (define socket-wait-timeout-usec 500000)
  (define code-tx
    (make-transcoder
     (utf-8-codec)
     (eol-style lf)
     (error-handling-mode replace)))
  (define bridge_wait_fd_readable
    (foreign-procedure "bridge_wait_fd_readable" (int long) long))

  (define-record-type repl-request
    (fields expr
            (mutable output)
            (mutable done?)))

  (define request-mutex (make-mutex))
  (define request-queue '())

  (define (condition->string c)
    (with-output-to-string
      (lambda ()
        (display-condition c))))

  (define (open-socket)
    (let ([socket (sock:create-socket
                   sock:socket-domain/internet
                   sock:socket-type/stream
                   sock:socket-protocol/auto)])
      (sock:set-socket-nonblocking! socket #t)
      (sock:bind-socket socket (sock:string->internet-address listen-address))
      (sock:listen-socket socket 1024)
      socket))

  (define (wait-readable socket)
    (let loop ()
      (let ([result (bridge_wait_fd_readable
                     (sock:socket-fd socket)
                     socket-wait-timeout-usec)])
        (cond
         [(positive? result) #t]
         [(zero? result) (loop)]
         [else (error 'wait-readable "select failed" result)]))))

  (define (send-string socket address s)
    (sock:send-to-socket socket (string->utf8 s) address))

  (define (send-prompt socket address)
    (send-string socket address "> "))

  (define (eval-expression expr)
    (let ([ok? #t]
          [result #f])
      (let ([output
             (with-output-to-string
               (lambda ()
                 (set! result
                   (guard (c
                           [else
                            (set! ok? #f)
                            (display-condition c)
                            #f])
                     (eval expr)))))])
        (if ok?
            (string-append output (format "~s\n" result))
            (string-append output "\n")))))

  (define (enqueue-expression expr)
    (let ([request (make-repl-request expr "" #f)])
      (with-mutex request-mutex
        (set! request-queue (append request-queue (list request))))
      request))

  (define (take-pending-requests)
    (with-mutex request-mutex
      (let ([requests request-queue])
        (set! request-queue '())
        requests)))

  (define (process-request! request)
    (repl-request-output-set!
     request
     (eval-expression (repl-request-expr request)))
    (repl-request-done?-set! request #t))

  (define (process-pending!)
    (for-each process-request! (take-pending-requests)))

  (define (wait-request request)
    (let loop ()
      (if (repl-request-done? request)
          (repl-request-output request)
          (begin
            (sleep polling-duration)
            (loop)))))

  (define (line-end-index s)
    (let ([n (string-length s)])
      (let loop ([i 0])
        (cond
         [(= i n) #f]
         [(or (char=? (string-ref s i) #\newline)
              (char=? (string-ref s i) #\return))
          i]
         [else (loop (+ i 1))]))))

  (define (drop-line-ending s i)
    (let ([n (string-length s)])
      (let ([j (if (and (< (+ i 1) n)
                        (char=? (string-ref s i) #\return)
                        (char=? (string-ref s (+ i 1)) #\newline))
                   (+ i 2)
                   (+ i 1))])
        (substring s j n))))

  (define (process-line socket address line)
    (guard (c
            [else
             (send-string socket address
                          (string-append (condition->string c) "\n"))])
      (unless (= (string-length line) 0)
        (let ([expr (call-with-port (open-string-input-port line) read)])
          (send-string socket address
                       (wait-request (enqueue-expression expr)))))))

  (define (process-buffer socket address buffer)
    (let loop ([buf buffer])
      (let ([i (line-end-index buf)])
        (if i
            (begin
              (process-line socket address (substring buf 0 i))
              (send-prompt socket address)
              (loop (drop-line-ending buf i)))
            buf))))

  (define (close-client socket)
    (guard (c [else #f])
      (sock:close-socket socket)))

  (define (serve-client socket address)
    (printf "New REPL @ ~s\r\n" (sock:internet-address->string address))
    (sock:set-socket-nonblocking! socket #t)
    (dynamic-wind
      (lambda () #f)
      (lambda ()
        (guard (c
                [else
                 (printf "REPL client closed @ ~s: ~a\r\n"
                         (sock:internet-address->string address)
                         (condition->string c))])
          (send-prompt socket address)
          (let loop ([buffer ""])
            (wait-readable socket)
            (let-values ([(request peer)
                          (sock:receive-from-socket socket max-chunk-length)])
              (cond
               [(and request (positive? (bytevector-length request)))
                (let* ([chunk (bytevector->string request code-tx)]
                       [buffer (process-buffer socket address
                                               (string-append buffer chunk))])
                  (loop buffer))]
               [(and request (zero? (bytevector-length request)))
                (printf "REPL client EOF @ ~s\r\n"
                        (sock:internet-address->string address))]
               [(sock:socket-condition? peer)
                (loop buffer)]
               [else
                (loop buffer)])))))
      (lambda ()
        (close-client socket))))

  (define (accept-connections repl-server-socket)
    (fork-thread
      (lambda ()
       (let loop ()
         (wait-readable repl-server-socket)
         (let-values ([(socket address) (sock:accept-socket repl-server-socket)])
           (when socket
             (serve-client socket address)))
         (loop)))))

  (define (start-repl-server)
    (accept-connections (open-socket))))
