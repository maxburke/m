#pragma once

/*
 * Branch methods
 */

struct m_object_t;

struct m_object_t *
m_branch_create(const char *branch_name, struct m_object_t *head_commit);

void
m_branch_finalize(struct m_object_t *object);


