#include <assert.h>
#include <stdlib.h>

#include "m_base.h"
#include "m_commit.h"

struct m_commit_t
{
    struct m_object_t header;
    const char *log;
    struct m_object_t *previous_commit;
    struct m_object_t *root;
};

struct m_object_t *
m_commit_create(const char *log, struct m_object_t *previous_commit, struct m_object_t *root)
{
    struct m_commit_t *commit;

    assert(previous_commit->type == M_COMMIT || previous_commit->type == M_NULL);
    assert(root->type == M_TREE);

    commit = calloc(1, sizeof(struct m_commit_t));
    commit->header.type = M_COMMIT;
    commit->header.realized = 1;
    commit->log = log;
    commit->previous_commit = previous_commit;
    commit->root = root;

    return (struct m_object_t *)commit;
}

void
m_commit_finalize(struct m_object_t *object)
{
    struct m_commit_t *commit;
    struct m_cas_write_handle_t *write_handle;
    const char *log;
    struct m_object_t *previous_commit;
    struct m_object_t *root;

    assert(object != NULL);
    assert(object->type == M_COMMIT);

    if (object->finalized)
    {
        return;
    }

    commit = (struct m_commit_t *)object;
    log = commit->log;
    previous_commit = commit->previous_commit;
    root = commit->root;
    
    write_handle = m_serialize_begin(object);
    m_serialize_string(write_handle, log);
    m_serialize_object_ref(write_handle, previous_commit);
    m_serialize_object_ref(write_handle, root);
    m_serialize_end(write_handle, object);
}

