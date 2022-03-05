/**
 * @file main.c
 * @author Florian W
 * @brief Get graphite metrics in a usable csv format
 * @version 0.1
 * @date 2022-02-20
 *
 * https://en.wikipedia.org/wiki/Box-drawing_character
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include <regex.h>
#include <time.h>

#define GRAPHITE_IP "192.168.8.42:81"
#define TIME_ZONE "CET"

// #define DEBUG

struct url_data
{
    size_t size;
    char *data;
};

size_t write_data(void *ptr, size_t size, size_t nmemb, struct url_data *data)
{
    size_t index = data->size;
    size_t n = (size * nmemb);
    char *tmp;

    data->size += (size * nmemb);

    tmp = realloc(data->data, data->size + 1); /* +1 for '\0' */

    if (tmp)
    {
        data->data = tmp;
    }
    else
    {
        if (data->data)
        {
            free(data->data);
        }
        fprintf(stderr, "Failed to allocate memory.\n");
        return 0;
    }

    memcpy((data->data + index), ptr, n);
    data->data[data->size] = '\0';

    return size * nmemb;
}

char *handle_url(char *url)
{
    CURL *curl;

    struct url_data data;
    data.size = 0;
    data.data = malloc(10000); /* reasonable size initial buffer */
    if (NULL == data.data)
    {
        fprintf(stderr, "Failed to allocate memory.\n");
        return NULL;
    }

    data.data[0] = '\0';

    CURLcode res;

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }
    return data.data;
}

int check_index(int index)
{
    if (index > 18 || index < 1)
    {
        puts("Ungültig!");
        return 0;
    }
    return 1;
}

int use_regex(char *text_to_check)
{
    regex_t compiled_regex;
    int reti;
    int return_value = -1;

    /* Compile regular expression */
    reti = regcomp(&compiled_regex, "[0-9]+(d|s|min|w|mon|y|h)", REG_EXTENDED);

    if (reti)
    {
        return -2;
    }

    /* Execute compiled regular expression */
    reti = regexec(&compiled_regex, text_to_check, 0, NULL, 0);
    if (!reti)
    {
        return_value = 1;
    }
    else if (reti == REG_NOMATCH)
    {
        puts("Falsches Format!");
        return_value = 0;
    }
    else
    {
        return_value = -3;
    }

    /* Free memory allocated to the pattern buffer by regcomp() */
    regfree(&compiled_regex);
    return return_value;
}

char *to_datetime(time_t unix_time)
{
    char *buf = malloc(100);
    struct tm ts;

    ts = *localtime(&unix_time);
    strftime(buf, 100, "%d.%m.%Y %H:%M:%S", &ts);

    return buf;
}

struct METRIC
{
    char *name;
    time_t start_time;
    time_t end_time;
    size_t interval;
    char *data_points;
};

int main(void)
{
    int metric_index;
    char time_range[16];
    char selected_metric[128];

    /* display available metrics to download */
    printf("╔═══════════════════════╗%s", "\t╔═══════════════════════╗\r\n");
    printf("║ \tSMARTMETER\t║%s", "\t║ \tWETTERSTATION\t║\r\n");
    printf("╠════╦══════════════════╣%s", "\t╠════╦══════════════════╣\r\n");
    printf("║  1 ║ Wirkenergie ges.\t║%s", "\t║ 12 ║ Temperatur\t║\r\n");
    printf("║  2 ║ Momentanleistung\t║%s", "\t║ 13 ║ Feuchte\t\t║\r\n");
    printf("║  3 ║ Leistungsfaktor\t║%s", "\t║ 14 ║ Druck\t\t║\r\n");
    printf("║  4 ║ Spannung L1\t║%s", "\t║ 15 ║ Licht\t\t║\r\n");
    printf("║  5 ║ Spannung L2\t║%s", "\t║ 16 ║ Wind\t\t║\r\n");
    printf("║  6 ║ Spannung L3\t║%s", "\t║ 17 ║ Niederschlag\t║\r\n");
    printf("║  7 ║ Strom L1\t\t║%s", "\t║    ║ \t\t\t║\r\n");
    printf("║  8 ║ Strom L2\t\t║%s", "\t║    ║ \t\t\t║\r\n");
    printf("║  9 ║ Strom L3\t\t║%s", "\t║    ║ \t\t\t║\r\n");
    printf("║ 10 ║ WiFi RSSI\t║%s", "\t║    ║ \t\t\t║\r\n");
    printf("╠════╬══════════════════╣%s", "\t╠════╬══════════════════╣\r\n");
    printf("║ 11 ║ Alle\t\t║%s", "\t║ 18 ║ Alle\t\t║\r\n");
    printf("╚════╩══════════════════╝%s", "\t╚════╩══════════════════╝\r\n");
    puts("Daten zum Herunterladen wählen (z.B. \"2\"):");

    /* get metric to download from user */
    do
    {
        scanf("%d", &metric_index);
    } while (!check_index(metric_index));

    /* set selected metric string */
    switch (metric_index)
    {
    case 1:
        snprintf(selected_metric, 128, "smartmeter.wirkenergie.plus");
        break;
    case 2:
        snprintf(selected_metric, 128, "smartmeter.momentanleistung.plus");
        break;
    case 3:
        snprintf(selected_metric, 128, "smartmeter.cosphi");
        break;
    case 4:
        snprintf(selected_metric, 128, "smartmeter.spannung.L1");
        break;
    case 5:
        snprintf(selected_metric, 128, "smartmeter.spannung.L2");
        break;
    case 6:
        snprintf(selected_metric, 128, "smartmeter.spannung.L3");
        break;
    case 7:
        snprintf(selected_metric, 128, "smartmeter.strom.L1");
        break;
    case 8:
        snprintf(selected_metric, 128, "smartmeter.strom.L2");
        break;
    case 9:
        snprintf(selected_metric, 128, "smartmeter.strom.L3");
        break;
    case 10:
        snprintf(selected_metric, 128, "test.data.rssi");
        break;
    case 11:
        snprintf(selected_metric, 128, "smartmeter.*.*&target=smartmeter.*&target=test.data.r*");
        break;
    case 12:
        snprintf(selected_metric, 128, "wetter.temperatur.*");
        break;
    case 13:
        snprintf(selected_metric, 128, "wetter.feuchte.*");
        break;
    case 14:
        snprintf(selected_metric, 128, "wetter.druck.*");
        break;
    case 15:
        snprintf(selected_metric, 128, "wetter.licht.*");
        break;
    case 16:
        snprintf(selected_metric, 128, "wetter.wind.*");
        break;
    case 17:
        snprintf(selected_metric, 128, "wetter.niederschlag.*");
        break;
    case 18:
        snprintf(selected_metric, 128, "wetter.*.*");
        break;
    }
    printf("Ausgewählt: %s\r\n", selected_metric);

    /* display time range formatting hint */
    puts("╔═══════════════════════╗");
    puts("║ Zeitformat\t\t║");
    puts("╠═══════╦═══════════════╣");
    puts("║ s\t║ Sekunden\t║");
    puts("║ min\t║ Minuten\t║");
    puts("║ h\t║ Stunden\t║");
    puts("║ d\t║ Tage\t\t║");
    puts("║ w\t║ Wochen\t║");
    puts("║ mon\t║ Monate (30d)\t║");
    puts("║ y\t║ Jahre (365d)\t║");
    puts("╚═══════╩═══════════════╝");
    puts("Zeitspanne wählen (z.B. \"30d\"):");

    /* get time span from user and check with regex */
    do
    {
        scanf("%s", &time_range);
    } while (!use_regex(time_range));

    puts("Lade Daten...");

    char graphite_url[1024];
    snprintf(graphite_url, 1024, "http://%s/render/?target=%s&tz=%s&from=-%s&format=raw", GRAPHITE_IP, selected_metric, TIME_ZONE, time_range);

#ifdef DEBUG
    printf("%s\n", graphite_url);
#endif

    /* Open url with curl and get data */
    char *data;
    data = handle_url(graphite_url);

    if (data)
    {
        char *saveptr;   // save pointer for strtok
        char *lines[99]; // max 99 metrics
        int metric_cnt = 0;

#ifdef DEBUG
        FILE *debug_file;
        debug_file = fopen("debug.txt", "w+");
        fprintf(debug_file, "%s\n\n%s\n\n\n", graphite_url, data);
#endif

        /* each raw metric is split by a LF */
        char *p;
        p = strtok_r(data, "\r\n", &saveptr);
        while (p != NULL)
        {
            // store each raw metric
            lines[metric_cnt] = p;
#ifdef DEBUG
            fprintf(debug_file, "%s\n", p);
#endif
            p = strtok_r(NULL, "\r\n", &saveptr); // get next token
            metric_cnt += 1;
        }

        /* build metric struct */
        struct METRIC metrics[metric_cnt];

        FILE *file;
        char filename[100];
        snprintf(filename, 100, "DATA%lu.csv", (int)time(NULL));
        file = fopen(filename, "w+");
        // fprintf(file, "time");

        for (uint8_t i = 0; i < metric_cnt; i++)
        {
            free(saveptr);
            /* save all parameters to a char* */
            char *_name = strtok_r(lines[i], ",", &saveptr);
            char *_start = strtok_r(NULL, ",", &saveptr);
            char *_end = strtok_r(NULL, ",", &saveptr);
            char *_interval = strtok_r(NULL, "|", &saveptr);
            char *_data = strtok_r(NULL, "", &saveptr);

            /* convert the strings to their correct type and store them in the struct */
            metrics[i].name = _name;
            sscanf(_start, "%zu", &metrics[i].start_time);
            sscanf(_end, "%zu", &metrics[i].end_time);
            sscanf(_interval, "%zu", &metrics[i].interval);
            metrics[i].data_points = _data;

            // fprintf(file, ";%s", _name);
            fprintf(file, "time_%s;%s;", _name, _name);
        }
        fprintf(file, "\r\n");

        /* now we split each datapoint and write it to the file */
        char *saveptrL[metric_cnt]; // save pointer for each metric
        char *pL[metric_cnt];
        size_t row = 0;

        for (int i = 0; i < metric_cnt; i++)
        {
            pL[i] = strtok_r(metrics[i].data_points, ",", &saveptrL[i]);
        }

        while (pL[0] != NULL)
        {
            // fprintf(file, "%s", to_datetime(metrics[0].start_time + row * metrics[0].interval));

            for (int i = 0; i < metric_cnt; i++)
            {
                // fprintf(file, ";%s", pL[i]);
                if (pL[i] != NULL)
                {
                    fprintf(file, "%s;%s;", to_datetime(metrics[i].start_time + row * metrics[i].interval), pL[i]);
                }
                else
                {
                    fprintf(file, ";;");
                }
                pL[i] = strtok_r(NULL, ",", &saveptrL[i]);
            }

            fprintf(file, "\r\n");

            row += 1;
        }
        free(data);
        fclose(file);
        puts("Fertig!");

        char cwd[100];
        getcwd(cwd, sizeof(cwd));
        printf("Datei: %s/%s\r\n", cwd, filename);
        puts("Drücke ENTER um zu beenden...");
        getch();
        getch();
    }

    return 0;
}