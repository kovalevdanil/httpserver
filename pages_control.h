#ifndef _PAGES_CONTROL_H

#define _PAGES_CONTROL_H

#define ROUTES_STD_FILENAME "routes"

struct page
{
    int content_len;
    char *filename;
    char *url;
    char *content;

    struct page *next;
};

typedef struct page page_t;

struct pages
{
    int n_pages;
    page_t *first_page;
    page_t *last_page;
};


typedef struct pages pages_t;

pages_t *pages_create();
int pages_init(pages_t *pages, char *routes_filename);

page_t *pages_get_by_url(pages_t *pages, char *url);

#endif