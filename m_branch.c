#include <assert.h>
#include <stdlib.h>

#include "m_base.h"
#include "m_branch.h"

struct m_branch_t
{
    struct m_object_t header;
    const char *name;
    struct m_object_t *head_commit;
};

struct m_object_t *
m_branch_create(const char *name, struct m_object_t *head_commit)
{
    struct m_branch_t *branch;

    assert(head_commit->type == M_COMMIT);

    branch = calloc(1, sizeof(struct m_branch_t));
    branch->header.type = M_BRANCH;
    branch->header.realized = 1;
    branch->name = name;
    branch->head_commit = head_commit;

    return &branch->header;
}

void
m_branch_finalize(struct m_object_t *object)
{
    struct m_branch_t *branch;
    struct m_cas_write_handle_t *write_handle;

    assert(object != NULL);
    assert(object->type == M_BRANCH);

    if (object->finalized)
    {
        return;
    }

    branch = (struct m_branch_t *)object;

    write_handle = m_serialize_begin(object);
    m_serialize_string(write_handle, branch->name);
    m_serialize_object_ref(write_handle, branch->head_commit);
    m_serialize_end(write_handle, object);
}

struct m_object_t *
m_branch_construct(struct m_sha1_hash_t hash, const void *data, size_t size)
{
    size_t offset;
    struct m_branch_t *branch;

    offset = 0;
    branch = calloc(1, sizeof(struct m_branch_t));

    m_realize_header(data, &offset, size, &branch->header);
    branch->header.hash = hash;
    branch->header.finalized = 1;
    branch->name = m_realize_string(data, &offset, size);
    branch->head_commit = m_realize_ref(data, &offset, size);

    return &branch->header;
}

