#pragma once

/*
 * Commit methods
 */

struct m_object_t;

/**
 * Commit constructor.
 *
 * \param[in] log Full commit log.
 * \param[in] previous_commit Reference to the previous commit.
 * \param[in] root Reference to the new source tree root for this commit.
 */
struct m_object_t *
m_commit_create(const char *log, struct m_object_t *previous_commit, struct m_object_t *root);

struct m_object_t *
m_commit_construct(const void *data, size_t size);

void
m_commit_finalize(struct m_object_t *commit);


