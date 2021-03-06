#ifndef _TEST_H
#define _TEST_H

#define fail_if(expr) \
   do { \
        if ( expr ) \
          { \
             FILE *fp; \
             char buf[1024]; \
             sprintf(buf, "fail_%s.txt", elm_win_title_get(win)); \
             if ((fp = fopen(buf, "a")) != NULL) \
               { \
                  fprintf(fp, "Failed at %s:%d on <%s>\n", \
                        __FILE__, __LINE__, #expr); \
                  fclose(fp); \
               } \
          } \
   } while(0)
#endif

extern int _log_domain;

#define CRITICAL(...) EINA_LOG_DOM_CRIT(_log_domain, _VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_log_domain, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_log_domain, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_log_domain, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(_log_domain, __VA_ARGS__)
