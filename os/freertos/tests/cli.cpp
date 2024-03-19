#include "../services/cli/cli.h"

#include <string.h>

#include <catch2/catch_all.hpp>

extern "C" struct cli_node *cli_node_get_root();

int cli_default_handler(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return 0;
}

void cli_check_correct_cmd(char *cmd_name) {
    struct cli_node *node_;
    struct cli_node *node;

    node = cli_cmd_reg(cmd_name, cli_default_handler);
    REQUIRE(node != NULL);
    REQUIRE(strstr(cmd_name, node->name) != NULL);

    node_ = cli_cmd_find(cmd_name);
    REQUIRE(node_ != NULL);
    REQUIRE(strcmp(node_->name, node->name) == 0);
    REQUIRE(node_->handler == cli_default_handler);
}

void cli_check_incorrect_cmd(char *cmd_name) {
    struct cli_node *node;
    node = cli_cmd_reg(cmd_name, cli_default_handler);
    REQUIRE(node == NULL);
}

TEST_CASE("Create command with correct name") {
    cli_check_correct_cmd("sens");
    cli_check_correct_cmd("actuators");
    cli_check_correct_cmd("pid");
}

TEST_CASE("Create command with incorrect name") {
    cli_check_incorrect_cmd(" sens");
    cli_check_incorrect_cmd("actu ators");
    cli_check_incorrect_cmd("pid ");
}
