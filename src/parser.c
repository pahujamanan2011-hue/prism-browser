#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define MAX_TAG_NAME 32
#define MAX_TOKEN_LENGTH 8192

typedef struct Token {
    char data[MAX_TOKEN_LENGTH];
    int length;
    bool is_tag;
    bool is_closing;
    bool is_self_closing;
    char tag_name[MAX_TAG_NAME];
    char attributes[1024];
} Token;

static bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

static char* strtrim(char* str) {
    if (!str) return NULL;
    
    while (is_whitespace(*str)) str++;
    
    if (*str == 0) return str;
    
    char* end = str + strlen(str) - 1;
    while (end > str && is_whitespace(*end)) end--;
    end[1] = '\0';
    
    return str;
}

static char* strndup_safe(const char* s, size_t n) {
    if (!s || n == 0) return NULL;
    
    size_t len = 0;
    while (len < n && s[len] != '\0') len++;
    
    char* result = malloc(len + 1);
    if (!result) return NULL;
    
    memcpy(result, s, len);
    result[len] = '\0';
    return result;
}

static bool parse_tag(Token* token) {
    char* start = token->data + 1;
    char* end = strchr(start, '>');
    
    if (!end) return false;
    
    *end = '\0';
    token->length = end - token->data + 1;
    
    char* tag_content = strndup_safe(start, end - start);
    if (!tag_content) return false;
    
    char* trimmed = strtrim(tag_content);
    
    if (trimmed[0] == '/') {
        token->is_closing = true;
        strncpy(token->tag_name, trimmed + 1, MAX_TAG_NAME - 1);
        token->tag_name[MAX_TAG_NAME - 1] = '\0';
    } else {
        token->is_closing = false;
        
        /* Find first whitespace character - handles spaces, tabs, newlines */
        char* whitespace = trimmed;
        while (*whitespace && !is_whitespace(*whitespace)) whitespace++;
        
        if (*whitespace) {
            *whitespace = '\0';
            strncpy(token->tag_name, trimmed, MAX_TAG_NAME - 1);
            token->tag_name[MAX_TAG_NAME - 1] = '\0';
            
            char* attr_start = whitespace + 1;
            char* attr_end = attr_start + strlen(attr_start) - 1;
            
            /* Check for self-closing slash */
            while (attr_end > attr_start && is_whitespace(*attr_end)) attr_end--;
            if (attr_end > attr_start && *attr_end == '/') {
                token->is_self_closing = true;
                *attr_end = '\0';
            }
            
            strncpy(token->attributes, attr_start, sizeof(token->attributes) - 1);
            token->attributes[sizeof(token->attributes) - 1] = '\0';
        } else {
            strncpy(token->tag_name, trimmed, MAX_TAG_NAME - 1);
            token->tag_name[MAX_TAG_NAME - 1] = '\0';
            token->attributes[0] = '\0';
            
            /* Check for self-closing tag */
            size_t len = strlen(token->tag_name);
            if (len > 0 && token->tag_name[len - 1] == '/') {
                token->is_self_closing = true;
                token->tag_name[len - 1] = '\0';
            }
        }
    }
    
    free(tag_content);
    return true;
}

static Token get_next_token(const char** pos) {
    Token token = {0};
    token.is_tag = false;
    token.is_closing = false;
    token.is_self_closing = false;
    
    const char* start = *pos;
    const char* current = start;
    
    while (*current && *current != '<') {
        current++;
    }
    
    if (current > start) {
        int text_len = current - start;
        strncpy(token.data, start, text_len < MAX_TOKEN_LENGTH - 1 ? text_len : MAX_TOKEN_LENGTH - 1);
        token.data[text_len] = '\0';
        token.length = text_len;
        token.is_tag = false;
        *pos = current;
        return token;
    }
    
    if (*current == '<') {
        const char* tag_end = strchr(current, '>');
        if (tag_end) {
            int tag_len = tag_end - current + 1;
            strncpy(token.data, current, tag_len < MAX_TOKEN_LENGTH - 1 ? tag_len : MAX_TOKEN_LENGTH - 1);
            token.data[tag_len] = '\0';
            token.length = tag_len;
            token.is_tag = true;
            parse_tag(&token);
            *pos = tag_end + 1;
            return token;
        }
    }
    
    *pos = current;
    return token;
}

static void add_element(ParsedPage* page, ElementType type, const char* content) {
    if (!page || page->element_count >= page->capacity) {
        if (!page) return;
        
        int new_capacity = page->capacity == 0 ? 100 : page->capacity * 2;
        ParsedElement* new_elements = realloc(page->elements, new_capacity * sizeof(ParsedElement));
        if (!new_elements) return;
        
        page->elements = new_elements;
        page->capacity = new_capacity;
    }
    
    ParsedElement* elem = &page->elements[page->element_count];
    memset(elem, 0, sizeof(ParsedElement));
    elem->type = type;
    
    if (content) {
        elem->content = strdup(content);
    }
    
    page->element_count++;
}

static void extract_attributes(const char* attr_str, ParsedElement* elem) {
    if (!attr_str || !elem) return;
    
    char* attr_copy = strdup(attr_str);
    if (!attr_copy) return;
    
    char* attr = attr_copy;
    while (*attr) {
        while (is_whitespace(*attr)) attr++;
        if (!*attr) break;
        
        char* eq = strchr(attr, '=');
        if (!eq) break;
        
        *eq = '\0';
        char* name = attr;
        char* value_start = eq + 1;
        
        while (is_whitespace(*value_start)) value_start++;
        
        char* value_end;
        if (*value_start == '"' || *value_start == '\'') {
            char quote = *value_start;
            value_start++;
            value_end = strchr(value_start, quote);
            if (value_end) {
                *value_end = '\0';
                value_end++;
            }
        } else {
            value_end = value_start;
            while (*value_end && !is_whitespace(*value_end)) value_end++;
            if (*value_end) {
                *value_end = '\0';
                value_end++;
            }
        }
        
        char* trimmed_name = strtrim(name);
        char* trimmed_value = strtrim(value_start);
        
        if (strcmp(trimmed_name, "href") == 0) {
            elem->href = strdup(trimmed_value);
        } else if (strcmp(trimmed_name, "src") == 0) {
            elem->src = strdup(trimmed_value);
        } else if (strcmp(trimmed_name, "alt") == 0) {
            elem->alt = strdup(trimmed_value);
        }
        
        if (elem->attribute_count < MAX_ATTRIBUTES) {
            strncpy(elem->attributes[elem->attribute_count].name, trimmed_name, 63);
            elem->attributes[elem->attribute_count].name[63] = '\0';
            strncpy(elem->attributes[elem->attribute_count].value, trimmed_value, MAX_ATTR_VALUE - 1);
            elem->attributes[elem->attribute_count].value[MAX_ATTR_VALUE - 1] = '\0';
            elem->attribute_count++;
        }
        
        attr = value_end;
    }
    
    free(attr_copy);
}

ParsedPage* parser_parse_html(const char* html, size_t length) {
    if (!html) return NULL;
    
    ParsedPage* page = calloc(1, sizeof(ParsedPage));
    if (!page) return NULL;
    
    page->capacity = 100;
    page->elements = calloc(page->capacity, sizeof(ParsedElement));
    if (!page->elements) {
        free(page);
        return NULL;
    }
    
    const char* pos = html;
    Token token;
    
    bool in_script = false;
    bool in_style = false;
    char text_buffer[4096] = {0};
    int text_buffer_pos = 0;
    int indent_level = 0;
    
    while (pos && *pos) {
        token = get_next_token(&pos);
        
        if (token.is_tag) {
            if (text_buffer_pos > 0) {
                text_buffer[text_buffer_pos] = '\0';
                char* trimmed = strtrim(text_buffer);
                if (strlen(trimmed) > 0 && !in_script && !in_style) {
                    add_element(page, ELEMENT_TEXT, trimmed);
                }
                text_buffer_pos = 0;
                memset(text_buffer, 0, sizeof(text_buffer));
            }
            
            if (strcmp(token.tag_name, "script") == 0) {
                in_script = !token.is_closing;
                continue;
            }
            
            if (strcmp(token.tag_name, "style") == 0) {
                in_style = !token.is_closing;
                continue;
            }
            
            if (in_script || in_style) continue;
            
            if (token.is_closing) {
                indent_level = indent_level > 0 ? indent_level - 1 : 0;
                continue;
            }
            
            ElementType type = ELEMENT_UNKNOWN;
            
            if (strcmp(token.tag_name, "a") == 0) {
                type = ELEMENT_LINK;
            } else if (strcmp(token.tag_name, "h1") == 0 || strcmp(token.tag_name, "h2") == 0 ||
                       strcmp(token.tag_name, "h3") == 0 || strcmp(token.tag_name, "h4") == 0 ||
                       strcmp(token.tag_name, "h5") == 0 || strcmp(token.tag_name, "h6") == 0) {
                type = ELEMENT_HEADING;
            } else if (strcmp(token.tag_name, "p") == 0) {
                type = ELEMENT_PARAGRAPH;
            } else if (strcmp(token.tag_name, "img") == 0) {
                type = ELEMENT_IMAGE;
            } else if (strcmp(token.tag_name, "li") == 0) {
                type = ELEMENT_LIST_ITEM;
            } else if (strcmp(token.tag_name, "blockquote") == 0) {
                type = ELEMENT_BLOCKQUOTE;
            } else if (strcmp(token.tag_name, "code") == 0) {
                type = ELEMENT_CODE;
            } else if (strcmp(token.tag_name, "form") == 0) {
                type = ELEMENT_FORM;
            } else if (strcmp(token.tag_name, "input") == 0) {
                type = ELEMENT_INPUT;
            } else if (strcmp(token.tag_name, "button") == 0) {
                type = ELEMENT_BUTTON;
            } else if (strcmp(token.tag_name, "div") == 0) {
                type = ELEMENT_DIV;
            } else if (strcmp(token.tag_name, "span") == 0) {
                type = ELEMENT_SPAN;
            }
            
            if (type != ELEMENT_UNKNOWN) {
                ParsedElement* elem = &page->elements[page->element_count];
                memset(elem, 0, sizeof(ParsedElement));
                elem->type = type;
                elem->indent_level = indent_level;
                
                extract_attributes(token.attributes, elem);
                
                page->element_count++;
                indent_level++;
            }
            
            if (token.is_self_closing) {
                indent_level = indent_level > 0 ? indent_level - 1 : 0;
            }
        } else {
            if (!in_script && !in_style && !is_whitespace(token.data[0])) {
                int remaining = sizeof(text_buffer) - text_buffer_pos - 1;
                if (remaining > 0) {
                    strncat(text_buffer, token.data, remaining);
                    text_buffer_pos += token.length;
                }
            }
        }
    }
    
    if (text_buffer_pos > 0) {
        text_buffer[text_buffer_pos] = '\0';
        char* trimmed = strtrim(text_buffer);
        if (strlen(trimmed) > 0 && !in_script && !in_style) {
            add_element(page, ELEMENT_TEXT, trimmed);
        }
    }
    
    return page;
}

void parser_free_page(ParsedPage* page) {
    if (!page) return;
    
    if (page->elements) {
        for (int i = 0; i < page->element_count; i++) {
            ParsedElement* elem = &page->elements[i];
            if (elem->content) free(elem->content);
            if (elem->href) free(elem->href);
            if (elem->src) free(elem->src);
            if (elem->alt) free(elem->alt);
            if (elem->image_data) free(elem->image_data);
        }
        free(page->elements);
    }
    
    if (page->base_url) free(page->base_url);
    free(page);
}

char* parser_extract_text(const char* html, size_t length) {
    if (!html) return NULL;
    
    ParsedPage* page = parser_parse_html(html, length);
    if (!page) return NULL;
    
    size_t total_length = 0;
    for (int i = 0; i < page->element_count; i++) {
        if (page->elements[i].content) {
            total_length += strlen(page->elements[i].content) + 1;
        }
    }
    
    char* text = malloc(total_length + 1);
    if (!text) {
        parser_free_page(page);
        return NULL;
    }
    
    text[0] = '\0';
    for (int i = 0; i < page->element_count; i++) {
        if (page->elements[i].content) {
            strcat(text, page->elements[i].content);
            strcat(text, " ");
        }
    }
    
    parser_free_page(page);
    return text;
}

ParsedElement* parser_get_links(ParsedPage* page, int* count) {
    if (!page || !count) return NULL;
    
    *count = 0;
    for (int i = 0; i < page->element_count; i++) {
        if (page->elements[i].type == ELEMENT_LINK) {
            (*count)++;
        }
    }
    
    if (*count == 0) return NULL;
    
    ParsedElement* links = malloc((*count) * sizeof(ParsedElement));
    if (!links) return NULL;
    
    int idx = 0;
    for (int i = 0; i < page->element_count; i++) {
        if (page->elements[i].type == ELEMENT_LINK) {
            links[idx++] = page->elements[i];
        }
    }
    
    return links;
}

ParsedElement* parser_get_images(ParsedPage* page, int* count) {
    if (!page || !count) return NULL;
    
    *count = 0;
    for (int i = 0; i < page->element_count; i++) {
        if (page->elements[i].type == ELEMENT_IMAGE) {
            (*count)++;
        }
    }
    
    if (*count == 0) return NULL;
    
    ParsedElement* images = malloc((*count) * sizeof(ParsedElement));
    if (!images) return NULL;
    
    int idx = 0;
    for (int i = 0; i < page->element_count; i++) {
        if (page->elements[i].type == ELEMENT_IMAGE) {
            images[idx++] = page->elements[i];
        }
    }
    
    return images;
}