#ifndef M_SHA1_H
#define M_SHA1_H

#include <stdint.h>

struct m_sha1_hash_t
{
    uint32_t h[5];
};

struct m_sha1_hash_context_t
{
    char buf[64];
    size_t buf_idx;
    struct m_sha1_hash_t hash;
};

void
m_sha1_hash_init(struct m_sha1_hash_context_t *context);

void
m_sha1_hash_update(struct m_sha1_hash_context_t *, const void *, size_t);

struct m_sha1_hash_t
m_sha1_hash_finalize(struct m_sha1_hash_context_t *);

struct m_sha1_hash_t
m_sha1_hash_buffer(const void *data, size_t length);

struct m_sha1_hash_t
m_sha1_hash_string(const char *string);

#endif

