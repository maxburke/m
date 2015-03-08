#include <stdio.h>

#include "m_base.h"
#include "m_branch.h"
#include "m_commit.h"
#include "m_file.h"
#include "m_reference_set.h"
#include "m_repository.h"
#include "m_tree.h"

typedef int (*m_action_fn_t)(int, char **);

struct m_action_t
{
    const char *command;
    m_action_fn_t action;
    const char *short_help;
};

static int
m_add(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

static int
m_edit(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

static int
m_rm(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

static int
m_revert(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

static int
m_init(int argc, char **argv)
{
    const char *repository_name;
    const char *default_branch_name = "master";

    if (argc < 3)
    {
        return -1;
    }

    repository_name = argv[2];

    if (argc >= 4)
    {
        default_branch_name = argv[3];
    }

    if (m_make_meta_dir())
    {
        return -1;
    }

    struct m_object_t *head_items;
    struct m_object_t *tree;
    struct m_object_t *head;
    struct m_object_t *null_object;
    struct m_object_t *branch;
    struct m_object_t *branch_list;
    struct m_object_t *repository;

    head_items      = m_reference_set_create();
    tree            = m_tree_create("", head_items);
    null_object     = m_object_null_create();
    head            = m_commit_create("Repository created.", null_object, tree);
    branch          = m_branch_create(default_branch_name, head);

    branch_list     = m_reference_set_create();
    m_reference_set_add(branch_list, branch);
    repository      = m_repository_create(branch, repository_name, branch_list);
    m_repository_finalize(repository);

    m_write("repo.dat", &repository->hash, sizeof repository->hash);

    return 0;
}

static int
m_branch(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

static int
m_checkout(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

static int
m_merge(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

static int
m_push(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

static int
m_pull(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

static int
m_resolve(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

static int
m_log(int argc, char **argv)
{
    M_UNUSED(argc);
    M_UNUSED(argv);

    return -1;
}

static int
m_help(int argc, char **argv);

static struct m_action_t actions[] = {
    { "add",        m_add },
    { "edit",       m_edit },
    { "rm",         m_rm },
    { "revert",     m_revert },

    { "init",       m_init },
    { "branch",     m_branch },
    { "checkout",   m_checkout },
    { "merge",      m_merge },
    { "push",       m_push },
    { "pull",       m_pull },
    { "resolve",    m_resolve },

    { "log",        m_log },
    { "help",       m_help }
};

static int
m_help(int argc, char **argv)
{
    M_UNUSED(argv);

    if (argc < 2)
    {
        size_t i;

        puts("usage: m <command> [args]");

        for (i = 0; i < M_ARRAY_COUNT(actions); ++i)
        {
            printf("    %-10s %s\n", actions[i].command, actions[i].short_help);
        }
    }

    return -1;
}

int
main(int argc, char **argv)
{
    size_t i;

    if (argc < 2)
    {
        return m_help(argc, argv);
    }

    for (i = 0; i < M_ARRAY_COUNT(actions); ++i)
    {
        if (m_streq(actions[i].command, argv[1]))
        {
            return actions[i].action(argc, argv);
        }
    }

    return -1;
}
