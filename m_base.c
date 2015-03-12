#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "m_base.h"
#include "m_branch.h"
#include "m_commit.h"
#include "m_file.h"
#include "m_reference_set.h"
#include "m_repository.h"
#include "m_tree.h"

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

static void
m_serialize_header(struct m_cas_write_handle_t *write_handle, struct m_object_t *object)
{
    /*
     * NOTE: This function cannot read from the hash or finalized fields.
     */

    m_serialize_i4(write_handle, object->type);
    m_serialize_i4(write_handle, object->flags);
}

struct m_cas_write_handle_t *
m_serialize_begin(struct m_object_t *object)
{
    struct m_cas_write_handle_t *write_handle;

    write_handle = m_cas_write_open();
    m_serialize_header(write_handle, object);

    return write_handle;
}

void
m_serialize_i4(struct m_cas_write_handle_t *write_handle, unsigned int value)
{
    int bytes;
    char buf[5];

    union
    {
        char bytes[4];
        unsigned int value;
    } u;

    u.value = value;

    if (value < 128)
    {
        buf[0] = u.bytes[0];
        bytes = 1;
    }
    else if (value < 65536)
    {
        buf[0] = 0x80;
        buf[1] = u.bytes[0];
        buf[2] = u.bytes[1];
        bytes = 3;
    }
    else
    {
        buf[0] = 0xff;
        buf[1] = u.bytes[0];
        buf[2] = u.bytes[1];
        buf[3] = u.bytes[2];
        buf[4] = u.bytes[3];
        bytes = 5;
    }

#if M_BIG_ENDIAN
#   error TODO: Handle big endian integers
#endif

    m_cas_write(write_handle, buf, bytes);
}

void
m_serialize_string(struct m_cas_write_handle_t *write_handle, const char *string)
{
    size_t string_length;
    unsigned int length;

    string_length = strlen(string);
    length = (unsigned int)string_length;

    assert((size_t)length == string_length);

    m_serialize_i4(write_handle, length);
    m_cas_write(write_handle, string, length);
}

void
m_serialize_object_ref(struct m_cas_write_handle_t *write_handle, struct m_object_t *object)
{
    char *p;

    if (!object->finalized)
    {
        m_object_finalize(object);
    }

    p = (char *)&object->hash;
    m_cas_write(write_handle, p, sizeof(struct m_sha1_hash_t));
}

void
m_serialize_end(struct m_cas_write_handle_t *write_handle, struct m_object_t *object)
{
    object->hash = m_cas_write_close(write_handle);
    object->finalized = 1;
}

struct m_object_t *
m_object_null_create(void)
{
    struct m_cas_write_handle_t *write_handle;
    char buf[1] = { 0 };
    struct m_sha1_hash_t hash;
    struct m_object_t *object;

    object = calloc(1, sizeof(struct m_object_t));

    write_handle = m_cas_write_open();
    m_serialize_header(write_handle, object);
    m_cas_write(write_handle, buf, 0);
    hash = m_cas_write_close(write_handle);

    object->hash = hash;
    object->finalized = 1;

    return object;
}

static void
m_object_finalize_noop(struct m_object_t *object)
{
    M_UNUSED(object);

    assert(object->finalized == 1);
}

static void
m_object_finalize_error(struct m_object_t *object)
{
    M_UNUSED(object);

    abort();
}

void
m_object_finalize(struct m_object_t *object)
{
    typedef void (*m_finalize_fn_t)(struct m_object_t *);
    static m_finalize_fn_t object_finalizers[M_NUM_TYPES] = {
        m_object_finalize_noop,
        m_reference_set_finalize,
        m_object_finalize_error,
        m_tree_finalize,
        m_commit_finalize,
        m_branch_finalize,
        m_repository_finalize
    };

    object_finalizers[object->type](object); 
}

unsigned int
m_realize_i4(const void *data, size_t *offset_ptr, size_t size)
{
    const char *ptr;
    unsigned char b0;
    size_t offset;
    
    union
    {
        char bytes[4];
        unsigned int value;
    } u;

    ptr = data; 
    offset = *offset_ptr;
    u.value = 0;

    assert(offset < size);
    b0 = (unsigned char)*ptr;

    if (b0 < 128)
    {
        *offset_ptr = offset + 1;
        return (unsigned int)b0;
    }
    else if (b0 == 0x80)
    {
        u.bytes[0] = ptr[1];
        u.bytes[1] = ptr[2];
        *offset_ptr = offset + 3;

        return u.value;
    }
    else if (b0 == 0xff)
    {
        u.bytes[0] = ptr[1];
        u.bytes[1] = ptr[2];
        u.bytes[2] = ptr[3];
        u.bytes[3] = ptr[4];
        *offset_ptr = offset + 5;

        return u.value;
    }

#if M_BIG_ENDIAN
#   error TODO: Handle big endian integers
#endif

    assert(0 && "This shouldn't be here.");
    return 0;
}

static void
m_realize_header(const void *data, size_t *offset, size_t size, struct m_object_t *object)
{
    object->type = m_realize_i4(data, offset, size);
    object->flags = m_realize_i4(data, offset, size);
}

static enum m_object_type_t
m_peek_type(const void *data, size_t size)
{
    const char *ptr;
    char byte;

    ptr = data;
    byte = *ptr;

    assert(size != 0);
    assert(byte < M_NUM_TYPES);

    return (enum m_object_type_t)byte;
}

static struct m_object_t *
m_object_construct_error(const void *data, size_t size)
{
    M_UNUSED(data);
    M_UNUSED(size);

    abort();
}

struct m_object_t *
m_object_realize(struct m_sha1_hash_t hash)
{
    typedef struct m_object_t *(*m_constructor_fn_t)(const void *, size_t);
    m_constructor_fn_t object_constructors[M_NUM_TYPES] = {
        m_object_construct_error,
        m_reference_set_construct,
        m_object_construct_error,
        m_tree_construct,
        m_commit_construct,
        m_branch_construct,
        m_repository_construct
    };

    enum m_object_type_t type;
    int rv;
    void *data;
    size_t size;
    struct m_object_t *object;

    rv = m_cas_read(hash, &data, &size);

    if (rv < 0)
    {
        return NULL;
    }

    type = m_peek_type(data, size);
    object = object_constructors[type](data, size);
    free(data);

    return object;
}

