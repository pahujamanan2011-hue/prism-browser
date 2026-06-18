#include "blackhole.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

static struct {
    char** domains;
    int count;
    int capacity;
    int block_count;
    time_t last_refresh;
    bool initialized;
} blackhole_engine = {0};

static const char* default_blackhole_domains[] = {
    "doubleclick.net",
    "googleadservices.com",
    "google-analytics.com",
    "googletagmanager.com",
    "facebook.com/tr",
    "facebook.net",
    "fbcdn.net",
    "amazon-adsystem.com",
    "amazonadsystem.com",
    "adnxs.com",
    "adservice.google.com",
    "adsrvr.org",
    "adzerk.net",
    "adroll.com",
    "adform.net",
    "criteo.com",
    "taboola.com",
    "outbrain.com",
    "revcontent.com",
    "mgid.com",
    "popads.net",
    "popcash.net",
    "propellerads.com",
    "exoclick.com",
    "juicyads.com",
    "trafficjunky.net",
    "adblade.com",
    "adbrite.com",
    "addthis.com",
    "adition.com",
    "adobe.com/analytics",
    "adsafeprotected.com",
    "adscoop.com",
    "adserver.com",
    "adtech.com",
    "advertising.com",
    "advertise.com",
    "affiliate.com",
    "alexametrics.com",
    "analytics.yahoo.com",
    "apache.org",
    "api.google.com",
    "atdmt.com",
    "bidswitch.net",
    "bluesky.com",
    "brave.com",
    "bugsnag.com",
    "chango.com",
    "click.net",
    "cloudflare.com",
    "cmail.com",
    "cnn.com",
    "comscore.com",
    "conversion.com",
    "criteo.com",
    "doubleclick.com",
    "dynatrace.com",
    "ecommerce.com",
    "etracker.com",
    "exelate.com",
    "facebook.com",
    "fastclick.net",
    "flurry.com",
    "google.com/analytics",
    "google.com/recaptcha",
    "googleapis.com",
    "googlesyndication.com",
    "googletagservices.com",
    "hotjar.com",
    "imrworldwide.com",
    "intellitxt.com",
    "ip-api.com",
    "ip2location.com",
    "jsdelivr.com",
    "krxd.net",
    "linkedin.com",
    "liveinternet.ru",
    "loom.com",
    "lotame.com",
    "matomo.com",
    "media.net",
    "microsofte.com",
    "mixpanel.com",
    "moat.com",
    "nielsen.com",
    "nr-data.net",
    "omtrdc.net",
    "openx.net",
    "optimizely.com",
    "outbrain.com",
    "paypalobjects.com",
    "pinterest.com",
    "piwik.com",
    "popunder.com",
    "pubmatic.com",
    "quantcast.com",
    "quora.com",
    "rapidapi.com",
    "reddit.com",
    "rfihub.com",
    "rubiconproject.com",
    "scorecardresearch.com",
    "server.com",
    "sharethrough.com",
    "sizmek.com",
    "smartadserver.com",
    "snap.com",
    "sonobi.com",
    "spotx.tv",
    "statcounter.com",
    "stumbleupon.com",
    "taboola.com",
    "tapad.com",
    "thetradedesk.com",
    "trafficjunky.net",
    "twitter.com",
    "usertesting.com",
    "verizon.com",
    "vimeo.com",
    "visualwebsiteoptimizer.com",
    "vungle.com",
    "whatsapp.com",
    "wordpress.com",
    "yahoo.com",
    "yandex.ru",
    "yieldbot.com",
    "yieldmo.com",
    "yieldoptimizer.com",
    "youtube.com",
    "zedo.com",
    "zoh.com"
};

static void ensure_capacity(int min_capacity) {
    if (blackhole_engine.capacity < min_capacity) {
        int new_capacity = blackhole_engine.capacity == 0 ? 100 : blackhole_engine.capacity * 2;
        if (new_capacity < min_capacity) new_capacity = min_capacity;
        
        char** new_domains = realloc(blackhole_engine.domains, new_capacity * sizeof(char*));
        if (new_domains) {
            blackhole_engine.domains = new_domains;
            blackhole_engine.capacity = new_capacity;
        }
    }
}

static void lowercase_string(char* str) {
    if (!str) return;
    for (char* p = str; *p; p++) {
        *p = tolower(*p);
    }
}

static bool domain_match(const char* url, const char* domain) {
    if (!url || !domain) return false;
    
    char* url_lower = strdup(url);
    char* domain_lower = strdup(domain);
    if (!url_lower || !domain_lower) {
        if (url_lower) free(url_lower);
        if (domain_lower) free(domain_lower);
        return false;
    }
    
    lowercase_string(url_lower);
    lowercase_string(domain_lower);
    
    bool match = strstr(url_lower, domain_lower) != NULL;
    
    free(url_lower);
    free(domain_lower);
    return match;
}

bool blackhole_init(void) {
    if (blackhole_engine.initialized) return true;
    
    memset(&blackhole_engine, 0, sizeof(blackhole_engine));
    blackhole_engine.capacity = 0;
    blackhole_engine.domains = NULL;
    blackhole_engine.count = 0;
    blackhole_engine.block_count = 0;
    
    int default_count = sizeof(default_blackhole_domains) / sizeof(default_blackhole_domains[0]);
    for (int i = 0; i < default_count; i++) {
        blackhole_add_domain(default_blackhole_domains[i]);
    }
    
    blackhole_engine.last_refresh = time(NULL);
    blackhole_engine.initialized = true;
    return true;
}

void blackhole_cleanup(void) {
    if (blackhole_engine.domains) {
        for (int i = 0; i < blackhole_engine.count; i++) {
            if (blackhole_engine.domains[i]) {
                free(blackhole_engine.domains[i]);
            }
        }
        free(blackhole_engine.domains);
    }
    memset(&blackhole_engine, 0, sizeof(blackhole_engine));
}

bool blackhole_is_blocked(const char* url) {
    if (!url || !blackhole_engine.initialized) return false;
    
    char* url_clean = strdup(url);
    if (!url_clean) return false;
    
    char* protocol_end = strstr(url_clean, "://");
    if (protocol_end) {
        char* start = protocol_end + 3;
        char* path_start = strchr(start, '/');
        if (path_start) {
            *path_start = '\0';
        }
        memmove(url_clean, start, strlen(start) + 1);
    }
    
    bool blocked = false;
    for (int i = 0; i < blackhole_engine.count && !blocked; i++) {
        if (domain_match(url_clean, blackhole_engine.domains[i])) {
            blocked = true;
            blackhole_engine.block_count++;
        }
    }
    
    free(url_clean);
    return blocked;
}

int blackhole_get_block_count(void) {
    return blackhole_engine.block_count;
}

bool blackhole_add_domain(const char* domain) {
    if (!domain || !blackhole_engine.initialized) return false;
    
    ensure_capacity(blackhole_engine.count + 1);
    if (blackhole_engine.count >= blackhole_engine.capacity) return false;
    
    char* domain_copy = strdup(domain);
    if (!domain_copy) return false;
    
    lowercase_string(domain_copy);
    
    blackhole_engine.domains[blackhole_engine.count++] = domain_copy;
    return true;
}

bool blackhole_remove_domain(const char* domain) {
    if (!domain || !blackhole_engine.initialized) return false;
    
    char* domain_copy = strdup(domain);
    if (!domain_copy) return false;
    lowercase_string(domain_copy);
    
    for (int i = 0; i < blackhole_engine.count; i++) {
        if (strcmp(blackhole_engine.domains[i], domain_copy) == 0) {
            free(blackhole_engine.domains[i]);
            blackhole_engine.domains[i] = blackhole_engine.domains[blackhole_engine.count - 1];
            blackhole_engine.count--;
            free(domain_copy);
            return true;
        }
    }
    
    free(domain_copy);
    return false;
}

void blackhole_refresh(void) {
    if (!blackhole_engine.initialized) return;
    
    time_t now = time(NULL);
    if (now - blackhole_engine.last_refresh < BLACKHOLE_REFRESH_HOURS * 3600) {
        return;
    }
    
    /* In production, this would fetch updated blocklist from a remote source */
    blackhole_engine.last_refresh = now;
}