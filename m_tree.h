#pragma once

struct m_object_t;

struct m_object_t *
m_tree_create(const char *name, struct m_object_t *contents);

struct m_object_t *
m_tree_construct(const void *data, size_t size);

void
m_tree_finalize(struct m_object_t *tree);

