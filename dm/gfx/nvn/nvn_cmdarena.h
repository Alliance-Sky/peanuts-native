// PEANUTS begin: host chunks for command memory grants from an empty pool (GitHub issue #807)
// A game whose own allocator for command memory comes back empty grants a null pool before it records (hle_nvn.c, "Host-supplied segment"). Such a recording has no guest
// bytes the game could give out again, so the host never reclaims it; it lends each grant room from a chunk the command buffer owns, and the next grant of that command buffer
// starts where its previous recording ended, so a list takes its own bytes (Super Mario Odyssey's display lists: about 370 on average) instead of a 4 KB segment each.
// One chunk per command buffer: two threads recording at once never share one. Free of locks and of NVN, so dm/host/tests/t_nvn_cmdarena.c can check it; the caller holds its lock.
#ifndef NVN_CMDARENA_H
#define NVN_CMDARENA_H
#include <stddef.h>
#include <stdint.h>

#define NVN_CMDARENA_N 64
// A command buffer's chunk: [cur, end) is still free. cur follows every write pointer seen in the chunk (nvn_cmdarena_seen: at the end of a recording and when a command
// buffer moves to another segment), whichever command buffer object it was seen in: in Super Mario Odyssey the object that asks for the next grant does not hold the write
// pointer where its last list ended (no segment switch was seen in between: it was initialized again, or the list went through another object), and the room handed out at a grant is not used up until a recording is seen ending in it
typedef struct { const void *owner; uint8_t *cur, *end; } nvn_cmdarena_tail;
typedef struct { nvn_cmdarena_tail t[NVN_CMDARENA_N]; unsigned evict; } nvn_cmdarena;

static inline nvn_cmdarena_tail *nvn_cmdarena_find(nvn_cmdarena *a, const void *owner) {
    for (int i = 0; i < NVN_CMDARENA_N; i++) if (a->t[i].owner == owner && a->t[i].end) return &a->t[i];
    return NULL;
}
// A command buffer's write pointer wp in the segment that ends at seg_end (a recording ended there, or it moves to another segment): if that segment is a chunk, the
// chunk's free bytes start at wp or later
static inline void nvn_cmdarena_seen(nvn_cmdarena *a, const uint8_t *wp, const uint8_t *seg_end) {
    if (!wp || !seg_end) return;
    for (int i = 0; i < NVN_CMDARENA_N; i++) if (a->t[i].end == seg_end && wp > a->t[i].cur && wp <= a->t[i].end) a->t[i].cur = (uint8_t *)wp;
}
// Room of at least `room` bytes for a grant of the command buffer, in its chunk: the start (8-byte aligned), and *end the chunk's end; NULL when it has no chunk or too little
// is left (the caller gives it a new chunk with nvn_cmdarena_set). wp / seg_end: the command buffer's write pointer and segment end now
static inline uint8_t *nvn_cmdarena_take(nvn_cmdarena *a, const void *owner, uint8_t *wp, const uint8_t *seg_end, size_t room, uint8_t **end) {
    nvn_cmdarena_seen(a, wp, seg_end);
    nvn_cmdarena_tail *t = nvn_cmdarena_find(a, owner); if (!t) return NULL;
    uint8_t *p = (uint8_t *)(((uintptr_t)t->cur + 7) & ~(uintptr_t)7);
    if (p > t->end || (size_t)(t->end - p) < room) return NULL;
    *end = t->end; return p;
}
// A new chunk [mem, end) for the command buffer, in place of its old one (whose free tail is dropped); with every slot taken, the oldest-set slot's command buffer loses its
// chunk (its recordings stay where they are: chunks are never freed or handed to another command buffer)
static inline void nvn_cmdarena_set(nvn_cmdarena *a, const void *owner, uint8_t *mem, uint8_t *end) {
    nvn_cmdarena_tail *t = nvn_cmdarena_find(a, owner);
    for (int i = 0; i < NVN_CMDARENA_N && !t; i++) if (!a->t[i].end) t = &a->t[i];
    if (!t) { t = &a->t[a->evict]; a->evict = (a->evict + 1) % NVN_CMDARENA_N; }
    *t = (nvn_cmdarena_tail){owner, mem, end};
}
#endif
// PEANUTS end
