#include "../cli/cli.h"

#include <string.h>

#include <catch2/catch_all.hpp>
#include <iostream>

extern "C" struct cli_node *cli_node_get_root();

int cli_default_handler(int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        printf("cli_default_handler: command number = %d, command = %s\n", i,
               argv[i]);
    }
    return 0;
}

void cli_check_correct_cmd(const char *cmd_path) {
    struct cli_node *node_;
    struct cli_node *node;

    node = cli_cmd_reg(cmd_path, cli_default_handler);
    REQUIRE(node != NULL);
    REQUIRE(strstr(cmd_path, node->name) != NULL);

    node_ = cli_cmd_find(cmd_path, CLI_MODE_FULL_PATH);
    REQUIRE(node_ != NULL);
    REQUIRE(strcmp(node_->name, node->name) == 0);
    REQUIRE(node_->handler == cli_default_handler);
}

void cli_check_incorrect_cmd(const char *cmd_path) {
    struct cli_node *node;
    node = cli_cmd_reg(cmd_path, cli_default_handler);
    REQUIRE(node == NULL);
}

#define MAX_NAME 32
#define MAX_ARGS 32

TEST_CASE("Create command with correct paths") {
    struct cli_node *root = cli_node_get_root();

    char name[MAX_NAME] = {0};
    char *arg[MAX_ARGS] = {0};
    char *arg_str;

    for(int i = 0; i < MAX_ARGS; i++) {
        arg[i] = (char *)calloc(MAX_NAME, 1);
    }

    arg_str = arg[0];
    strcpy(arg_str, "sens");
    snprintf(name, MAX_NAME, "Command: %s", arg_str);
    SECTION(name) {
        cli_check_correct_cmd(arg_str);
        REQUIRE(strstr(arg_str, root->child->name) != NULL);
        cli_cmd_execute(arg_str, 1, (char **)arg);
    }

    arg_str = arg[1];
    strcpy(arg_str, "sens stat");
    snprintf(name, MAX_NAME, "Command: %s", arg_str);
    SECTION(name) {
        cli_check_correct_cmd(arg_str);
        REQUIRE(strstr(arg_str, root->child->name) != NULL);
        cli_cmd_execute(arg_str, 1, (char **)(arg + 1));
    }

    arg_str = arg[2];
    strcpy(arg_str, "act");
    snprintf(name, MAX_NAME, "Command: %s", arg_str);
    SECTION(name) {
        cli_check_correct_cmd(arg_str);
        REQUIRE(strstr(arg_str, root->child->sibling->name) != NULL);
        cli_cmd_execute(arg_str, 1, (char **)(arg + 2));
    }

    arg_str = arg[3];
    strcpy(arg_str, "act pos set");
    snprintf(name, MAX_NAME, "Command: %s", arg_str);
    SECTION(name) {
        cli_check_correct_cmd(arg_str);
        REQUIRE(strstr("pos", root->child->sibling->child->name) != NULL);
        REQUIRE(strstr("set", root->child->sibling->child->child->name) != NULL);
        cli_cmd_execute(arg_str, 1, (char **)(arg + 3));
    }

    SECTION("Show all commands") {
        REQUIRE(cli_show_subcmd("") == 0);
        REQUIRE(cli_show_subcmd("act") == 0);
        REQUIRE(cli_show_subcmd("act pos") == 0);
        REQUIRE(cli_show_subcmd("sens") == 0);
    }
}
