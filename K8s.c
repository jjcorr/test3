#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

/*
  DevOps helper (C) — create a Deployment and Service in Kubernetes by calling
  the Kubernetes REST API using HTTP basic auth.

  SECURITY NOTES:
  - This program is a bad example of hardcode credentials. 
 
  Compile:
    gcc main.c -o k8_deploy -lcurl

  Run:
    ./k8_deploy
*/

struct string_buffer {
    char *data;
    size_t size;
};

static size_t write_cb(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t realsize = size * nmemb;
    struct string_buffer *sb = (struct string_buffer*)userdata;
    char *newdata = realloc(sb->data, sb->size + realsize + 1);
    if (!newdata) return 0;
    sb->data = newdata;
    memcpy(&(sb->data[sb->size]), ptr, realsize);
    sb->size += realsize;
    sb->data[sb->size] = '\0';
    return realsize;
}

static int send_json_post(CURL *curl, const char *url, const char *user, const char *pw, const char *json, struct string_buffer *resp, int insecure_skip) {
    CURLcode res;
    struct curl_slist *headers = NULL;

    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(json));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp);
    curl_easy_setopt(curl, CURLOPT_USERNAME, user);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, pw);
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_BASIC);
    if (insecure_skip) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }

    res = curl_easy_perform(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return -1;
    }
    return 0;
}

int main(void) {
    const char *api = "https://127.0.0.1:6443";
    const char *user = "jjcorr-user";
    const char *pw = "Password12345";
    const char *app =  "demo-app";
    const char *image =  "nginx:latest";
    const char *ns =  "default";
    const char *skip_verify = getenv("K8_INSECURE_SKIP_TLS");
    int insecure = (skip_verify && strcmp(skip_verify, "1") == 0) ? 1 : 0;

    
    CURL *curl = NULL;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to init curl\n");
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    // Build Deployment JSON (apps/v1)
    char deploy_json[8192];
    snprintf(deploy_json, sizeof(deploy_json),
"{\n"
"  \"apiVersion\": \"apps/v1\",\n"
"  \"kind\": \"Deployment\",\n"
"  \"metadata\": {\n"
"    \"name\": \"%s\"\n"
"  },\n"
"  \"spec\": {\n"
"    \"replicas\": 1,\n"
"    \"selector\": {\n"
"      \"matchLabels\": { \"app\": \"%s\" }\n"
"    },\n"
"    \"template\": {\n"
"      \"metadata\": { \"labels\": { \"app\": \"%s\" } },\n"
"      \"spec\": {\n"
"        \"containers\": [\n"
"          {\n"
"            \"name\": \"%s\",\n"
"            \"image\": \"%s\",\n"
"            \"ports\": [{ \"containerPort\": 80 }]\n"
"          }\n"
"        ]\n"
"      }\n"
"    }\n"
"  }\n"
"}", app, app, app, app, image);

    // Build Service JSON (ClusterIP)
    char service_json[4096];
    snprintf(service_json, sizeof(service_json),
"{\n"
"  \"apiVersion\": \"v1\",\n"
"  \"kind\": \"Service\",\n"
"  \"metadata\": {\n"
"    \"name\": \"%s\"\n"
"  },\n"
"  \"spec\": {\n"
"    \"selector\": { \"app\": \"%s\" },\n"
"    \"ports\": [{ \"protocol\": \"TCP\", \"port\": 80, \"targetPort\": 80 }]\n"
"  }\n"
"}\n", app, app);

    // Compose URLs
    char deploy_url[1024];
    char svc_url[1024];
    snprintf(deploy_url, sizeof(deploy_url), "%s/apis/apps/v1/namespaces/%s/deployments", api, ns);
    snprintf(svc_url, sizeof(svc_url), "%s/api/v1/namespaces/%s/services", api, ns);

    struct string_buffer resp;
    resp.data = malloc(1);
    resp.size = 0;

    printf("Creating deployment '%s' in namespace '%s'...\n", app, ns);
    if (send_json_post(curl, deploy_url, user, pw, deploy_json, &resp, insecure) != 0) {
        fprintf(stderr, "Failed to create deployment.\n");
        free(resp.data);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return EXIT_FAILURE;
    }
    printf("Deployment response:\n%s\n", resp.data);

    // Reset response buffer
    resp.size = 0;
    resp.data[0] = '\0';

    printf("Creating service '%s' in namespace '%s'...\n", app, ns);
    if (send_json_post(curl, svc_url, user, pw, service_json, &resp, insecure) != 0) {
        fprintf(stderr, "Failed to create service.\n");
        free(resp.data);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return EXIT_FAILURE;
    }
    printf("Service response:\n%s\n", resp.data);

    free(resp.data);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    printf("Done. If resources already exist you may need to update instead of create.\n");
    return EXIT_SUCCESS;
}
