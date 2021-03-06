#include <assert.h>
#include <stdlib.h>

#include "m_base.h"
#include "m_tree.h"

struct m_tree_t
{
    struct m_object_t header;
    const char *name;
    struct m_object_t *contents;
};

struct m_object_t *
m_tree_create(const char *name, struct m_object_t *contents)
{
    struct m_tree_t *tree;

    assert(name != NULL);
    assert(contents->type == M_REFERENCE_SET);

    tree = calloc(1, sizeof(struct m_tree_t));
    tree->header.type = M_TREE;
    tree->header.realized = 1;
    tree->name = name;
    tree->contents = contents;

    return &tree->header;
}

void
m_tree_finalize(struct m_object_t *object)
{
    struct m_tree_t *tree;
    struct m_cas_write_handle_t *write_handle;
    const char *name;
    struct m_object_t *contents;

    assert(object != NULL);
    assert(object->type == M_TREE);

    if (object->finalized)
    {
        return;
    }

    tree = (struct m_tree_t *)object;
    name = tree->name;
    contents = tree->contents;

    write_handle = m_serialize_begin(object);
    m_serialize_string(write_handle, name);
    m_serialize_object_ref(write_handle, contents);
    m_serialize_end(write_handle, object);
}

struct m_object_t *
m_tree_construct(struct m_sha1_hash_t hash, const void *data, size_t size)
{
    size_t offset;
    struct m_tree_t *tree;

    offset = 0;
    tree = calloc(1, sizeof(struct m_tree_t));

    m_realize_header(data, &offset, size, &tree->header);
    tree->header.hash = hash;
    tree->header.finalized = 1;
    tree->name = m_realize_string(data, &offset, size);
    tree->contents = m_realize_ref(data, &offset, size);

    return &tree->header;
}

