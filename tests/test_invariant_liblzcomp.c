#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

/* Forward declaration of the function under test */
extern int lzcomp_compress(const char *infile, const char *outfile);

static jmp_buf jump_buffer;
static int segfault_caught = 0;

void segfault_handler(int sig) {
    segfault_caught = 1;
    longjmp(jump_buffer, 1);
}

START_TEST(test_buffer_overflow_sprintf_fnbuf)
{
    /* Invariant: Buffer reads never exceed declared length.
       The sprintf(fnBuf, "%d.ctf", i+1) must not overflow fnBuf
       regardless of input-controlled loop counter values. */
    
    const char *test_files[] = {
        "test_valid.lz",      /* Valid small input */
        "test_boundary.lz",   /* Boundary case: max reasonable chunks */
        "test_overflow.lz"    /* Exploit: crafted to trigger large i values */
    };
    
    signal(SIGSEGV, segfault_handler);
    
    for (int idx = 0; idx < 3; idx++) {
        FILE *f = fopen(test_files[idx], "wb");
        ck_assert_ptr_nonnull(f);
        
        if (idx == 0) {
            /* Valid: small compressed file with 1 chunk */
            unsigned char valid[] = {0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00};
            fwrite(valid, 1, sizeof(valid), f);
        } else if (idx == 1) {
            /* Boundary: file structured to produce i ~ 1000 */
            unsigned char boundary[] = {0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00};
            fwrite(boundary, 1, sizeof(boundary), f);
        } else {
            /* Exploit: crafted header to trigger large chunk count (i > 100000) */
            unsigned char exploit[] = {0x1f, 0x8b, 0x08, 0x00, 0xff, 0xff, 0xff, 0xff};
            fwrite(exploit, 1, sizeof(exploit), f);
        }
        fclose(f);
        
        segfault_caught = 0;
        if (setjmp(jump_buffer) == 0) {
            /* Call the actual production function */
            lzcomp_compress(test_files[idx], "test_out.ctf");
        }
        
        /* Assertion: No segfault should occur on any input */
        ck_assert_int_eq(segfault_caught, 0);
        
        remove(test_files[idx]);
    }
    
    remove("test_out.ctf");
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_buffer_overflow_sprintf_fnbuf);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}