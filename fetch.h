#include <curl/curl.h>

/** Several sites doesn't allow clients except browsers, in that case */
static char *user_agent = "Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/109.0";

static void yawn_get_file(char *url, char *file)
{
    CURLcode ret;
    CURL *curl;
    FILE *fp;
    
    curl = curl_easy_init();
    if (curl == NULL) {
        perror("curl_easy_init()");
        exit(EXIT_FAILURE);
    }
    
    fp = fopen(file, "wb");
    if (fp == NULL) {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    ret = curl_easy_perform(curl);

    if (ret != CURLE_OK) {
        perror("curl_easy_perform()");
        exit(EXIT_FAILURE);
    }

    curl_easy_cleanup(curl);
    fclose(fp); /* No checks for errors */
}
