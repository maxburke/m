#pragma once

/*
 * Repository methods
 */

struct m_object_t;

struct m_object_t *
m_repository_create(struct m_object_t *current_branch, const char *name, struct m_object_t *branch_list);

struct m_object_t *
m_repository_construct(struct m_sha1_hash_t hash, const void *data, size_t size);

void
m_repository_finalize(struct m_object_t *repository);

