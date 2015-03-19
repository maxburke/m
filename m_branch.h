#pragma once

/*
 * Branch methods
 */

struct m_object_t;

struct m_object_t *
m_branch_create(const char *name, struct m_object_t *head_commit);

struct m_object_t *
m_branch_construct(struct m_sha1_hash_t hash, const void *data, size_t size);

void
m_branch_finalize(struct m_object_t *object);


