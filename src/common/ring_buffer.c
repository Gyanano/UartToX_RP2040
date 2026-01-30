/**
 * 环形缓冲区实现
 */

#include "../include/uart_to_x.h"

typedef struct {
    uint8_t *buffer;
    size_t size;
    volatile size_t head;
    volatile size_t tail;
} ring_buffer_t;

void ring_buffer_init(ring_buffer_t *rb, uint8_t *buffer, size_t size) {
    rb->buffer = buffer;
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
}

bool ring_buffer_is_empty(ring_buffer_t *rb) {
    return rb->head == rb->tail;
}

bool ring_buffer_is_full(ring_buffer_t *rb) {
    return ((rb->head + 1) % rb->size) == rb->tail;
}

size_t ring_buffer_available(ring_buffer_t *rb) {
    if (rb->head >= rb->tail) {
        return rb->head - rb->tail;
    }
    return rb->size - rb->tail + rb->head;
}

size_t ring_buffer_free(ring_buffer_t *rb) {
    return rb->size - ring_buffer_available(rb) - 1;
}

bool ring_buffer_put(ring_buffer_t *rb, uint8_t data) {
    if (ring_buffer_is_full(rb)) {
        return false;
    }
    rb->buffer[rb->head] = data;
    rb->head = (rb->head + 1) % rb->size;
    return true;
}

bool ring_buffer_get(ring_buffer_t *rb, uint8_t *data) {
    if (ring_buffer_is_empty(rb)) {
        return false;
    }
    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % rb->size;
    return true;
}

size_t ring_buffer_write(ring_buffer_t *rb, const uint8_t *data, size_t len) {
    size_t written = 0;
    while (written < len && !ring_buffer_is_full(rb)) {
        ring_buffer_put(rb, data[written++]);
    }
    return written;
}

size_t ring_buffer_read(ring_buffer_t *rb, uint8_t *data, size_t len) {
    size_t read = 0;
    while (read < len && !ring_buffer_is_empty(rb)) {
        ring_buffer_get(rb, &data[read++]);
    }
    return read;
}

void ring_buffer_clear(ring_buffer_t *rb) {
    rb->head = 0;
    rb->tail = 0;
}
