#ifdef WIN32
#define EXPORT extern __declspec (dllexport)
#else
#define EXPORT extern
#endif

#include <sys/select.h>
#include <soundio/soundio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include "scheme.h"

static long bridge_write_callback_calls_counter = 0;
static long bridge_frames_requested_min_total_counter = 0;
static long bridge_frames_requested_max_total_counter = 0;
static long bridge_frames_copied_from_ring_counter = 0;
static long bridge_zero_fill_frames_counter = 0;
static long bridge_last_fill_count_counter = 0;
static long bridge_last_frame_count_min_counter = 0;
static long bridge_last_frame_count_max_counter = 0;
static long bridge_last_requested_count_counter = 0;
static long bridge_last_read_count_counter = 0;
static long bridge_last_copied_count_counter = 0;
static long bridge_last_zero_count_counter = 0;
static long bridge_last_begin_write_count_counter = 0;
static long bridge_begin_write_zero_count_counter = 0;

// <write_callback>
static void write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max) {
  struct SoundIoRingBuffer *ring_buffer = outstream->userdata;
  struct SoundIoChannelArea *areas;
  int frame_count;
  int frames_left;
  int err;

  char *read_ptr = soundio_ring_buffer_read_ptr(ring_buffer);
  int fill_bytes = soundio_ring_buffer_fill_count(ring_buffer);
  int fill_count = fill_bytes / outstream->bytes_per_frame;

  bridge_write_callback_calls_counter += 1;
  bridge_frames_requested_min_total_counter += frame_count_min;
  bridge_frames_requested_max_total_counter += frame_count_max;
  bridge_last_fill_count_counter = fill_count;
  bridge_last_frame_count_min_counter = frame_count_min;
  bridge_last_frame_count_max_counter = frame_count_max;

  // <copy-samples-from-buffer>
  int requested_count = fill_count >= frame_count_min
    ? (frame_count_max < fill_count ? frame_count_max : fill_count)
    : frame_count_min;
  int read_count = requested_count < fill_count ? requested_count : fill_count;
  int copied_count = 0;
  bridge_last_requested_count_counter = requested_count;
  bridge_last_read_count_counter = read_count;
  bridge_last_copied_count_counter = 0;
  bridge_last_zero_count_counter = 0;
  bridge_last_begin_write_count_counter = 0;
  frames_left = requested_count;
  
  while (frames_left > 0) {
    int frame_count = frames_left;
  
    // <begin-write>
    if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count))) {
      fprintf(stderr, "begin_write: %s\n", soundio_strerror(err));
      exit(1);
    }
    // </begin-write>

  
    bridge_last_begin_write_count_counter = frame_count;
    if (frame_count <= 0) {
      bridge_begin_write_zero_count_counter += 1;
      break;
    }
    int copied_this_chunk = 0;
    int zero_this_chunk = 0;
  
    for (int frame = 0; frame < frame_count; frame += 1) {
      for (int ch = 0; ch < outstream->layout.channel_count; ch += 1) {
        if (copied_count < read_count) {
          memcpy(areas[ch].ptr, read_ptr, outstream->bytes_per_sample);
          read_ptr += outstream->bytes_per_sample;
        } else {
          memset(areas[ch].ptr, 0, outstream->bytes_per_sample);
        }
        areas[ch].ptr += areas[ch].step;
      }
      if (copied_count < read_count) {
        copied_count += 1;
        copied_this_chunk += 1;
      } else {
        bridge_zero_fill_frames_counter += 1;
        zero_this_chunk += 1;
      }
    }
    bridge_frames_copied_from_ring_counter += copied_this_chunk;
    bridge_last_copied_count_counter += copied_this_chunk;
    bridge_last_zero_count_counter += zero_this_chunk;
  
    // <end-write>
    if ((err = soundio_outstream_end_write(outstream))) {
      fprintf(stderr, "end_write: %s\n", soundio_strerror(err));
      // REVIEW pthread_exit?
      exit(1);
    }
    // </end-write>

  
    frames_left -= frame_count;
  }
  // </copy-samples-from-buffer>


  soundio_ring_buffer_advance_read_ptr(ring_buffer, read_count * outstream->bytes_per_frame);
}
// </write_callback>

// <bridge_outstream_attach_ring_buffer>
EXPORT void bridge_outstream_attach_ring_buffer
(struct SoundIoOutStream *outstream, struct SoundIoRingBuffer *buffer) {
  outstream->format = SoundIoFormatFloat32NE;
  outstream->userdata = buffer;
  outstream->write_callback = write_callback;
}
// </bridge_outstream_attach_ring_buffer>

// <bridge_soundio_wait_events>
EXPORT void bridge_soundio_wait_events(struct SoundIo *soundio) {
  Sdeactivate_thread();
  soundio_wait_events(soundio);
  Sactivate_thread();
}
// </bridge_soundio_wait_events>

// <bridge_wait_fd_readable>
EXPORT long bridge_wait_fd_readable(int fd, long timeout_usec) {
  fd_set read_fds;
  struct timeval timeout;
  struct timeval *timeout_ptr = NULL;
  int ret;

  do {
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);

    if (timeout_usec >= 0) {
      timeout.tv_sec = timeout_usec / 1000000;
      timeout.tv_usec = timeout_usec % 1000000;
      timeout_ptr = &timeout;
    } else {
      timeout_ptr = NULL;
    }

    Sdeactivate_thread();
    ret = select(fd + 1, &read_fds, NULL, NULL, timeout_ptr);
    Sactivate_thread();
  } while (ret < 0 && errno == EINTR);

  return ret;
}
// </bridge_wait_fd_readable>

// <bridge_counters>
EXPORT long bridge_write_callback_calls(void) {
  return bridge_write_callback_calls_counter;
}

EXPORT long bridge_frames_requested_min_total(void) {
  return bridge_frames_requested_min_total_counter;
}

EXPORT long bridge_frames_requested_max_total(void) {
  return bridge_frames_requested_max_total_counter;
}

EXPORT long bridge_frames_copied_from_ring(void) {
  return bridge_frames_copied_from_ring_counter;
}

EXPORT long bridge_zero_fill_frames(void) {
  return bridge_zero_fill_frames_counter;
}

EXPORT long bridge_last_fill_count(void) {
  return bridge_last_fill_count_counter;
}

EXPORT long bridge_last_frame_count_min(void) {
  return bridge_last_frame_count_min_counter;
}

EXPORT long bridge_last_frame_count_max(void) {
  return bridge_last_frame_count_max_counter;
}

EXPORT long bridge_last_requested_count(void) {
  return bridge_last_requested_count_counter;
}

EXPORT long bridge_last_read_count(void) {
  return bridge_last_read_count_counter;
}

EXPORT long bridge_last_copied_count(void) {
  return bridge_last_copied_count_counter;
}

EXPORT long bridge_last_zero_count(void) {
  return bridge_last_zero_count_counter;
}

EXPORT long bridge_last_begin_write_count(void) {
  return bridge_last_begin_write_count_counter;
}

EXPORT long bridge_begin_write_zero_count(void) {
  return bridge_begin_write_zero_count_counter;
}

EXPORT void bridge_reset_counters(void) {
  bridge_write_callback_calls_counter = 0;
  bridge_frames_requested_min_total_counter = 0;
  bridge_frames_requested_max_total_counter = 0;
  bridge_frames_copied_from_ring_counter = 0;
  bridge_zero_fill_frames_counter = 0;
  bridge_last_fill_count_counter = 0;
  bridge_last_frame_count_min_counter = 0;
  bridge_last_frame_count_max_counter = 0;
  bridge_last_requested_count_counter = 0;
  bridge_last_read_count_counter = 0;
  bridge_last_copied_count_counter = 0;
  bridge_last_zero_count_counter = 0;
  bridge_last_begin_write_count_counter = 0;
  bridge_begin_write_zero_count_counter = 0;
}
// </bridge_counters>

// <usleep>
EXPORT void usleep (long seconds, long microseconds) {
  struct timeval timeout;
  timeout.tv_sec = seconds;
  timeout.tv_usec = microseconds;
  Sdeactivate_thread();
  select(0, NULL, NULL, NULL, &timeout);
  Sactivate_thread();
}
// </usleep>
