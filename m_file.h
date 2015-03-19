#ifndef M_FILE_H
#define M_FILE_H

#include "m_base.h"

int
m_make_meta_dir(void);

struct m_cas_write_handle_t;

struct m_cas_write_handle_t *
m_cas_write_open(void);

void
m_cas_write(struct m_cas_write_handle_t *, const void *, size_t);

struct m_sha1_hash_t
m_cas_write_close(struct m_cas_write_handle_t *);

struct m_cas_read_handle_t *
m_cas_read_open(struct m_sha1_hash_t hash, void **data, size_t *bytes);

void
m_cas_read_close(struct m_cas_read_handle_t *handle);

int
m_write(const char *filename, const void *data, size_t bytes);

int
m_read(const char *filename, void **data, size_t *bytes);

#endif

