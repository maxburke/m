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

m_ref_t
m_cas_write_close(struct m_cas_write_handle_t *);

#endif

