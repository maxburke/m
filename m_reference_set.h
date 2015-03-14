#pragma once

struct m_object_t;

/**
 * Reference set instance constructor.
 */
struct m_object_t *
m_reference_set_create(void);

/**
 * Add a reference to the reference set. It is considered invalid behavior to 
 * add a duplicate to the reference set.
 *
 * \param[in] reference_set A valid reference set.
 * \param[in] item The item to add to the reference set.
 */
void
m_reference_set_add(struct m_object_t *reference_set, struct m_object_t *item);

struct m_object_t *
m_reference_set_construct(struct m_sha1_hash_t hash, const void *data, size_t size);

/**
 * Serializes a reference set working instance to disk, invalidating the runtime
 * object, and returning its data store key.
 *
 * \param[in] reference_set A valid reference set.
 *
 * \return Data store key usable as values for other structures.
 */
void
m_reference_set_finalize(struct m_object_t *object);


