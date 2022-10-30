#include "node.h"

typedef struct {
    uint8_t *file_str;
    size_t file_len;
    size_t offset;
} xml_inline_ocb_t; // open context block

/**
 * This call is generated by the c-atom's tool xml2c_inliner.py
 * simply run 'make prebuild'
 * @param path
 * @param size
 * @return
 */
const char *xml_inline_find_file(const char *path, size_t *size);

static int xml_inline_open_callback(void *devcfg, void *file, const char* pathname, int flags) {
    xml_inline_ocb_t *ocb = (xml_inline_ocb_t*) file;

    size_t filesize;

    // FIXME: quick fixed now, fix it appropriately
    pathname += strlen("/cfg/");

    const char *file_string = xml_inline_find_file(pathname, &filesize);

    if (file_string == NULL) {
        errno = ENOENT;
        return -1;
    }
    // check size
#   define MAXLEN (filesize + 100)
    ocb->file_len = strnlen(file_string, MAXLEN);
    if (ocb->file_len >= MAXLEN) {
        errno = EFBIG;
        return -1;
    }
    if (ocb->file_len != filesize) {
        errno = EIO;
        return -1;
    }

    ocb->file_str = (uint8_t *) file_string;

    ocb->offset = 0;

    return 0;
}

static ssize_t xml_inline_read_callback(void *devcfg, void *file, void *buf, size_t count) {
    xml_inline_ocb_t *ocb = (xml_inline_ocb_t*) file;

    size_t bytes_left = ocb->file_len - ocb->offset;
    ssize_t br = count > bytes_left ? bytes_left : count;

    if (br > 0) {
        memcpy(buf, ocb->file_str + ocb->offset, br);
        ocb->offset += br;
    }

    errno = 0;

    return br;
}


static int xml_inline_close_callback(void *devcfg, void *file) {
    xml_inline_ocb_t *ocb = (xml_inline_ocb_t*) file;
    // TODO
    return 0;
}


void *xml_inline_filealloc_callback(void) {
    xml_inline_ocb_t *ocb = calloc(1, sizeof(*ocb));
    return ocb;
}


int xml_inline_mount(const char *mount_to) {

    int nd = nodereg(mount_to);
    noderegopen(nd, xml_inline_open_callback);
    noderegread(nd, xml_inline_read_callback);
    noderegclose(nd, xml_inline_close_callback);
    noderegfilealloc(nd, xml_inline_filealloc_callback);
    noderegdevcfg(nd, NULL);

    return 0;
}
