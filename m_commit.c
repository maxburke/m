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

    return &commit->header;
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

struct m_object_t *
m_commit_construct(struct m_sha1_hash_t hash, const void *data, size_t size)
{
    size_t offset;
    struct m_commit_t *commit;

    offset = 0;
    commit = calloc(1, sizeof(struct m_commit_t));

    m_realize_header(data, &offset, size, &commit->header);
    commit->header.hash = hash;
    commit->header.finalized = 1;
    commit->log = m_realize_string(data, &offset, size);
    commit->previous_commit = m_realize_ref(data, &offset, size);
    commit->root = m_realize_ref(data, &offset, size);

    return &commit->header;
}

