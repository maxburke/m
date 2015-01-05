#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "m_base.h"
#include "m_file.h"

#ifndef MAX
#define MAX(x, y) ((x > y) ? (x) : (y))
#endif

enum m_type_tag_t
{
    M_REPOSITORY    = 1,
    M_COMMIT_ITEM   = 2,
    M_TREE          = 3,
    M_COMMIT        = 4,
    M_RESOLVE       = 5,
    M_BRANCH        = 6,
    M_MASTER_COMMIT = 7,

    /*
     * These type tags are unserialized.
     */

    M_REFERENCE_SET,
    M_CONTENT
};


struct m_reference_set_t
{
    int tag;

    int num_items;
    int capacity;
    m_ref_t *items;
};


struct m_repository_t
{
    int tag;

    m_ref_t active_branch;
    struct m_reference_set_t *branch_list;
    const char *name;
};


struct m_commit_t
{
    int tag;

    m_ref_t previous_commit;
    m_ref_t root;
    const char *log;
};


struct m_commit_item_t
{
    int tag;

    uint32_t flags;
    m_ref_t content;
    m_ref_t history;
    const char *name;
};


struct m_resolve_t
{
    int tag;

    m_ref_t base;
    m_ref_t local;
};


struct m_tree_t
{
    int tag;

    m_ref_t tree_contents;
    const char *name;
};


struct m_branch_t
{
    int tag;

    m_ref_t head;
    const char *name;
};


int
m_streq(const char *a, const char *b)
{
    return strcmp(a, b) == 0;
}


void
m_report_fatal_error(const char *reason)
{
    fputs(reason, stderr);
    exit(-1);
}


#ifdef _MSC_VER
    size_t
    strlcpy(char *dest, const char *src, size_t size)
    {
        size_t i;

        for (i = 0; i != (size - 1); ++i)
        {
            char c;

            c = dest[i] = src[i];

            if (c == 0)
            {
                break;
            }
        }

        dest[size - 1] = 0;

        return i;
    }
#endif


struct m_reference_set_t *
m_reference_set_create(void)
{
    const int initial_capacity = 1;

    struct m_reference_set_t *reference_set = calloc(1, sizeof(struct m_reference_set_t));

    reference_set->tag = M_REFERENCE_SET;
    reference_set->num_items = 0;
    reference_set->capacity = initial_capacity;
    reference_set->items = calloc(initial_capacity, sizeof(m_ref_t));

    return reference_set;
}


static int
m_reference_set_find(struct m_reference_set_t *reference_set, m_ref_t item)
{
    int i;
    int e;

    for (i = 0, e = reference_set->num_items; i != e; ++i)
    {
        int ii;
        uint32_t w;
        m_ref_t *ptr = reference_set->items + i;

        for (ii = 0; ii < 5; ++ii)
        {
            w = ptr->h[ii];

            if (w != item.h[ii])
            {
                goto next;
            }
        }

        return i;

    next:;
    }

    return -1;
}


void
m_reference_set_add(struct m_reference_set_t *reference_set, m_ref_t item)
{
    int capacity;
    int num_items;

    assert(reference_set != NULL);
    assert(m_reference_set_find(reference_set, item) < 0);

    capacity = reference_set->capacity;
    num_items = reference_set->num_items;

    if (num_items == capacity)
    {
        capacity = MAX(capacity + (capacity >> 1), capacity + 1);

        m_ref_t *old_items = reference_set->items;
        m_ref_t *new_items = calloc(capacity, sizeof(m_ref_t));

        memcpy(new_items, old_items, sizeof(m_ref_t) * num_items);

        reference_set->capacity = capacity;
        reference_set->items = new_items;
    }

    ++num_items;
    reference_set->items[num_items] = item;
    reference_set->num_items = num_items;
}


static int
m_hash_comparer(const void *a, const void *b)
{
    return memcmp(a, b, sizeof(m_ref_t));
}


static void
m_cas_write_length_header(struct m_cas_write_handle_t *handle, uint32_t length)
{
    if (length < 0x80)
    {
        unsigned char short_length = (unsigned char)length;

        m_cas_write(handle, &short_length, 1);
    }
    else if (length >= 0x80 && length < 0x10000)
    {
        unsigned char length_header[3];

        length_header[0] = 0x80;
        length_header[1] = (unsigned char)length;
        length_header[2] = (unsigned char)((length >> 8) & 0xFF);
        
        m_cas_write(handle, length_header, 3);
    }
    else
    {
        unsigned char length_header[5];

        length_header[0] = 0xFF;
        length_header[1] = (unsigned char)length;
        length_header[2] = (unsigned char)((length >> 8) & 0xFF);
        length_header[3] = (unsigned char)((length >> 16) & 0xFF);
        length_header[4] = (unsigned char)((length >> 24) & 0xFF);
        
        m_cas_write(handle, length_header, 5);
    }
}


static void
m_cas_write_string(struct m_cas_write_handle_t *handle, const char *string)
{
    size_t length;

    length = strlen(string);
    assert((uint64_t)length <= UINT64_C(0xFFFFFFFF));

    m_cas_write_length_header(handle, (uint32_t)length);
    m_cas_write(handle, string, length);
}


static void
m_cas_write_ref(struct m_cas_write_handle_t *handle, m_ref_t ref)
{
    m_cas_write(handle, &ref, sizeof ref);
}


m_ref_t
m_reference_set_finalize(struct m_reference_set_t *reference_set)
{
    int num_items;
    m_ref_t *items;
    m_ref_t rv;
    struct m_cas_write_handle_t *handle;

    items = reference_set->items;
    num_items = reference_set->num_items;

    assert((uint64_t)num_items <= UINT64_C(0xFFFFFFFF));

    qsort(items, num_items, sizeof(m_ref_t), m_hash_comparer);

    handle = m_cas_write_open();

    m_cas_write_length_header(handle, (uint32_t)num_items);
    m_cas_write(handle, items, num_items * sizeof(m_ref_t));
   
    rv = m_cas_write_close(handle);

    free(reference_set);

    return rv;
}


struct m_commit_t *
m_commit_create(const char *log, m_ref_t previous_commit, m_ref_t root)
{
    struct m_commit_t *commit;

    commit = calloc(1, sizeof(struct m_commit_t));

    commit->tag = M_COMMIT;
    commit->log = log;
    commit->previous_commit = previous_commit;
    commit->root = root;

    return commit;
}


m_ref_t
m_commit_finalize(struct m_commit_t *commit)
{
    int32_t tag;
    m_ref_t rv;
    struct m_cas_write_handle_t *handle;

    tag = M_COMMIT;
    handle = m_cas_write_open();

    m_cas_write(handle, &tag, sizeof tag);
    m_cas_write_ref(handle, commit->previous_commit);
    m_cas_write_ref(handle, commit->root);
    m_cas_write_string(handle, commit->log);

    rv = m_cas_write_close(handle);

    free(commit);

    return rv;
}


struct m_commit_item_t *
m_commit_item_create(const char *name, m_ref_t content, m_ref_t history)
{
    struct m_commit_item_t *commit_item;

    commit_item = calloc(1, sizeof(struct m_commit_item_t));

    commit_item->tag = M_COMMIT_ITEM;
    commit_item->content = content;
    commit_item->history = history;

    return commit_item;
}


m_ref_t
m_commit_item_finalize(struct m_commit_item_t *commit_item)
{
    int32_t tag;
    m_ref_t rv;
    struct m_cas_write_handle_t *handle;

    tag = M_COMMIT_ITEM;
    handle = m_cas_write_open();

    m_cas_write(handle, &tag, sizeof tag);
    m_cas_write(handle, &commit_item->flags, sizeof commit_item->flags);
    m_cas_write_ref(handle, commit_item->content);
    m_cas_write_ref(handle, commit_item->history);
    m_cas_write_string(handle, commit_item->name);

    rv = m_cas_write_close(handle);

    free(commit);

    return rv;
}


struct m_resolve_t *
m_resolve_create(m_ref_t base, m_ref_t local)
{
    struct m_resolve_t *commit_item;

    resolve_object = calloc(1, sizeof(struct m_resolve_t));

    resolve_object->tag = M_RESOLVE;
    resolve_object->content = base;
    resolve_object->history = local;

    return resolve_object;
}


m_ref_t
m_resolve_finalize(struct m_resolve_t *resolve_object)
{
    int32_t tag;
    m_ref_t rv;
    struct m_cas_write_handle_t *handle;

    tag = M_RESOLVE;
    handle = m_cas_write_open();

    m_cas_write(handle, &tag, sizeof tag);
    m_cas_write_ref(handle, resolve_object->base);
    m_cas_write_ref(handle, resolve_object->local);

    rv = m_cas_write_close(handle);

    free(commit);

    return rv;
}


struct m_branch_t *
m_branch_create(const char *branch_name, m_ref_t head_ref)
{
    M_UNUSED(branch_name);
    M_UNUSED(head_ref);

    return NULL;
}


m_ref_t
m_branch_finalize(struct m_branch_t *branch)
{
    m_ref_t rv = { 0 };

    M_UNUSED(branch);
    
    return rv;
}


struct m_repository_t *
m_repository_create(m_ref_t branch_ref, const char *repository_name, m_ref_t branch_list_ref)
{
    M_UNUSED(branch_ref);
    M_UNUSED(repository_name);
    M_UNUSED(branch_list_ref);

    return NULL;
}


m_ref_t
m_repository_finalize(struct m_repository_t *repository)
{
    m_ref_t rv = { 0 };

    M_UNUSED(repository);

    return rv;
}

