// PEANUTS begin: host chunks for empty-pool command memory grants (dm/gfx/nvn/nvn_cmdarena.h; hle_nvn.c cmdarena_grant). Super Mario Odyssey grants a null pool of 512
// bytes before each display list once its display-list heap is full; a 4 KB segment for each took 570 MB in the Wooded Kingdom (GitHub issue #807). Here a command buffer
// records lists of a few hundred bytes one after another, in its chunk, with real grants and another command buffer in between, the way hle_nvn.c drives the arena; and as
// Odyssey's grants behave, with the command buffer initialized again before each list (its write pointer then says nothing about the chunk: a first build gave each list
// the start of the one before, and the replay read a record of size 0 within a minute in the Wooded Kingdom).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../gfx/nvn/nvn_cmdarena.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

enum { CHUNK = 256 << 10, ROOM = 4 << 10 };
typedef struct { uint8_t *cur, *end; } cmdbuf;   // a command buffer's write pointer and segment end, as cmd_t holds them
static int chunks; static uint8_t *made[256]; static int nmade;   // every chunk, freed at the end
// hle_nvn.c: cmdarena_grant, then cmd_new_segment (which reports the old segment's write pointer first)
static void grant(nvn_cmdarena *a, cmdbuf *c) {
    uint8_t *end = NULL, *p = nvn_cmdarena_take(a, c, c->cur, c->end, ROOM, &end);
    if (!p) { p = made[nmade++] = malloc(CHUNK); end = p + CHUNK; nvn_cmdarena_set(a, c, p, end); chunks++; }
    nvn_cmdarena_seen(a, c->cur, c->end); c->cur = p; c->end = end;
}
static void real_grant(nvn_cmdarena *a, cmdbuf *c, uint8_t *mem, size_t n) { nvn_cmdarena_seen(a, c->cur, c->end); c->cur = mem; c->end = mem + n; }
// one list, its END included; EndRecording reports where it ended
static uint8_t *record(nvn_cmdarena *a, cmdbuf *c, size_t n, int tag) { uint8_t *p = c->cur; memset(p, tag, n); c->cur += n; nvn_cmdarena_seen(a, c->cur, c->end); return p; }

int main(void) {
    nvn_cmdarena a = {0}; cmdbuf c = {0}, d = {0};
    static uint8_t guest[8192];
    uint8_t *lists[2000]; size_t lens[2000]; int tags[2000];
    for (int i = 0; i < 2000; i++) {
        if (i >= 1000) c = (cmdbuf){0};   // the second half: initialized again before each list, as Odyssey does
        lens[i] = 200 + (size_t)(i * 37 % 400); tags[i] = 1 + i % 250;   // 200 to 600 bytes, about 400 on average
        grant(&a, &c);
        CHECK(c.end - c.cur >= ROOM, "grant %d left %ld bytes, not the %d a grant is promised", i, (long)(c.end - c.cur), ROOM);
        lists[i] = record(&a, &c, (lens[i] + 7) & ~(size_t)7, tags[i]);
        if (i % 7 == 3) { real_grant(&a, &c, guest, sizeof guest); record(&a, &c, 64, 0xEE); }   // the game's own memory in between: its bytes are not the chunk's
        if (i % 5 == 1) { grant(&a, &d); record(&a, &d, 300, 0xDD); }                             // another command buffer: a chunk of its own
    }
    int broken = 0;
    for (int i = 0; i < 2000; i++) for (size_t k = 0; k < lens[i]; k++) if (lists[i][k] != tags[i]) { broken++; break; }
    CHECK(!broken, "%d of 2000 lists were overwritten by a later grant", broken);
    // 2000 lists of about 400 bytes, 800 KB, fit in 4 chunks of 256 KB, plus the other command buffer's (400 × 300 bytes, one chunk); a 4 KB segment each took 8 MB
    CHECK(chunks <= 6, "%d chunks of 256 KB for 2000 lists of about 400 bytes and 400 of 300", chunks);
    // a command buffer whose slot was taken by 64 others gets a new chunk and never one in use
    nvn_cmdarena b = {0}; cmdbuf many[NVN_CMDARENA_N + 1] = {{0}}; chunks = 0;
    for (int i = 0; i <= NVN_CMDARENA_N; i++) { grant(&b, &many[i]); record(&b, &many[i], 100, i); }
    grant(&b, &many[0]);
    int shared = 0; for (int i = 1; i <= NVN_CMDARENA_N; i++) shared += many[0].cur < many[i].end && many[i].cur - 100 < many[0].end;
    CHECK(!shared && chunks == NVN_CMDARENA_N + 2, "an evicted command buffer's new room overlaps another's chunk (%d), or %d chunks", shared, chunks);
    // a recording that filled its chunk: the next grant takes a new one
    nvn_cmdarena e = {0}; cmdbuf f = {0}; chunks = 0;
    grant(&e, &f); record(&e, &f, CHUNK - ROOM + 8, 1); grant(&e, &f);
    CHECK(chunks == 2, "a chunk with less than %d bytes left was used again (%d chunks)", ROOM, chunks);
    while (nmade) free(made[--nmade]);
    if (fails) { printf("t_nvn_cmdarena: %d failures\n", fails); return 1; }
    printf("t_nvn_cmdarena: ok\n"); return 0;
}
// PEANUTS end
