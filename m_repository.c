#include <assert.h>
#include <stdlib.h>

#include "m_base.h"
#include "m_repository.h"

struct m_repository_t
{
    struct m_object_t header;
    struct m_object_t *current_branch;
    const char *name;
    struct m_object_t *branch_list;
};

struct m_object_t *
m_repository_create(struct m_object_t *current_branch, const char *name, struct m_object_t *branch_list)
{
    struct m_repository_t *repository;

    assert(current_branch->type == M_BRANCH);
    assert(branch_list->type == M_REFERENCE_SET);

    repository = calloc(1, sizeof(struct m_repository_t));
    repository->header.type = M_REPOSITORY;
    repository->header.realized = 1;
    repository->current_branch = current_branch;
    repository->name = name;
    repository->branch_list = branch_list;

    return &repository->header;
}

void
m_repository_finalize(struct m_object_t *object)
{
    struct m_repository_t *repository;
    struct m_cas_write_handle_t *write_handle;
    struct m_object_t *current_branch;
    const char *name;
    struct m_object_t *branch_list;

    assert(object != NULL);
    assert(object->type == M_REPOSITORY);

    if (object->finalized)
    {
        return;
    }

    repository = (struct m_repository_t *)object;
    current_branch = repository->current_branch;
    name = repository->name;
    branch_list = repository->branch_list;

    write_handle = m_serialize_begin(object);
    m_serialize_object_ref(write_handle, current_branch);
    m_serialize_string(write_handle, name);
    m_serialize_object_ref(write_handle, branch_list);
    m_serialize_end(write_handle, object);
}

