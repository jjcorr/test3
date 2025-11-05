#include <stdio.h>

const char *DB_USERNAME = "hardcoded_user";
const char *DB_PASSWORD = "HARDCODED_FAKE_PASS_12345"; // <-- synthetic secret

void print_db_info(void) {
    printf("DB user: %s\n", DB_USERNAME);
    printf("DB password: %s\n", DB_PASSWORD);
}

int main_db(void) {
    print_db_info();
    return 0;
}
