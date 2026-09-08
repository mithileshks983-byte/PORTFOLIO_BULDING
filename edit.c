/*
 * editor.c — A simple command-line line editor
 *
 * Data structure: dynamic array of C-strings (char**), growing by doubling.
 * Justification: line numbers map directly to array indices, so display
 * and "go to line N" operations are O(1) to locate. Insert/delete are
 * O(n) because of the shift, which is perfectly fine for a small document
 * edited interactively. A linked list would make insert/delete O(1) once
 * you're at the right node, but getting to "line N" is O(n) anyway with
 * a list (no random access), and it costs more memory per line (extra
 * pointers) and locality (pointer chasing vs. a contiguous array). For
 * this use case the dynamic array is simpler and just as fast in practice.
 *
 * Compile:  gcc -Wall -Wextra -o editor editor.c
 * Run:      ./editor
 * Help:     type "help" at the prompt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INITIAL_CAPACITY 8
#define MAX_INPUT_LEN 2048

typedef struct {
    char **lines;   /* array of heap-allocated line strings */
    int count;      /* number of lines currently stored */
    int capacity;   /* allocated slots in the array */
} Document;

/* ---------- small helpers ---------- */

static char *str_duplicate(const char *s) {
    char *copy = malloc(strlen(s) + 1);
    if (!copy) { fprintf(stderr, "Out of memory.\n"); exit(1); }
    strcpy(copy, s);
    return copy;
}

static char *skip_spaces(char *s) {
    while (*s == ' ' || *s == '\t') s++;
    return s;
}

/* ---------- document lifecycle ---------- */

static void doc_init(Document *doc) {
    doc->capacity = INITIAL_CAPACITY;
    doc->count = 0;
    doc->lines = malloc(sizeof(char *) * doc->capacity);
    if (!doc->lines) { fprintf(stderr, "Out of memory.\n"); exit(1); }
}

static void doc_grow_if_needed(Document *doc) {
    if (doc->count >= doc->capacity) {
        doc->capacity *= 2;
        doc->lines = realloc(doc->lines, sizeof(char *) * doc->capacity);
        if (!doc->lines) { fprintf(stderr, "Out of memory.\n"); exit(1); }
    }
}

static void doc_free(Document *doc) {
    for (int i = 0; i < doc->count; i++) free(doc->lines[i]);
    free(doc->lines);
    doc->lines = NULL;
    doc->count = doc->capacity = 0;
}

/* ---------- core features ---------- */

static void doc_insert(Document *doc, int line_num, const char *text) {
    if (line_num < 1 || line_num > doc->count + 1) {
        printf("Error: invalid line number %d (valid range: 1 to %d).\n",
               line_num, doc->count + 1);
        return;
    }
    doc_grow_if_needed(doc);
    for (int i = doc->count; i >= line_num; i--) {
        doc->lines[i] = doc->lines[i - 1];
    }
    doc->lines[line_num - 1] = str_duplicate(text);
    doc->count++;
    printf("Inserted line %d.\n", line_num);
}

static void doc_delete(Document *doc, int line_num) {
    if (doc->count == 0) {
        printf("Error: document is empty, nothing to delete.\n");
        return;
    }
    if (line_num < 1 || line_num > doc->count) {
        printf("Error: invalid line number %d (valid range: 1 to %d).\n",
               line_num, doc->count);
        return;
    }
    free(doc->lines[line_num - 1]);
    for (int i = line_num - 1; i < doc->count - 1; i++) {
        doc->lines[i] = doc->lines[i + 1];
    }
    doc->count--;
    printf("Deleted line %d.\n", line_num);
}

static void doc_display(Document *doc) {
    if (doc->count == 0) {
        printf("(document is empty)\n");
        return;
    }
    for (int i = 0; i < doc->count; i++) {
        printf("%4d | %s\n", i + 1, doc->lines[i]);
    }
}

/* ---------- save / load (core-adjacent, required by deliverables) ---------- */

static void doc_save(Document *doc, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        printf("Error: could not open '%s' for writing.\n", filename);
        return;
    }
    for (int i = 0; i < doc->count; i++) fprintf(f, "%s\n", doc->lines[i]);
    fclose(f);
    printf("Saved %d line(s) to '%s'.\n", doc->count, filename);
}

static void doc_load(Document *doc, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("Error: could not open '%s' for reading.\n", filename);
        return;
    }
    for (int i = 0; i < doc->count; i++) free(doc->lines[i]);
    doc->count = 0;

    char buffer[MAX_INPUT_LEN];
    while (fgets(buffer, sizeof(buffer), f)) {
        buffer[strcspn(buffer, "\n")] = '\0';
        doc_grow_if_needed(doc);
        doc->lines[doc->count++] = str_duplicate(buffer);
    }
    fclose(f);
    printf("Loaded %d line(s) from '%s'.\n", doc->count, filename);
}

/* ---------- bonus: search ---------- */

static void doc_search(Document *doc, const char *needle) {
    int found = 0;
    for (int i = 0; i < doc->count; i++) {
        if (strstr(doc->lines[i], needle)) {
            printf("  line %d: %s\n", i + 1, doc->lines[i]);
            found = 1;
        }
    }
    if (!found) printf("\"%s\" was not found in the document.\n", needle);
}

/* ---------- bonus: find & replace ---------- */

/* Returns a newly allocated string with every occurrence of `old_w`
 * replaced by `new_w`. Caller must free the result. */
static char *replace_in_string(const char *src, const char *old_w, const char *new_w) {
    size_t old_len = strlen(old_w);
    size_t new_len = strlen(new_w);
    if (old_len == 0) return str_duplicate(src); /* nothing to match */

    /* First pass: count occurrences so we can size the buffer exactly. */
    size_t count = 0;
    const char *scan = src;
    while ((scan = strstr(scan, old_w)) != NULL) {
        count++;
        scan += old_len;
    }

    size_t result_len = strlen(src) + count * (new_len - old_len) + 1;
    char *result = malloc(result_len);
    if (!result) { fprintf(stderr, "Out of memory.\n"); exit(1); }

    char *out = result;
    const char *cur = src;
    const char *match;
    while ((match = strstr(cur, old_w)) != NULL) {
        size_t chunk = (size_t)(match - cur);
        memcpy(out, cur, chunk);
        out += chunk;
        memcpy(out, new_w, new_len);
        out += new_len;
        cur = match + old_len;
    }
    strcpy(out, cur); /* copy the remainder, including the null terminator */
    return result;
}

static void doc_replace_line(Document *doc, int line_num, const char *old_w, const char *new_w) {
    if (line_num < 1 || line_num > doc->count) {
        printf("Error: invalid line number %d (valid range: 1 to %d).\n",
               line_num, doc->count);
        return;
    }
    char *new_line = replace_in_string(doc->lines[line_num - 1], old_w, new_w);
    free(doc->lines[line_num - 1]);
    doc->lines[line_num - 1] = new_line;
    printf("Replaced \"%s\" with \"%s\" on line %d.\n", old_w, new_w, line_num);
}

static void doc_replace_all(Document *doc, const char *old_w, const char *new_w) {
    for (int i = 0; i < doc->count; i++) {
        char *new_line = replace_in_string(doc->lines[i], old_w, new_w);
        free(doc->lines[i]);
        doc->lines[i] = new_line;
    }
    printf("Replaced \"%s\" with \"%s\" across the whole document.\n", old_w, new_w);
}

/* ---------- bonus: line/word count ---------- */

static void doc_count(Document *doc) {
    long words = 0;
    for (int i = 0; i < doc->count; i++) {
        int in_word = 0;
        for (const char *p = doc->lines[i]; *p; p++) {
            if (!isspace((unsigned char)*p)) {
                if (!in_word) { words++; in_word = 1; }
            } else {
                in_word = 0;
            }
        }
    }
    printf("Lines: %d, Words: %ld\n", doc->count, words);
}

/* ---------- help ---------- */

static void print_help(void) {
    printf(
        "Commands:\n"
        "  insert <n> <text>       Insert <text> as line <n> (existing lines shift down)\n"
        "  delete <n>              Delete line <n> (lines below shift up)\n"
        "  display                 Show the whole document with line numbers\n"
        "  save <file>             Write the document to <file>\n"
        "  load <file>             Read a document from <file> (replaces current one)\n"
        "  search <word>           List every line containing <word>\n"
        "  replace <n> <old> <new> Replace <old> with <new> on line <n>\n"
        "  replaceall <old> <new>  Replace <old> with <new> on every line\n"
        "  count                   Show line count and word count\n"
        "  help                    Show this message\n"
        "  quit                    Exit the editor\n"
    );
}

/* ---------- command loop ---------- */

int main(void) {
    Document doc;
    doc_init(&doc);

    char input[MAX_INPUT_LEN];
    printf("Simple Line Editor. Type 'help' for commands, 'quit' to exit.\n");

    while (1) {
        printf("> ");
        if (!fgets(input, sizeof(input), stdin)) break; /* EOF */
        input[strcspn(input, "\n")] = '\0';

        char *p = skip_spaces(input);
        if (*p == '\0') continue; /* blank line */

        char cmd[32];
        int n = 0;
        if (sscanf(p, "%31s%n", cmd, &n) != 1) continue;
        p = skip_spaces(p + n);

        if (strcmp(cmd, "insert") == 0) {
            int line_num, n2 = 0;
            if (sscanf(p, "%d%n", &line_num, &n2) != 1) {
                printf("Usage: insert <line_number> <text>\n");
                continue;
            }
            char *text = skip_spaces(p + n2);
            doc_insert(&doc, line_num, text);

        } else if (strcmp(cmd, "delete") == 0) {
            int line_num;
            if (sscanf(p, "%d", &line_num) != 1) {
                printf("Usage: delete <line_number>\n");
                continue;
            }
            doc_delete(&doc, line_num);

        } else if (strcmp(cmd, "display") == 0 || strcmp(cmd, "list") == 0) {
            doc_display(&doc);

        } else if (strcmp(cmd, "save") == 0) {
            if (*p == '\0') { printf("Usage: save <filename>\n"); continue; }
            doc_save(&doc, p);

        } else if (strcmp(cmd, "load") == 0) {
            if (*p == '\0') { printf("Usage: load <filename>\n"); continue; }
            doc_load(&doc, p);

        } else if (strcmp(cmd, "search") == 0) {
            if (*p == '\0') { printf("Usage: search <word>\n"); continue; }
            doc_search(&doc, p);

        } else if (strcmp(cmd, "replaceall") == 0) {
            char old_w[256], new_w[256];
            int n2 = 0;
            if (sscanf(p, "%255s%n", old_w, &n2) != 1) {
                printf("Usage: replaceall <old_word> <new_word>\n");
                continue;
            }
            char *rest = skip_spaces(p + n2);
            if (sscanf(rest, "%255s", new_w) != 1) {
                printf("Usage: replaceall <old_word> <new_word>\n");
                continue;
            }
            doc_replace_all(&doc, old_w, new_w);

        } else if (strcmp(cmd, "replace") == 0) {
            int line_num, n2 = 0;
            if (sscanf(p, "%d%n", &line_num, &n2) != 1) {
                printf("Usage: replace <line_number> <old_word> <new_word>\n");
                continue;
            }
            char *rest = skip_spaces(p + n2);
            char old_w[256], new_w[256];
            int n3 = 0;
            if (sscanf(rest, "%255s%n", old_w, &n3) != 1) {
                printf("Usage: replace <line_number> <old_word> <new_word>\n");
                continue;
            }
            rest = skip_spaces(rest + n3);
            if (sscanf(rest, "%255s", new_w) != 1) {
                printf("Usage: replace <line_number> <old_word> <new_word>\n");
                continue;
            }
            doc_replace_line(&doc, line_num, old_w, new_w);

        } else if (strcmp(cmd, "count") == 0) {
            doc_count(&doc);

        } else if (strcmp(cmd, "help") == 0) {
            print_help();

        } else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) {
            break;

        } else {
            printf("Unknown command '%s'. Type 'help' for the list of commands.\n", cmd);
        }
    }

    doc_free(&doc);
    printf("Goodbye.\n");
    return 0;
}
