#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char *db_user = getenv("FAKE_DB_USER");
    const char *db_pass = getenv("FAKE_DB_PASSWORD");
    const char *api_key = getenv("FAKE_API_KEY");

    printf("db_user: %s\n", db_user ? db_user : "(not set)");
    printf("db_pass: %s\n", db_pass ? db_pass : "(not set)");
    printf("api_key: %s\n", api_key ? api_key : "(not set)");

    return 0;
}
