#pragma once

struct m_object_t;

struct m_object_t *
m_tree_create(const char *name, struct m_object_t *contents);

void
m_tree_finalize(struct m_object_t *tree);

