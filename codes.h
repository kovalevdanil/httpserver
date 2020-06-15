#ifndef _CODES_H

#define _CODES_H


#define OK 200
#define BAD_REQUEST 400
#define NOT_FOUND 404

#define RESPOND "HTTP/1.1 %d OK\n"                         \
                "Content-Type: text/html; charset=UTF-8\n" \
                "Content-Length: %d\n\n%s"

#define RESPOND_BAD_REQUEST "HTTP/1.1 404 Not Found\nContent-type text/plain\n\nPage is not found"

#endif