#include <catch2/catch_all.hpp>

#include "node.h"


extern "C" struct node *node_get_root();

void node_check_correct_path(const char *path) {
    file_operations f_op = {0};
    struct node *node_;
    struct node *node;

    node = node_mount(path, &f_op);
    REQUIRE(node != NULL);
    REQUIRE(strstr(path, node->name) != NULL);

    node_ = node_find(path);
    REQUIRE(node_ != NULL);
    REQUIRE(strcmp(node_->name, node->name) == 0);
}

void node_check_incorrect_path(const char *name, struct node *node) {
    file_operations f_op = {0};
    node = node_mount(name, &f_op);
    REQUIRE(node == NULL);
}

TEST_CASE("Create nodes with correct paths") {

    struct node *root = node_get_root();

    file_operations f_op = {};
    char name[32] = {};
    char section_name[64] = {};

    strcpy(name, "/dev");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name) {
        node_check_correct_path(name);
        REQUIRE(strstr(name, root->child->name) != NULL);
    }

    strcpy(name, "/dev/ttyUSB0");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name, name) {
        node_check_correct_path(name);
        REQUIRE(strstr(name, root->child->child->name) != NULL);
    }

    strcpy(name, "/dev/ttyUSB1");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name, name) {
        node_check_correct_path(name);
        REQUIRE(strstr(name, root->child->child->sibling->name) != NULL);
    }

    strcpy(name, "/fs");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name, name) {
        node_check_correct_path(name);
        REQUIRE(strstr(name, root->child->sibling->name) != NULL);
    }

    strcpy(name, "/dev/too/much/directories");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name, name) {
        node_check_correct_path(name);
        REQUIRE(!strcmp("dev", root->child->name));
        REQUIRE(!strcmp("too", root->child->child->sibling->sibling->name));
        REQUIRE(
            !strcmp("much", root->child->child->sibling->sibling->child->name));
        REQUIRE(
            !strcmp("directories",
                    root->child->child->sibling->sibling->child->child->name));
    }

    strcpy(name, "//tmp");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name, name) {
        node_check_correct_path(name);
        REQUIRE(strstr(name, root->child->sibling->sibling->name) != NULL);
    }
}

TEST_CASE("Create nodes with incorrect paths") {
    struct node *node;
    struct node *root;
    file_operations f_op = {};
    char name[64] = {};
    char section_name[128] = {};

    strcpy(name, "NULL name");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name, name) {
        node_check_incorrect_path(NULL, node);
        REQUIRE(errno == EINVAL);
    }

    strcpy(name, "NULL f_op");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name, name) {
        node = node_mount(name, NULL);
        REQUIRE(node == NULL);
        REQUIRE(errno == EINVAL);
    }

    strcpy(name, "/");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name) {
        node_check_incorrect_path(name, node);
        REQUIRE(errno == EEXIST);
    }

    strcpy(name, "/dev/ttyUSB0");
    sprintf(section_name, "Repeated path: %s", name);
    SECTION(section_name, name) {
        node_check_incorrect_path(name, node);
        REQUIRE(errno == EEXIST);
    }

    strcpy(name, "proc");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name, name) {
        node_check_incorrect_path(name, node);
    }

    strcpy(name, "/dev/");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name, name) {
        node_check_incorrect_path(name, node);
    }
}
