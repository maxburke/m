#include <assert.h>
#include <stdlib.h>

#include "m_base.h"
#include "m_branch.h"

struct m_branch_t
{
    struct m_object_t header;
    const char *branch_name;
    struct m_object_t *head_commit;
};

struct m_object_t *
m_branch_create(const char *branch_name, struct m_object_t *head_commit)
{
    struct m_branch_t *branch;

    assert(head_commit->type == M_COMMIT);

    branch = calloc(1, sizeof(struct m_branch_t));
    branch->header.type = M_BRANCH;
    branch->header.realized = 1;
    branch->branch_name = branch_name;
    branch->head_commit = head_commit;

    return (struct m_object_t *)branch;
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
    m_serialize_string(write_handle, branch->branch_name);
    m_serialize_object_ref(write_handle, branch->head_commit);
    m_serialize_end(write_handle, object);
}

