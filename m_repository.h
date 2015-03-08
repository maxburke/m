#pragma once

/*
 * Repository methods
 */

struct m_object_t;

struct m_object_t *
m_repository_create(struct m_object_t *current_branch, const char *name, struct m_object_t *branch_list);

void
m_repository_finalize(struct m_object_t *repository);

