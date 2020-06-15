#define _GNU_SOURCE

#include "pages_control.h"

#include <malloc.h>
#include <string.h>

#include <sys/stat.h>

ssize_t get_file_size(const char *filename)
{
    if (filename == NULL)
        return -1;

    struct stat st;
    stat(filename, &st);

    return st.st_size;
}

void trim_str(char *str)
{
    int len = strlen(str);
    if (len == 0)
        return;

    int ind = len - 1;
    while (ind >= 0 && (str[ind] == '\n' || str[ind] == ' ' || str[ind] == '\t'))
        str[ind--] = '\0';
}


void pages_insert(pages_t *pages, page_t *page)
{
    if (pages == NULL || page == NULL)
        return;

    if (pages->first_page == NULL)
    {
        pages->first_page = page;
        pages->last_page = page;
    }
    else
    {
        pages->last_page->next = page;
        pages->last_page = page;
    }

    pages->n_pages++;
}

// TODO Если разные url указывают на одинаковый html файл, не считывать файл еще раз
// TODO Валидация файла routes
int pages_init(pages_t *pages, char *routes_filename)
{
    FILE *routes = fopen(routes_filename == NULL ? ROUTES_STD_FILENAME : routes_filename, "r");
    if (routes == NULL)
        return -1;

    pages->n_pages = 0;
    char *line = NULL;
    size_t line_len = 0;
    ssize_t str_len = 0;

    // 1. read line froum routes
    // 2. allocate memory for page_t
    // 3. fill the page structure (name of html file, url, load content)
    // 4. add the page to page list
    while (getline(&line, &line_len, routes) != 0)
    {
        if (line == NULL)
            break;

        trim_str(line);
        str_len = strlen(line);

        if (str_len == 0)
            break;

        page_t *page = calloc(1, sizeof(page_t));

        int colon_pos = strstr(line, ":") - line;
        int url_len = colon_pos;
        int filename_len = str_len - url_len - 1;

        page->filename = calloc(1, filename_len + 1);
        page->url = calloc(1, url_len + 1);

        strncpy(page->url, line, url_len);
        strncpy(page->filename, line + colon_pos + 1, filename_len);

        FILE *content_file = fopen(page->filename, "r");

        if (content_file == NULL) // TODO сделать адекватную обработку ошибки
        {
            fclose(routes);
            return -1;
        }

        page->content_len = get_file_size(page->filename);
        page->content = calloc(page->content_len + 1, sizeof(char));
        fread(page->content, sizeof(char), page->content_len, content_file);
        page->next = NULL;

        // insert page into list
        pages_insert(pages, page);

        free(line);
        line = NULL;
        line_len = 0;
    }

    fclose(routes);

    return 0;
}

pages_t *pages_create()
{
    return (pages_t *)calloc(1, sizeof(pages_t));
}

page_t *pages_get_by_url(pages_t *pages, char *url)
{
    if (pages == NULL || url == NULL)
        return NULL;

    page_t *current_page = pages -> first_page;

    while (current_page != NULL)
    {
        if (strcmp(url, current_page -> url) == 0)
            return current_page;
        current_page = current_page -> next;
    } 

    return NULL;
}
