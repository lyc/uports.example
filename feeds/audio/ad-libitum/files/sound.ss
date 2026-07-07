(library (ad-libitum sound (1))
  (export start stop set-dsp! hush! *sample-rate* *channels* now
          last-dsp-error clear-dsp-error! status native-status
          reset-native-status! flush-events!)
  (import (chezscheme) (prefix (soundio) soundio:))
  ;; <sound>
  (define *time* 0.0)
  (define (now) *time*)
  
  (define (silence time channel) 0.0)
  (define *dsp* silence)

  (define *last-dsp-error* #f)
  (define *dsp-error-reported?* #f)

  (define (condition->string c)
    (with-output-to-string
      (lambda ()
        (display-condition c))))

  (define (last-dsp-error) *last-dsp-error*)

  (define (clear-dsp-error!)
    (set! *last-dsp-error* #f)
    (set! *dsp-error-reported?* #f))

  (define (set-dsp! f)
    (clear-dsp-error!)
    (set! *dsp* f))

  (define (hush!) (set-dsp! silence))
  
  (define (write-callback time channel)
    (set! *time* time)
    (guard (c
            [else
             (set! *last-dsp-error* (condition->string c))
             (unless *dsp-error-reported?*
               (set! *dsp-error-reported?* #t)
               (fprintf (current-error-port)
                        "ad-libitum DSP callback error at time=~s channel=~s:~%~a~%"
                        time
                        channel
                        *last-dsp-error*))
             0.0])
      (*dsp* time channel)))
  
  (define *sound-out* (soundio:open-default-out-stream write-callback))
  (define *sample-rate* (soundio:sample-rate *sound-out*))
  (define *channels* (soundio:channel-count *sound-out*))

  (define (status) (soundio:out-stream-status *sound-out*))

  (define (native-status) (soundio:native-status))

  (define (reset-native-status!) (soundio:reset-native-status!))

  (define (flush-events!) (soundio:flush-events *sound-out*))
  
  (define (start) (soundio:start-out-stream *sound-out*))
  (define (stop) (soundio:stop-out-stream *sound-out*))
  ;; </sound>

  )
