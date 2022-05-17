#include "picohttpparser.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUF_LEN 8192
#define min(a, b)                                                              \
    ({                                                                         \
        __typeof__(a) _a = (a);                                                \
        __typeof__(b) _b = (b);                                                \
        _a < _b ? _a : _b;                                                     \
    })

struct status {
    int code;
    char *str;
    char *html;
};

static struct status status_200 = {
    .code = 200,
    .str = "OK",
    .html = "<html><body><h1>OK</h1><hr><address>Kerla</address>",
};

static struct status status_304 = {
    .code = 304,
    .str = "Not Modified",
    .html = "<html><body><h1>Not Modified</h1><hr><address>Kerla</address>",
};

static struct status status_404 = {
    .code = 404,
    .str = "Not Found",
    .html = "<html><body><h1>Not Found</h1><hr><address>Kerla</address>",
};

static struct status status_414 = {
    .code = 414,
    .str = "URI Too Long",
    .html = "<html><body><h1>URI Too Long</h1><hr><address>Kerla</address>",
};

static struct status status_500 = {
    .code = 500,
    .str = "Internal Server Error",
    .html =
        "<html><body><h1>Internal Server Error</h1><hr><address>Kerla</address>",
};

bool is_prefix(const char *s, const char *prefix) {
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

bool is_suffix(const char *s, const char *suffix) {
    return strncmp(s + strlen(s) - strlen(suffix), suffix, strlen(suffix)) == 0;
}

int normalize_path(const char *url_path, char *buf, size_t max_len) {
    if (max_len < 2) {
        return -1;
    }

    if (*url_path == '\0') {
        strncpy(buf, "", max_len);
        return 0;
    }

    if (*url_path != '/') {
        return -1;
    }

    // Skip the first '/'.
    url_path++;

    size_t i = 0;
    while (*url_path != '\0' && i < max_len - 1) {
        if (is_prefix(url_path, "..")) {
            if (i >= 2) {
                i -= 2;
            }

            while (buf[i] != '/' && buf[i] != '\0' && i > 0) {
                i--;
            }
            i++;
            url_path += 2;
            while (*url_path != '\0' && *url_path == '/') {
                url_path++;
            }
        } else {
            buf[i] = *url_path;
            i++;
            url_path++;
        }
    }

    if (i > 0 && buf[i - 1] == '/') {
        buf[i - 1] = '\0';
    }

    buf[i] = '\0';
    return i;
}

void reply_common_headers(int sock, const char *log_header,
                          struct status *status, const char *content_type) {
    dprintf(sock,
            "HTTP/1.1 %d %s\r\n"
            "Server: Kerla (https://kerla.dev)\r\n"
            "Content-Type: %s\r\n",
            status->code, status->str, content_type);

    time_t t = time(NULL);
    char date[32];
    strftime(date, sizeof(date), "%FT%T%z", localtime(&t));

    printf("%s%s %d\n", log_header, status->str, status->code);
}

__attribute__((noreturn)) void exit_with_http_status(int sock,
                                                     const char *log_header,
                                                     struct status *status) {
    reply_common_headers(sock, log_header, status, "text/html");
    dprintf(sock,
            "Content-Length: %lu\r\n"
            "\r\n"
            "%s",
            strlen(status->html), status->html);
    exit(1);
}

char *get_content_type(const char *path) {
    char *types[][2] = {
        {".html", "text/html"},      {".xml", "text/xml"},
        {".css", "text/css"},        {".js", "application/javascript"},
        {".jpg", "image/jpeg"},      {".png", "image/png"},
        {".mp4", "video/mp4"},       {".svg", "image/svg+xml"},
        {".pdf", "application/pdf"}, {NULL, NULL},
    };

    for (int i = 0; types[i][0] != NULL; i++) {
        if (is_suffix(path, types[i][0])) {
            return types[i][1];
        }
    }

    return "application/octet-stream";
}

FILE *open_file(int sock, const char *log_header, char *path,
                size_t path_max_len, struct stat *st, char **content_type) {
    if (strlen(path) == 0 || !strcmp(path, "/")) {
        path = "index.html";
    }

    bool ok = false;
    if (!stat(path, st)) {
        if (S_ISDIR(st->st_mode)) {
            chdir(path);
            path = "index.html";
        } else {
            ok = true;
        }
    } else {
        size_t remaining_len = path_max_len - strlen(path) - 1;
        if (remaining_len < 5) {
            exit_with_http_status(sock, log_header, &status_414);
        }

        strncat(path, ".html", remaining_len);
    }

    if (!ok) {
        if (stat(path, st) < 0) {
            exit_with_http_status(sock, log_header, &status_404);
        }
    }

    FILE *file;
    if ((file = fopen(path, "r")) == NULL) {
        exit_with_http_status(sock, log_header, &status_404);
    }

    *content_type = get_content_type(path);
    return file;
}

void serve(int sock, struct sockaddr *addr) {
    char *buf = malloc(BUF_LEN);
    if (!buf) {
        fprintf(stderr, "malloc failed\n");
        exit_with_http_status(sock, "", &status_500);
    }

    char *method, *path;
    struct phr_header headers[32];
    size_t method_len, path_len, num_headers;
    while (1) {
        int pret, minor_version;
        ssize_t rret;
        size_t buflen = 0, prevbuflen = 0;
        while ((rret = read(sock, buf + buflen, BUF_LEN - buflen)) == -1
               && errno == EINTR)
            ;
        if (rret <= 0) {
            fprintf(stderr, "parser: I/O error\n");
            dprintf(
                sock,
                "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
            return;
        }

        prevbuflen = buflen;
        buflen += rret;
        num_headers = sizeof(headers) / sizeof(headers[0]);
        pret =
            phr_parse_request(buf, buflen, (const char **) &method, &method_len,
                              (const char **) &path, &path_len, &minor_version,
                              headers, &num_headers, prevbuflen);
        if (pret > 0)
            break;
        else if (pret == -1) {
            fprintf(stderr, "parser: parse error\n");
            dprintf(sock,
                    "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
            return;
        }

        if (buflen == BUF_LEN) {
            fprintf(stderr, "parser: request too long\n");
            dprintf(sock,
                    "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
            return;
        }
    }

    method[method_len] = '\0';
    path[path_len] = '\0';

    time_t t = time(NULL);
    char date[32];
    strftime(date, sizeof(date), "%FT%T%z", localtime(&t));
    char log_header[64];
    snprintf(log_header, sizeof(log_header), "%s [%s] \"%s %s\" ",
             inet_ntoa(((struct sockaddr_in *) addr)->sin_addr), date, method,
             path);

    char resolved_path[256];
    if (normalize_path(path, (char *) &resolved_path, sizeof(resolved_path))
        < 0) {
        exit_with_http_status(sock, log_header, &status_414);
    }

    struct stat st;
    char *content_type;
    FILE *file = open_file(sock, log_header, resolved_path,
                           sizeof(resolved_path), &st, &content_type);

    // Compute etag.
    char etag[16 * 2 + 2];
    snprintf(etag, sizeof(etag), "%lx-%lx", st.st_mtime, st.st_size);

    for (size_t i = 0; i != num_headers; ++i) {
        if (!strncmp(headers[i].name, "If-None-Match", headers[i].name_len)
            && !strncmp(headers[i].value, etag,
                        min(sizeof(etag), headers[i].value_len))) {
            exit_with_http_status(sock, log_header, &status_304);
        }
    }

    reply_common_headers(sock, log_header, &status_200, content_type);
    dprintf(sock,
            "Transfer-Encoding: chunked\r\n"
            "ETag: %s\r\n"
            "\r\n",
            etag);

    while (true) {
        int read_len = read(fileno(file), buf, BUF_LEN);
        if (read_len < 0) {
            perror("read");
            break;
        }

        if (!read_len) {
            break;
        }

        dprintf(sock, "%x\r\n", read_len);

        int off = 0;
        while (off < read_len) {
            int write_len = write(sock, buf + off, read_len - off);
            if (write_len < 0) {
                perror("write");
                break;
            }

            off += write_len;
        }

        dprintf(sock, "\r\n");
    }

    dprintf(sock, "0\r\n\r\n");

    free(buf);
    shutdown(sock, SHUT_RDWR);
}

int main(int argc, char **argv) {
    int port = 0;
    int backlog = 0;
    uint32_t bind_to = INADDR_LOOPBACK;
    int opt;
    char *www_dir = "/nonexistent";
    while ((opt = getopt(argc, argv, "p:b:c:d:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'b':
                bind_to =
                    (!strcmp(optarg, "any")) ? INADDR_ANY : INADDR_LOOPBACK;
                break;
            case 'c':
                backlog = atoi(optarg);
                break;
            case 'd':
                www_dir = optarg;
                break;
        }
    }

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket(2)");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(bind_to);
    if (bind(server_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("bind(2)");
        return 1;
    }

    if (listen(server_sock, backlog) < 0) {
        perror("listen(2)");
        return 1;
    }

    signal(SIGCHLD, SIG_IGN);

    if (chdir(www_dir) < 0) {
        perror("chdir(2)");
        return 1;
    }

    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd(3)");
        return 1;
    }

    printf("bind: %s\n", (bind_to == INADDR_ANY) ? " any" : "localhost");
    printf("port: %d\n", port);
    printf("dir:  %s\n", cwd);
    printf("\n");

    while (true) {
        struct sockaddr client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sock = accept(server_sock, &client_addr, &client_addr_len);
        if (client_sock < 0) {
            switch (errno) {
                case EINTR:
                case ECONNABORTED:
                    continue;
                default:
                    perror("accept(2)");
                    return 1;
            }
        }

        int ret = fork();
        if (ret < 0) {
            perror("fork(2)");
        } else if (ret == 0) {
            serve(client_sock, &client_addr);
            exit(0);
        }

        close(client_sock);
    }

    return 0;
}
