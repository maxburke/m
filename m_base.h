#ifndef M_BASE_H
#define M_BASE_H

#include "m_sha1.h"

#define M_ARRAY_COUNT(x) ((sizeof (x))/(sizeof (x)[0]))
#define M_UNUSED(x) ((void)x)
#define M_VERIFY(x, y) __pragma(warning(push)) __pragma(warning(disable:4127)) do { if (!(x)) { m_report_fatal_error(y); } } while (0) __pragma(warning(pop))

/**
 * Convenience method for testing string equality.
 *
 * \return Will return 0 if strings do not match, non-zero if they do.
 */
int
m_streq(const char *a, const char *b);

/**
 * Report a fatal error then exit the program with a non-zero exit code.
 */
void
m_report_fatal_error(const char *reason);

/**
 * Convenience method for platform C libraries that do not support strlcpy,
 * such as Windows. Unlike strncpy, strlcpy will guarantee that the output
 * is null terminated. If the returned value is less than the size parameter
 * then output has been truncated.
 *
 * \param[out] dest Buffer into which string contents are copied.
 * \param[in] src Source from which data is copied.
 * \param[in] size
 *
 * \return The c-string length of the output copied to the destination buffer.
 */
size_t
strlcpy(char *dest, const char *src, size_t size);


/*
 * Reference set methods
 */

struct m_reference_set_t;

/**
 * Reference set instance constructor.
 */
struct m_reference_set_t *
m_reference_set_create(void);

/**
 * Add a reference to the reference set. It is considered invalid behavior to 
 * add a duplicate to the reference set.
 *
 * \param[in] reference_set A valid reference set.
 * \param[in] item The item to add to the reference set.
 */
void
m_reference_set_add(struct m_reference_set_t *reference_set, struct m_ref_t item);

/**
 * Serializes a reference set working instance to disk, invalidating the runtime
 * object, and returning its data store key.
 *
 * \param[in] reference_set A valid reference set.
 *
 * \return Data store key usable as values for other structures.
 */
void
m_reference_set_finalize(struct m_reference_set_t *reference_set);


/*
 * Repository tree
 */

struct m_tree_t;

struct m_tree_t *
m_tree_create(const char *name, struct m_ref_t contents);


/*
 * Commit items
 */
struct m_commit_item_t;

/**
 * Commit item constructor.
 *
 * \param[in] name The name of the commit item.
 * \param[in] content Reference to the commit item's content.
 * \param[in] history Reference to either the previous commit or a resolve
 *                    object instance.
 */
struct m_commit_item_t *
m_commit_item_create(const char *name, struct m_ref_t content, struct m_ref_t history);


/*
 * Resolve methods
 */

struct m_resolve_t;

/**
 * Resolve object constructor.
 *
 * \param[in] base Reference to the commit item used for the merge base.
 * \param[in] history Reference to the commit item that is the merge target.
 */
struct m_resolve_t *
m_resolve_create(struct m_ref_t base, struct m_ref_t local);


/*
 * Commit methods
 */

struct m_commit_t;

/**
 * Commit constructor.
 *
 * \param[in] log Full commit log.
 * \param[in] previous_commit Reference to the previous commit.
 * \param[in] root Reference to the new source tree root for this commit.
 */
struct m_commit_t *
m_commit_create(const char *log, struct m_ref_t previous_commit, struct m_ref_t root);


/*
 * Branch methods
 */

struct m_branch_t;

struct m_branch_t *
m_branch_create(const char *, struct m_ref_t);


/*
 * Repository methods
 */

struct m_repository_t;

struct m_repository_t *
m_repository_create(struct m_ref_t, const char *, struct m_ref_t);

#endif

