#include <catch2/catch.hpp>
#include <node.h>

TEST_CASE("Create root") {
    struct node *root;
    root = node_init();
    REQUIRE(root != NULL);
    REQUIRE(root == node_init());
}

void node_check_correct_path(const char *name, struct node *node, 
                             struct node *root) {
    file_operations f_op = {};
    struct node *node_;

    node = node_reg(name, &f_op);
    REQUIRE(node != NULL);
    REQUIRE(strstr(name, node->name) != NULL);

    node_ = node_find(name);
    REQUIRE(node_ != NULL);
    REQUIRE(strcmp(node_->name, node->name) == 0);
}

void node_check_incorrect_path(const char *name, struct node *node, 
                               struct node *root) {
    file_operations f_op = {};
    node = node_reg(name, &f_op);
    REQUIRE(node == NULL);
}

TEST_CASE("Create nodes with correct paths") {
    struct node *node;
    struct node *root;
    file_operations f_op = {};
    char name[32] = {};
    char section_name[64] = {};

    root = node_init();

    strcpy(name, "/dev");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name) {
        node_check_correct_path(name, node, root);
        REQUIRE(strstr(name, root->child->name) != NULL);
    }

    strcpy(name, "/dev/ttyUSB0");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name, name) {
        node_check_correct_path(name, node, root);
        REQUIRE(strstr(name, root->child->child->name) != NULL);
    }

    strcpy(name, "/dev/ttyUSB1");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name, name) {
        node_check_correct_path(name, node, root);
        REQUIRE(strstr(name, root->child->child->sibling->name) != NULL);
    }

    strcpy(name, "/fs");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name, name) {
        node_check_correct_path(name, node, root);
        REQUIRE(strstr(name, root->child->sibling->name) != NULL);
    }

    strcpy(name, "/dev/too/much/directories");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name, name) {
        node_check_correct_path(name, node, root);
        REQUIRE(!strcmp("dev", root->child->name));
        REQUIRE(!strcmp("too", root->child->child->sibling->sibling->name));
        REQUIRE(!strcmp("much", root->child->child->sibling->sibling->child->name));
        REQUIRE(!strcmp("directories", root->child->child->sibling->sibling->child->child->name));
    }

    strcpy(name, "//tmp");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name, name) {
        node_check_correct_path(name, node, root);
        REQUIRE(strstr(name, root->child->sibling->sibling->name) != NULL);
    }

    strcpy(name, "proc");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name, name) {
        node_check_correct_path(name, node, root);
        REQUIRE(strstr(name, root->child->sibling->sibling->sibling->name) != NULL);
    }

}

TEST_CASE("Create nodes with incorrect paths") {
    struct node *node;
    struct node *root;
    file_operations f_op = {};
    char name[64] = {};
    char section_name[128] = {};

    root = node_init();

    strcpy(name, "NULL name");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name, name) {
        node_check_incorrect_path(NULL, node, root);
        REQUIRE(errno == EINVAL);
    }

    strcpy(name, "NULL f_op");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name, name) {
        node = node_reg(name, NULL);
        REQUIRE(node == NULL);
        REQUIRE(errno == EINVAL);
    }

    strcpy(name, "/");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name) {
        node_check_incorrect_path(name, node, root);
        REQUIRE(errno == EINVAL);
    }

    strcpy(name, "/dev/ttyUSB0");
    sprintf(section_name, "Repeated path: %s", name);
    SECTION(section_name, name) {
        node_check_incorrect_path(name, node, root);
        REQUIRE(errno == EEXIST);
    }

    strcpy(name, "/longlongname/longlongname/longlongname");
    sprintf(section_name, "Path: %s", name);
    SECTION(section_name, name) {
        node_check_incorrect_path(name, node, root);
        REQUIRE(errno == EINVAL);
    }
}
