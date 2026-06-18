#ifndef PRISM_PARSER_H
#define PRISM_PARSER_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_ELEMENTS 5000
#define MAX_ATTRIBUTES 10
#define MAX_ATTR_VALUE 256

typedef enum ElementType {
    ELEMENT_UNKNOWN,
    ELEMENT_TEXT,
    ELEMENT_LINK,
    ELEMENT_HEADING,
    ELEMENT_PARAGRAPH,
    ELEMENT_IMAGE,
    ELEMENT_LIST_ITEM,
    ELEMENT_BLOCKQUOTE,
    ELEMENT_CODE,
    ELEMENT_FORM,
    ELEMENT_INPUT,
    ELEMENT_BUTTON,
    ELEMENT_DIV,
    ELEMENT_SPAN,
    ELEMENT_SCRIPT,
    ELEMENT_STYLE
} ElementType;

typedef struct Attribute {
    char name[64];
    char value[MAX_ATTR_VALUE];
} Attribute;

typedef struct ParsedElement {
    ElementType type;
    char* content;
    char* src;
    char* href;
    char* alt;
    Attribute attributes[MAX_ATTRIBUTES];
    int attribute_count;
    int indent_level;
    bool visited;
    uint32_t* image_data;
    int image_width;
    int image_height;
} ParsedElement;

typedef struct ParsedPage {
    ParsedElement* elements;
    int element_count;
    int capacity;
    char* base_url;
} ParsedPage;

ParsedPage* parser_parse_html(const char* html, size_t length);
void parser_free_page(ParsedPage* page);
char* parser_extract_text(const char* html, size_t length);
ParsedElement* parser_get_links(ParsedPage* page, int* count);
ParsedElement* parser_get_images(ParsedPage* page, int* count);

#endif /* PRISM_PARSER_H */