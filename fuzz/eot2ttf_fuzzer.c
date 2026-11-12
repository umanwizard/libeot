/* A libFuzzer harness around EOT2ttf_buffer. Build the library with
   CFLAGS="-g -O1 -fsanitize=address,fuzzer-no-link" and --disable-shared, then
   clang -g -O1 -fsanitize=address,fuzzer -I inc fuzz/eot2ttf_fuzzer.c src/.libs/libeot.a -o eot2ttf_fuzzer
   and run it on a corpus of EOT files. */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <libeot/libeot.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  if (size > 0xFFFFFFFFu) {
    return 0;
  }
  struct EOTMetadata metadata;
  memset(&metadata, 0, sizeof(metadata));
  uint8_t *fontOut = NULL;
  unsigned fontSizeOut = 0;
  EOT2ttf_buffer(data, (unsigned)size, &metadata, &fontOut, &fontSizeOut);
  EOTfreeBuffer(fontOut);
  EOTfreeMetadata(&metadata);
  return 0;
}
