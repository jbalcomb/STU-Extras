/* FuncExtract_treesitter.c
 * Minimal example: parse a C file with Tree-sitter and print function names.
 * Build only when Tree-sitter dev headers/libs are available.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <tree_sitter/api.h>

/* The C grammar provides this function when linked with the language library. */
const TSLanguage *tree_sitter_c(void);

static TSNode make_null_node(void) {
    TSNode node = {0};
    return node;
}

static bool node_is_function_definition(TSNode node) {
    const char *t = ts_node_type(node);
    return (t && strcmp(t, "function_definition") == 0);
}

static TSNode find_identifier_recursive(TSNode node) {
    if (ts_node_is_null(node)) return make_null_node();

    const char *t = ts_node_type(node);
    if (t && strcmp(t, "identifier") == 0) return node;

    uint32_t n = ts_node_named_child_count(node);
    for (uint32_t i = 0; i < n; ++i) {
        TSNode c = ts_node_named_child(node, i);
        TSNode res = find_identifier_recursive(c);
        if (!ts_node_is_null(res)) return res;
    }
    return make_null_node();
}

static void extract_functions(TSNode node, const char *source, size_t source_len) {
    if (ts_node_is_null(node)) return;

    if (node_is_function_definition(node)) {
        TSNode id = find_identifier_recursive(node);
        if (!ts_node_is_null(id)) {
            uint32_t start = ts_node_start_byte(id);
            uint32_t end = ts_node_end_byte(id);
            if (start < source_len && end <= source_len && end > start) {
                printf("%.*s\n", (int)(end - start), source + start);
            }
        }
    }

    uint32_t n = ts_node_named_child_count(node);
    for (uint32_t i = 0; i < n; ++i) {
        TSNode c = ts_node_named_child(node, i);
        extract_functions(c, source, source_len);
    }
}

static char *read_file(const char *path, size_t *out_len) {
    FILE *f = NULL;
    if (0 != fopen_s(&f, path, "rb") || !f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return NULL; }
    rewind(f);
    char *buf = malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t r = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[r] = '\0';
    if (out_len) *out_len = r;
    return buf;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source.c>\n", argv[0]);
        return 2;
    }

    size_t len = 0;
    char *source = read_file(argv[1], &len);
    if (!source) {
        perror("read_file");
        return 1;
    }

    TSParser *parser = ts_parser_new();
    if (!parser) {
        fprintf(stderr, "Failed to create TSParser\n");
        free(source);
        return 1;
    }

    ts_parser_set_language(parser, tree_sitter_c());

    TSTree *tree = ts_parser_parse_string(parser, NULL, source, (uint32_t)len);
    if (!tree) {
        fprintf(stderr, "Parse failure\n");
        ts_parser_delete(parser);
        free(source);
        return 1;
    }

    TSNode root = ts_tree_root_node(tree);
    extract_functions(root, source, len);

    ts_tree_delete(tree);
    ts_parser_delete(parser);
    free(source);
    return 0;
}
