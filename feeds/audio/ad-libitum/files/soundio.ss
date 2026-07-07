(library (soundio (1))
  (export open-default-out-stream
          start-out-stream
          stop-out-stream
          teardown-out-stream
          out-stream-status
          native-status
          reset-native-status!
          flush-events
          sample-rate
          channel-count
          usleep)
  (import (chezscheme))
  (include "soundio-ffi.ss")
  ;; <high-level-wrapper>
  ;; <build-bridge>
  ;; <bridge-paths>
  (define bridge-source-filename "bridge.c")
  (define bridge-library-filename "libbridge.so")
  (define scheme-headers-path (format "/usr/local/lib/csv9.5.5.5/~a" (machine-type)))
  ;; </bridge-paths>

  (define init-bridge
    (begin
      (unless (file-exists? bridge-library-filename)
        ;; <build-bridge>
        (case (machine-type)
          [(i3nt ti3nt a6nt ta6nt)
           (begin
             (error "init-bridge"
                    "don't know how to build for Windows, look at the source for template to adjust")
             (system (format "cl -c -DWIN32 ~a"
                             bridge-source-filename))
             (system (format "link -dll -out:~a ~a.obj"
                             bridge-library-filename
                             bridge-source-filename)))]
          [(i3osx ti3osx a6osx ta6osx tarm64osx)
           (system (format "cc -O3 -dynamiclib -Wl,-undefined -Wl,dynamic_lookup -I~a -lsoundio -o ~a ~a"
                           scheme-headers-path
                           bridge-library-filename
                           bridge-source-filename))]
          [(i3le ti3le a6le ta6le)
           (system (format "cc -O3 -fPIC -shared -Wl,-undefined -Wl,dynamic_lookup -I~a -lsoundio -o ~a ~a.c"
                           scheme-headers-path
                           bridge-library-filename
                           bridge-source-filename))]
          [else (error "init-bridge"
                       "don't know how to build bridge shared library on this machine-type"
                       (machine-type))])
        ;; </build-bridge>

        )
      (load-shared-object bridge-library-filename)))
  ;; </build-bridge>

  ;; <bridge-ffi>
  (define-foreign-procedure
    [bridge_outstream_attach_ring_buffer ((* SoundIoOutStream) (* SoundIoRingBuffer)) void]
    [bridge_soundio_wait_events ((* SoundIo)) void]
    [bridge_write_callback_calls () long]
    [bridge_frames_requested_min_total () long]
    [bridge_frames_requested_max_total () long]
    [bridge_frames_copied_from_ring () long]
    [bridge_zero_fill_frames () long]
    [bridge_last_fill_count () long]
    [bridge_last_frame_count_min () long]
    [bridge_last_frame_count_max () long]
    [bridge_last_requested_count () long]
    [bridge_last_read_count () long]
    [bridge_last_copied_count () long]
    [bridge_last_zero_count () long]
    [bridge_last_begin_write_count () long]
    [bridge_begin_write_zero_count () long]
    [bridge_reset_counters () void]
    [usleep (long #|seconds|# long #|microseconds|#) void])
  ;; </bridge-ffi>

  ;; <sound-out-record>
  (define-record-type sound-out
    (fields soundio
            stream
            ring-buffer
            (mutable write-callback)
            (mutable write-thread)
            (mutable event-thread)))
  ;; </sound-out-record>

  ;; <producer-counters>
  (define *producer-loop-count* 0)
  (define *producer-zero-free-sleeps* 0)
  (define *producer-write-batches* 0)
  (define *producer-frames-written* 0)
  (define *producer-last-free-count* 0)
  (define *producer-last-free-frames* 0)

  (define (reset-producer-counters!)
    (set! *producer-loop-count* 0)
    (set! *producer-zero-free-sleeps* 0)
    (set! *producer-write-batches* 0)
    (set! *producer-frames-written* 0)
    (set! *producer-last-free-count* 0)
    (set! *producer-last-free-frames* 0))
  ;; </producer-counters>

  ;; <open-default-out-stream>
  (define (open-default-out-stream write-callback)
    ;; <try-create-connect-sio>
    (let ([sio (soundio_create)])
      (when (ftype-pointer-null? sio)
        (error "soundio_create" "out of memory"))
      (let ([err (soundio_connect sio)])
        (when (not (zero? err))
          (error "soundio_connect" (soundio_strerror err)))
        (soundio_flush_events sio)
        ;; <try-create-device>
        (let ([idx (soundio_default_output_device_index sio)])
          (when (< idx 0)
            (error "soundio_default_output_device_index" "no output device found"))
          (let ([device (soundio_get_output_device sio idx)])
            (when (ftype-pointer-null? device)
              (error "soundio_get_output_device" "out of memory"))
            ;; <try-create-stream>
            (let ([out-stream (soundio_outstream_create device)])
              (when (ftype-pointer-null? out-stream)
                (error "soundio_outstream_create" "out of memory"))
              ;; <try-open-stream>
              (let ([err (soundio_outstream_open out-stream)])
                (when (not (zero? err))
                  (error "soundio_outstream_open" (soundio_strerror err)))
                (let ([err (ftype-ref SoundIoOutStream (layout_error) out-stream)])
                  (when (not (zero? err))
                    (error "soundio_outstream_open" (soundio_strerror err))))
                ;; <attach-buffer-to-stream>
                (let* ([frame-size (ftype-sizeof float)]
                       [channel-count (ftype-ref SoundIoOutStream (layout channel_count) out-stream)]
                       [sample-rate (ftype-ref SoundIoOutStream (sample_rate) out-stream)]
                       [latency (ftype-ref SoundIoOutStream (software_latency) out-stream)]
                       [buffer-size (exact (ceiling (* latency sample-rate)))] ; in samples
                       [buffer-capacity (* buffer-size frame-size channel-count)] ; in bytes
                       [ring-buffer (soundio_ring_buffer_create sio buffer-capacity)])
                  (when (ftype-pointer-null? ring-buffer)
                    (error "soundio_ring_buffer_create" "out of memory"))
                  (bridge_outstream_attach_ring_buffer out-stream ring-buffer)
                  ;; <make-sound-out>
                  (printf "Channels:\t~s\r\n" channel-count)
                  (printf "Sample rate:\t~s\r\n" sample-rate)
                  (printf "Latency:\t~s\r\n" latency)
                  (printf "Buffer:\t\t~s\r\n" buffer-size)
                  (make-sound-out sio out-stream ring-buffer write-callback #f #f)
                  ;; </make-sound-out>

                  )
                ;; </attach-buffer-to-stream>

                )
              ;; </try-open-stream>

              )
            ;; </try-create-stream>

            ))
        ;; </try-create-device>

        ))
    ;; </try-create-connect-sio>

    )
  ;; </open-default-out-stream>

  ;; <start-out-stream>
  (define (start-event-loop sound-out)
    (unless (sound-out-event-thread sound-out)
      (sound-out-event-thread-set! sound-out (get-thread-id))
      (fork-thread
       (lambda ()
         (let loop ()
           (when (sound-out-event-thread sound-out)
             (bridge_soundio_wait_events (sound-out-soundio sound-out))
             (loop)))))))

  (define (start-out-stream sound-out)
    (let* ([frame-size (ftype-sizeof float)]
           [out-stream (sound-out-stream sound-out)]
           [channel-count (ftype-ref SoundIoOutStream (layout channel_count) out-stream)]
           [sample-rate (ftype-ref SoundIoOutStream (sample_rate) out-stream)]
           [seconds-per-sample (inexact (/ sample-rate))]
           [ring-buffer (sound-out-ring-buffer sound-out)]
           [polling-microseconds 100]
           [sample-number 0])
      (start-event-loop sound-out)
      (sound-out-write-thread-set! sound-out (get-thread-id))
      (fork-thread
       (lambda ()
         (let loop ()
           (set! *producer-loop-count* (+ *producer-loop-count* 1))
           (let ([write-callback (sound-out-write-callback sound-out)])
             (when (sound-out-write-thread sound-out)
               (let ([free-count (soundio_ring_buffer_free_count ring-buffer)])
                 (set! *producer-last-free-count* free-count)
                 (if (zero? free-count)
                     (begin
                       (set! *producer-zero-free-sleeps*
                         (+ *producer-zero-free-sleeps* 1))
                       (usleep 0 polling-microseconds)
                       (loop))
                     (let ([free-frames (/ free-count frame-size channel-count)]
                           [write-ptr (ftype-pointer-address (soundio_ring_buffer_write_ptr ring-buffer))])
                       (set! *producer-write-batches*
                         (+ *producer-write-batches* 1))
                       (set! *producer-frames-written*
                         (+ *producer-frames-written* free-frames))
                       (set! *producer-last-free-frames* free-frames)
                       (do ([frame 0 (+ frame 1)])
                           ((= frame free-frames) 0)
                         (let* ([sample-number (+ sample-number frame)]
                                [time (fl* (fixnum->flonum sample-number) seconds-per-sample)])
                           (do ([channel 0 (+ channel 1)])
                               ((= channel channel-count) 0)
                             (foreign-set!
                              'float
                              write-ptr
                              (* (+ (* frame channel-count) channel) frame-size)
                              (write-callback time channel))
                             )))
                       (soundio_ring_buffer_advance_write_ptr ring-buffer free-count)
                       (set! sample-number (+ sample-number free-frames))
                       (loop))
                     )))))))
      (soundio_outstream_start out-stream)))
  ;; </start-out-stream>

  ;; <stop-out-stream>
  (define (stop-out-stream sound-out)
    (sound-out-write-thread-set! sound-out #f)
    (sound-out-event-thread-set! sound-out #f)
    (soundio_wakeup (sound-out-soundio sound-out))
    (soundio_outstream_pause (sound-out-stream sound-out) #t))
  ;; </stop-out-stream>

  ;; <teardown-out-stream>
  (define (teardown-out-stream sound-out)
    (let* ([stream (sound-out-stream sound-out)]
           [ring-buffer (sound-out-ring-buffer sound-out)]
           [device (ftype-ref SoundIoOutStream (device) stream)]
           [soundio (ftype-ref SoundIoDevice (soundio) device)])
      (soundio_outstream_destroy stream)
      (soundio_ring_buffer_destroy ring-buffer)
      (soundio_device_unref device)
      (soundio_destroy soundio)))
  ;; </teardown-out-stream>

  ;; <channel-count>
  (define (channel-count sound-out)
    (ftype-ref SoundIoOutStream
               (layout channel_count)
               (sound-out-stream sound-out)))
  ;; </channel-count>

  ;; <out-stream-status>
  (define (out-stream-status sound-out)
    (let* ([stream (sound-out-stream sound-out)]
           [ring-buffer (sound-out-ring-buffer sound-out)]
           [frame-size (ftype-sizeof float)]
           [channel-count (ftype-ref SoundIoOutStream (layout channel_count) stream)]
           [bytes-per-frame (* frame-size channel-count)]
           [fill-bytes (soundio_ring_buffer_fill_count ring-buffer)]
           [free-bytes (soundio_ring_buffer_free_count ring-buffer)]
           [capacity-bytes (soundio_ring_buffer_capacity ring-buffer)])
      `((write-thread-active . ,(if (sound-out-write-thread sound-out) #t #f))
        (event-thread-active . ,(if (sound-out-event-thread sound-out) #t #f))
        (fill-bytes . ,fill-bytes)
        (free-bytes . ,free-bytes)
        (capacity-bytes . ,capacity-bytes)
        (fill-frames . ,(/ fill-bytes bytes-per-frame))
        (free-frames . ,(/ free-bytes bytes-per-frame))
        (capacity-frames . ,(/ capacity-bytes bytes-per-frame))
        (channel-count . ,channel-count)
        (sample-rate . ,(ftype-ref SoundIoOutStream (sample_rate) stream))
        (producer-loop-count . ,*producer-loop-count*)
        (producer-zero-free-sleeps . ,*producer-zero-free-sleeps*)
        (producer-write-batches . ,*producer-write-batches*)
        (producer-frames-written . ,*producer-frames-written*)
        (producer-last-free-count . ,*producer-last-free-count*)
        (producer-last-free-frames . ,*producer-last-free-frames*))))
  ;; </out-stream-status>

  ;; <flush-events>
  (define (flush-events sound-out)
    (soundio_flush_events (sound-out-soundio sound-out)))
  ;; </flush-events>

  ;; <native-status>
  (define (native-status)
    `((write-callback-calls . ,(bridge_write_callback_calls))
      (frames-requested-min-total . ,(bridge_frames_requested_min_total))
      (frames-requested-max-total . ,(bridge_frames_requested_max_total))
      (frames-copied-from-ring . ,(bridge_frames_copied_from_ring))
      (zero-fill-frames . ,(bridge_zero_fill_frames))
      (last-fill-count . ,(bridge_last_fill_count))
      (last-frame-count-min . ,(bridge_last_frame_count_min))
      (last-frame-count-max . ,(bridge_last_frame_count_max))
      (last-requested-count . ,(bridge_last_requested_count))
      (last-read-count . ,(bridge_last_read_count))
      (last-copied-count . ,(bridge_last_copied_count))
      (last-zero-count . ,(bridge_last_zero_count))
      (last-begin-write-count . ,(bridge_last_begin_write_count))
      (begin-write-zero-count . ,(bridge_begin_write_zero_count))))

  (define (reset-native-status!)
    (bridge_reset_counters)
    (reset-producer-counters!))
  ;; </native-status>

  ;; <sample-rate>
  (define (sample-rate sound-out)
    (ftype-ref SoundIoOutStream
               (sample_rate)
               (sound-out-stream sound-out)))
  ;; </sample-rate>

  ;; </high-level-wrapper>

)
