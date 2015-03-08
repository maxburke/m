#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "m_base.h"
#include "m_reference_set.h"

struct m_reference_set_t
{
    struct m_object_t header;
    int capacity;
    int count;
    struct m_object_t **objects;
};

struct m_object_t *
m_reference_set_create(void)
{
    struct m_reference_set_t *reference_set;
    const int initial_capacity = 3;

    reference_set = calloc(1, sizeof(struct m_reference_set_t));
    reference_set->header.type = M_REFERENCE_SET;
    reference_set->header.realized = 1;
    reference_set->capacity = initial_capacity;
    reference_set->count = 0;
    reference_set->objects = calloc(initial_capacity, sizeof(struct m_object_t *));

    return (struct m_object_t *)reference_set;
}

void
m_reference_set_add(struct m_object_t *reference_set_obj, struct m_object_t *item)
{
    struct m_reference_set_t *reference_set;
    int count;
    int capacity;
    struct m_object_t **objects;

    assert(reference_set_obj != NULL);
    assert(reference_set_obj->type == M_REFERENCE_SET);
    assert(item != NULL);

    reference_set = (struct m_reference_set_t *)reference_set_obj;
    count = reference_set->count;
    capacity = reference_set->capacity;
    objects = reference_set->objects;

    if (capacity == count)
    {
        int new_capacity;

        new_capacity = (capacity * 3) / 2;
        objects = realloc(objects, new_capacity * sizeof(struct m_object_t *));
        reference_set->objects = objects;
    }

    objects[count] = item;
    reference_set->count = count + 1;
}

static int
serialized_object_comparer(const void *a, const void *b)
{
    const struct m_object_t *o1;
    const struct m_object_t *o2;

    o1 = a;
    o2 = b;

    return memcmp(&o1->hash, &o2->hash, sizeof(struct m_sha1_hash_t));
}

void
m_reference_set_finalize(struct m_object_t *object)
{
    struct m_reference_set_t *reference_set;
    struct m_cas_write_handle_t *write_handle;
    int count;
    struct m_object_t **objects;
    int i;

    assert(object != NULL);
    assert(object->type == M_REFERENCE_SET);

    if (object->finalized)
    {
        return;
    }

    reference_set = (struct m_reference_set_t *)object;
    count = reference_set->count;
    objects = reference_set->objects;

    write_handle = m_serialize_begin(object);
    m_serialize_i4(write_handle, count);

    for (i = 0; i < count; ++i)
    {
        m_object_finalize(objects[i]);
    }

    qsort(objects, count, sizeof(struct m_object_t), serialized_object_comparer);

    for (i = 0; i < count; ++i)
    {
        m_serialize_object_ref(write_handle, objects[i]);
    }

    m_serialize_end(write_handle, object);
}

