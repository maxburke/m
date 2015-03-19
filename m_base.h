#pragma once

#include "m_sha1.h"

#define M_ARRAY_COUNT(x) ((sizeof (x))/(sizeof (x)[0]))
#define M_UNUSED(x) ((void)x)

#ifdef _MSC_VER
#   define M_VERIFY(x, y) \
        __pragma(warning(push)) \
        __pragma(warning(disable:4127)) \
        do { if (!(x)) { m_report_fatal_error(y); } } while (0) \
        __pragma(warning(pop))
#else
#   define M_VERIFY(x, y) do { if (!(x)) { m_report_fatal_error(y); } } while (0)
#endif


/**
 * Convenience method for testing string equality.
 *
 * \return Will return 0 if strings do not match, non-zero if they do.
 */
int
m_streq(const char *a, const char *b);

/**
 * Report a fatal error then exit the program with a non-zero exit code.
 */
void
m_report_fatal_error(const char *reason);

#ifdef _MSC_VER
/**
 * Convenience method for platform C libraries that do not support strlcpy,
 * such as Windows. Unlike strncpy, strlcpy will guarantee that the output
 * is null terminated. If the returned value is less than the size parameter
 * then output has been truncated.
 *
 * \param[out] dest Buffer into which string contents are copied.
 * \param[in] src Source from which data is copied.
 * \param[in] size
 *
 * \return The c-string length of the output copied to the destination buffer.
 */
size_t
strlcpy(char *dest, const char *src, size_t size);
#endif

enum m_object_type_t
{
    M_NULL,
    M_REFERENCE_SET,
    M_DATA,
    M_TREE,
    M_COMMIT,
    M_BRANCH,
    M_REPOSITORY,
    M_NUM_TYPES
};

struct m_object_t
{
    struct m_sha1_hash_t hash;
    char finalized;
    char realized;
    unsigned short version;
    unsigned int type;
    unsigned int flags;
};

struct m_cas_write_handle_t;

struct m_cas_write_handle_t *
m_serialize_begin(struct m_object_t *object);

void
m_serialize_i4(struct m_cas_write_handle_t *write_handle, unsigned int value);

void
m_serialize_string(struct m_cas_write_handle_t *write_handle, const char *string);

void
m_serialize_object_ref(struct m_cas_write_handle_t *write_handle, struct m_object_t *object);

void
m_serialize_end(struct m_cas_write_handle_t *write_handle, struct m_object_t *object);

struct m_object_t *
m_object_null_create(void);

void
m_object_finalize(struct m_object_t *object);

unsigned int
m_realize_i4(const void *data, size_t *offset, size_t size);

char *
m_realize_string(const void *data, size_t *offset, size_t size);

struct m_object_t *
m_realize_ref(const void *data, size_t *offset, size_t size);

void
m_realize_header(const void *data, size_t *offset, size_t size, struct m_object_t *object);

struct m_object_t *
m_object_realize(struct m_sha1_hash_t hash);

