#include "m_base.h"
#include "m_file.h"

typedef int (*m_action_fn_t)(int, char **);

struct m_action_t
{
    const char *command;
    m_action_fn_t action;
};

int m_add(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

int m_edit(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

int m_rm(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

int m_revert(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

int m_init(int argc, char **argv)
{
    const char *repository_name;

    struct m_reference_set_t *head_items;
    m_ref_t head_items_ref;

    struct m_commit_t *head;
    m_ref_t head_ref;

    struct m_branch_t *branch;
    m_ref_t branch_ref;

    struct m_reference_set_t *branch_list;
    m_ref_t branch_list_ref;

    struct m_repository_t *repository;
    m_ref_t repository_ref;

    char buf[1] = { 0 };
    m_ref_t null_ref = m_sha1_hash_buffer(buf, 0);

    if (argc < 3)
    {
        return -1;
    }

    repository_name = argv[2];

    if (m_make_meta_dir())
    {
        return -1;
    }

    head_items      = m_reference_set_create();
    head_items_ref  = m_reference_set_finalize(head_items);

    tree
#error commit needs a tree reference
    head            = m_commit_create(null_ref, head_items_ref);
    head_ref        = m_commit_finalize(head);

    branch          = m_branch_create("master", head_ref);
    branch_ref      = m_branch_finalize(branch);

    branch_list     = m_reference_set_create();
    m_reference_set_add(branch_list, branch_ref);

    branch_list_ref = m_reference_set_finalize(branch_list);

    repository      = m_repository_create(branch_ref, repository_name, branch_list_ref);
    repository_ref  = m_repository_finalize(repository);

    return 0;
}

int m_branch(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

int m_checkout(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

int m_merge(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

int m_push(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

int m_pull(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

int m_resolve(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

int m_log(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

static struct m_action_t actions[] = {
    { "add", m_add },
    { "edit", m_edit },
    { "rm", m_rm },
    { "revert", m_revert },

    { "init", m_init },
    { "branch", m_branch },
    { "checkout", m_checkout },
    { "merge", m_merge },
    { "push", m_push },
    { "pull", m_pull },
    { "resolve", m_resolve },

    { "log", m_log }
};


int main(int argc, char **argv)
{
    size_t i;

    if (argc < 2)
    {
        return -1;
    }

    for (i = 0; i < M_ARRAY_COUNT(actions); ++i)
    {
        if (m_streq(actions[i].command, argv[i]))
        {
            return actions[i].action(argc, argv);
        }
    }

    return -1;
}
