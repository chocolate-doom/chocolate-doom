//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2026 Sysop-64 contributors
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//     Tiny HTTP server for live Sysop-64 mega converter tuning.
//

#include "config.h"
#include "sysop64_image.h"
#include "sysop64_backend.h"
#include "sysop64_tune_http.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#define SYSOP_TUNE_HTTP_DEFAULT_HOST "0.0.0.0"
#define SYSOP_TUNE_HTTP_DEFAULT_PORT 6464
#define SYSOP_TUNE_HTTP_REQUEST_MAX 8192

static int g_tune_http_enabled = 0;
static char g_tune_http_host[64] = SYSOP_TUNE_HTTP_DEFAULT_HOST;
static int g_tune_http_port = SYSOP_TUNE_HTTP_DEFAULT_PORT;
static pthread_t g_tune_http_thread;
static volatile int g_tune_http_running = 0;
static int g_tune_http_started = 0;
static int g_tune_http_listen_fd = -1;

static char *g_tune_http_page = NULL;
static size_t g_tune_http_page_len = 0;
static char g_tune_http_page_path[PATH_MAX];
static int g_tune_http_page_reported = 0;

// Release the cached HTML tuner page so a future start or request can reload it.
static void unload_tune_http_page(void)
{
    free(g_tune_http_page);
    g_tune_http_page = NULL;
    g_tune_http_page_len = 0;
    g_tune_http_page_path[0] = '\0';
    g_tune_http_page_reported = 0;
}

// Load the standalone tuner HTML file into memory for HTTP responses.
static int read_tune_http_page_file(const char *path)
{
    FILE *fp;
    long file_len;
    char *buffer;
    size_t got;

    if (path == NULL || path[0] == '\0') {
        return 0;
    }

    fp = fopen(path, "rb");
    if (fp == NULL) {
        return 0;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }

    file_len = ftell(fp);
    if (file_len < 0) {
        fclose(fp);
        return 0;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }

    buffer = (char *)malloc((size_t)file_len + 1);
    if (buffer == NULL) {
        fclose(fp);
        return 0;
    }

    got = fread(buffer, 1, (size_t)file_len, fp);
    fclose(fp);

    if (got != (size_t)file_len) {
        free(buffer);
        return 0;
    }

    buffer[got] = '\0';
    unload_tune_http_page();
    g_tune_http_page = buffer;
    g_tune_http_page_len = got;
    snprintf(g_tune_http_page_path, sizeof(g_tune_http_page_path), "%s", path);
    return 1;
}

// Try one unique HTML page path while avoiding repeated file probes.
static int try_tune_http_page_candidate(char candidates[][PATH_MAX], int *count, const char *path)
{
    int i;

    if (path == NULL || path[0] == '\0' || *count >= 8) {
        return 0;
    }

    for (i = 0; i < *count; ++i) {
        if (!strcmp(candidates[i], path)) {
            return 0;
        }
    }

    snprintf(candidates[*count], PATH_MAX, "%s", path);
    ++(*count);
    return read_tune_http_page_file(path);
}

// Locate and cache sysop64_tune_http.html from an environment override,
// working tree path, or executable-relative deployment path.
static int load_tune_http_page(int force_reload)
{
    char candidates[8][PATH_MAX];
    int count = 0;
    const char *env_path = getenv("SYSOP_TUNE_HTTP_PAGE");
    char exe_path[PATH_MAX];
    ssize_t exe_len;

    if (g_tune_http_page != NULL && !force_reload) {
        return 1;
    }

    if (try_tune_http_page_candidate(candidates, &count, env_path) ||
        try_tune_http_page_candidate(candidates, &count, "./sysop64_tune_http.html") ||
        try_tune_http_page_candidate(candidates, &count, "./src/sysop64/sysop64_tune_http.html")) {
        return 1;
    }

    exe_len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (exe_len > 0) {
        char *slash;
        exe_path[exe_len] = '\0';
        slash = strrchr(exe_path, '/');
        if (slash != NULL) {
            char candidate[PATH_MAX];
            *slash = '\0';
            snprintf(candidate, sizeof(candidate), "%s/sysop64_tune_http.html", exe_path);
            if (try_tune_http_page_candidate(candidates, &count, candidate)) {
                return 1;
            }
            snprintf(candidate, sizeof(candidate), "%s/src/sysop64/sysop64_tune_http.html", exe_path);
            if (try_tune_http_page_candidate(candidates, &count, candidate)) {
                return 1;
            }
        }
    }

    fprintf(stderr,
            "Sysop mega HTTP: could not load sysop64_tune_http.html. "
            "Set SYSOP_TUNE_HTTP_PAGE or keep the file beside the executable or in src/sysop64.\n");
    return 0;
}


// Parse a TCP port, falling back to the default tuner port on invalid input.
static int parse_port(const char *text)
{
    int port = atoi(text);

    if (port <= 0 || port > 65535) {
        port = SYSOP_TUNE_HTTP_DEFAULT_PORT;
    }

    return port;
}

// Configure the HTTP tuner from command line text such as on, off, port, or
// host:port.
void Sysop_TuneHttpConfigure(const char *spec)
{
    char buffer[128];
    char *colon;

    g_tune_http_enabled = 1;
    snprintf(g_tune_http_host, sizeof(g_tune_http_host), "%s", SYSOP_TUNE_HTTP_DEFAULT_HOST);
    g_tune_http_port = SYSOP_TUNE_HTTP_DEFAULT_PORT;

    if (spec == NULL || spec[0] == '\0' || !strcasecmp(spec, "on")) {
        return;
    }

    if (!strcasecmp(spec, "off") || !strcasecmp(spec, "none") || !strcasecmp(spec, "false")) {
        g_tune_http_enabled = 0;
        return;
    }

    snprintf(buffer, sizeof(buffer), "%s", spec);
    colon = strrchr(buffer, ':');

    if (colon != NULL) {
        *colon = '\0';
        snprintf(g_tune_http_host, sizeof(g_tune_http_host), "%s", buffer[0] ? buffer : SYSOP_TUNE_HTTP_DEFAULT_HOST);
        g_tune_http_port = parse_port(colon + 1);
    } else if (buffer[0] >= '0' && buffer[0] <= '9') {
        g_tune_http_port = parse_port(buffer);
    } else {
        snprintf(g_tune_http_host, sizeof(g_tune_http_host), "%s", buffer);
    }
}

// Report whether command line parsing enabled the HTTP tuner.
int Sysop_TuneHttpEnabled(void)
{
    return g_tune_http_enabled;
}

// Send a complete response buffer unless the client disconnects or errors.
static void send_all(int fd, const char *data, size_t len)
{
    while (len > 0) {
        ssize_t sent = send(fd, data, len, 0);
        if (sent <= 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        data += sent;
        len -= (size_t)sent;
    }
}

// Send an HTTP response with an explicit body length.
static void send_response_len(int fd, const char *status, const char *type, const char *body, size_t body_len)
{
    char header[256];
    int header_len = snprintf(header, sizeof(header),
                              "HTTP/1.1 %s\r\n"
                              "Content-Type: %s\r\n"
                              "Content-Length: %u\r\n"
                              "Connection: close\r\n"
                              "Cache-Control: no-store\r\n"
                              "\r\n",
                              status, type, (unsigned)body_len);

    if (header_len > 0) {
        send_all(fd, header, (size_t)header_len);
    }
    send_all(fd, body, body_len);
}

// Send a null-terminated HTTP response body.
static void send_response(int fd, const char *status, const char *type, const char *body)
{
    send_response_len(fd, status, type, body, strlen(body));
}

// Convert one URL-escape hexadecimal digit into a numeric value.
static int hex_value(int c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// Decode application/x-www-form-urlencoded text in place.
static void url_decode_in_place(char *text)
{
    char *src = text;
    char *dst = text;

    while (*src) {
        if (*src == '+') {
            *dst++ = ' ';
            ++src;
        } else if (*src == '%' && hex_value(src[1]) >= 0 && hex_value(src[2]) >= 0) {
            *dst++ = (char)((hex_value(src[1]) << 4) | hex_value(src[2]));
            src += 3;
        } else {
            *dst++ = *src++;
        }
    }

    *dst = '\0';
}

// Parse HTTP tuner boolean fields, including adaptive/static palette aliases.
static int parse_on_off(const char *value, int default_value)
{
    if (value == NULL || value[0] == '\0') {
        return default_value;
    }

    if (!strcasecmp(value, "1") || !strcasecmp(value, "on")
        || !strcasecmp(value, "yes") || !strcasecmp(value, "true")
        || !strcasecmp(value, "adaptive")) {
        return 1;
    }

    if (!strcasecmp(value, "0") || !strcasecmp(value, "off")
        || !strcasecmp(value, "no") || !strcasecmp(value, "false")
        || !strcasecmp(value, "static") || !strcasecmp(value, "fixed")) {
        return 0;
    }

    return atoi(value) != 0;
}

// Apply one tuner option, routing adaptive palette controls to the backend and
// mega converter options to the image module.
static void apply_tune_option(const char *name, const char *value)
{
    if (!strcasecmp(name, "adaptive_palette")
        || !strcasecmp(name, "palette_adaptive")
        || !strcasecmp(name, "sysop_palette_adaptive")
        || !strcasecmp(name, "sysop-palette-adaptive")) {
        Sysop_SetAdaptivePaletteEnabled(parse_on_off(value, 1));
        return;
    }

    if (!strcasecmp(name, "sysop_palette")
        || !strcasecmp(name, "sysop-palette")) {
        if (!strcasecmp(value, "adaptive")) {
            Sysop_SetAdaptivePaletteEnabled(1);
        } else if (!strcasecmp(value, "static") || !strcasecmp(value, "fixed")
                   || !strcasecmp(value, "default") || !strcasecmp(value, "off")) {
            Sysop_SetAdaptivePaletteEnabled(0);
        }
        return;
    }

    Sysop_ImageMegaSetOption(name, value);
}

// Build the JSON state object returned to the tuner, including backend-only
// adaptive palette state.
static void write_state_json(char *json, size_t json_size)
{
    char *end;
    size_t used;
    int adaptive_palette_enabled;

    if (json == NULL || json_size == 0) {
        return;
    }

    Sysop_ImageMegaWriteStateJson(json, (int)json_size);
    json[json_size - 1] = '\0';

    end = strrchr(json, '}');
    if (end == NULL) {
        snprintf(json, json_size, "{}");
        end = strrchr(json, '}');
        if (end == NULL) {
            return;
        }
    }

    adaptive_palette_enabled = Sysop_AdaptivePaletteEnabled();
    used = (size_t)(end - json);
    snprintf(end, json_size - used,
             "%s\"adaptive_palette\":\"%s\",\"adaptive_palette_enabled\":%d}",
             end > json + 1 ? "," : "",
             adaptive_palette_enabled ? "on" : "off",
             adaptive_palette_enabled);
}

// Parse and apply key=value pairs from query strings or POST bodies.
static void apply_urlencoded_pairs(char *pairs)
{
    char *cursor = pairs;

    while (cursor != NULL && *cursor) {
        char *next = strchr(cursor, '&');
        char *equals;

        if (next != NULL) {
            *next++ = '\0';
        }

        equals = strchr(cursor, '=');
        if (equals != NULL) {
            *equals++ = '\0';
            url_decode_in_place(cursor);
            url_decode_in_place(equals);
            apply_tune_option(cursor, equals);
        }

        cursor = next;
    }
}

// Extract Content-Length from a small HTTP request header block.
static int content_length_from_request(const char *request)
{
    const char *p = request;

    while ((p = strstr(p, "\n")) != NULL) {
        ++p;
        if (!strncasecmp(p, "Content-Length:", 15)) {
            return atoi(p + 15);
        }
        if (p[0] == '\r' || p[0] == '\n') {
            break;
        }
    }

    return 0;
}

// Handle one simple HTTP client request and close the connection afterward.
static void handle_client(int fd)
{
    char request[SYSOP_TUNE_HTTP_REQUEST_MAX + 1];
    ssize_t total = 0;
    char *headers_end;
    char *method;
    char *path;
    char *version;

    for (;;) {
        ssize_t got = recv(fd, request + total, SYSOP_TUNE_HTTP_REQUEST_MAX - (size_t)total, 0);
        if (got <= 0) {
            break;
        }

        total += got;
        request[total] = '\0';
        headers_end = strstr(request, "\r\n\r\n");

        if (headers_end != NULL) {
            int content_length = content_length_from_request(request);
            int header_bytes = (int)(headers_end + 4 - request);
            if (total >= header_bytes + content_length || total >= SYSOP_TUNE_HTTP_REQUEST_MAX) {
                break;
            }
        }

        if (total >= SYSOP_TUNE_HTTP_REQUEST_MAX) {
            break;
        }
    }

    request[total] = '\0';
    method = request;
    path = strchr(request, ' ');
    if (path == NULL) {
        send_response(fd, "400 Bad Request", "text/plain", "Bad request\n");
        return;
    }

    *path++ = '\0';
    version = strchr(path, ' ');
    if (version == NULL) {
        send_response(fd, "400 Bad Request", "text/plain", "Bad request\n");
        return;
    }
    *version = '\0';

    if (!strcmp(method, "GET") && (!strcmp(path, "/") || !strncmp(path, "/?", 2))) {
        load_tune_http_page(1);
        if (g_tune_http_page != NULL) {
            send_response_len(fd, "200 OK", "text/html; charset=utf-8", g_tune_http_page, g_tune_http_page_len);
        } else {
            send_response(fd, "500 Internal Server Error", "text/plain", "sysop64_tune_http.html is not loaded. Set SYSOP_TUNE_HTTP_PAGE or keep the file beside the executable or in src/sysop64.\n");
        }
    } else if (!strcmp(method, "GET") && !strcmp(path, "/state")) {
        char json[4096];
        write_state_json(json, sizeof(json));
        send_response(fd, "200 OK", "application/json", json);
    } else if (!strcmp(method, "GET") && !strncmp(path, "/set?", 5)) {
        char query[1024];
        char json[4096];
        snprintf(query, sizeof(query), "%s", path + 5);
        apply_urlencoded_pairs(query);
        write_state_json(json, sizeof(json));
        send_response(fd, "200 OK", "application/json", json);
    } else if (!strcmp(method, "GET") && !strcmp(path, "/reset")) {
        char json[4096];
        Sysop_ImageMegaResetOptions();
        Sysop_SetAdaptivePaletteEnabled(0);
        write_state_json(json, sizeof(json));
        send_response(fd, "200 OK", "application/json", json);
    } else if (!strcmp(method, "POST") && !strcmp(path, "/state")) {
        char *body = strstr(request, "\r\n\r\n");
        char json[4096];

        if (body != NULL) {
            body += 4;
            apply_urlencoded_pairs(body);
        }

        write_state_json(json, sizeof(json));
        send_response(fd, "200 OK", "application/json", json);
    } else if (!strcmp(method, "GET") && !strcmp(path, "/favicon.ico")) {
        send_response(fd, "404 Not Found", "text/plain", "Not found\n");
    } else {
        send_response(fd, "404 Not Found", "text/plain", "Not found\n");
    }
}

// Listener thread for the minimal single-request-at-a-time tuner HTTP server.
static void *tune_http_thread_func(void *arg)
{
    struct sockaddr_in addr;
    int yes = 1;
    int flags;
    (void)arg;

    g_tune_http_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (g_tune_http_listen_fd < 0) {
        perror("Sysop mega HTTP socket");
        g_tune_http_running = 0;
        return NULL;
    }

    setsockopt(g_tune_http_listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    flags = fcntl(g_tune_http_listen_fd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(g_tune_http_listen_fd, F_SETFL, flags | O_NONBLOCK);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)g_tune_http_port);

    if (!strcmp(g_tune_http_host, "*") || !strcmp(g_tune_http_host, "0.0.0.0")) {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else if (inet_aton(g_tune_http_host, &addr.sin_addr) == 0) {
        fprintf(stderr, "Sysop mega HTTP: invalid host %s\n", g_tune_http_host);
        close(g_tune_http_listen_fd);
        g_tune_http_listen_fd = -1;
        g_tune_http_running = 0;
        return NULL;
    }

    if (bind(g_tune_http_listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Sysop mega HTTP bind");
        close(g_tune_http_listen_fd);
        g_tune_http_listen_fd = -1;
        g_tune_http_running = 0;
        return NULL;
    }

    if (listen(g_tune_http_listen_fd, 4) < 0) {
        perror("Sysop mega HTTP listen");
        close(g_tune_http_listen_fd);
        g_tune_http_listen_fd = -1;
        g_tune_http_running = 0;
        return NULL;
    }

    printf("Sysop mega HTTP tuner listening on http://%s:%d/\n",
           g_tune_http_host, g_tune_http_port);

    while (g_tune_http_running) {
        int client = accept(g_tune_http_listen_fd, NULL, NULL);

        if (client < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                usleep(20000);
                continue;
            }
            break;
        }

        handle_client(client);
        close(client);
    }

    if (g_tune_http_listen_fd >= 0) {
        close(g_tune_http_listen_fd);
        g_tune_http_listen_fd = -1;
    }

    g_tune_http_running = 0;
    return NULL;
}

// Start the HTTP tuner thread after ensuring the external HTML page is present.
void Sysop_TuneHttpStart(void)
{
    if (!g_tune_http_enabled || g_tune_http_started) {
        return;
    }

    if (!load_tune_http_page(0)) {
        return;
    }

    if (!g_tune_http_page_reported) {
        printf("Sysop mega HTTP tuner page: %s\n", g_tune_http_page_path);
        g_tune_http_page_reported = 1;
    }

    g_tune_http_running = 1;
    if (pthread_create(&g_tune_http_thread, NULL, tune_http_thread_func, NULL) != 0) {
        perror("Sysop mega HTTP pthread_create");
        g_tune_http_running = 0;
        return;
    }

    g_tune_http_started = 1;
}

// Stop the HTTP tuner thread, close the listener socket, and unload the page.
void Sysop_TuneHttpStop(void)
{
    if (!g_tune_http_started) {
        return;
    }

    g_tune_http_running = 0;
    if (g_tune_http_listen_fd >= 0) {
        shutdown(g_tune_http_listen_fd, SHUT_RDWR);
        close(g_tune_http_listen_fd);
        g_tune_http_listen_fd = -1;
    }

    pthread_join(g_tune_http_thread, NULL);
    g_tune_http_started = 0;
    unload_tune_http_page();
}
