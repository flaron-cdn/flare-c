#include "test_runner.h"

#include <stdio.h>

int flare_tests_run = 0;
int flare_tests_failed = 0;
const char *flare_current_test = "";

void register_runtime_tests(void);
void register_request_tests(void);
void register_response_tests(void);
void register_spark_tests(void);
void register_plasma_tests(void);
void register_log_tests(void);
void register_ws_tests(void);
void register_beam_tests(void);
void register_edgeops_tests(void);

int main(void) {
    printf("flare-c tests\n");
    printf("=============\n");

    printf("\n[runtime]\n");   register_runtime_tests();
    printf("\n[request]\n");   register_request_tests();
    printf("\n[response]\n");  register_response_tests();
    printf("\n[log]\n");       register_log_tests();
    printf("\n[spark]\n");     register_spark_tests();
    printf("\n[plasma]\n");    register_plasma_tests();
    printf("\n[ws]\n");        register_ws_tests();
    printf("\n[beam]\n");      register_beam_tests();
    printf("\n[edgeops]\n");   register_edgeops_tests();

    printf("\n=============\n");
    printf("ran: %d  failed: %d\n", flare_tests_run, flare_tests_failed);
    return flare_tests_failed == 0 ? 0 : 1;
}
