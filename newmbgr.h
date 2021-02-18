#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

enum NetherBiomes {
  NetherWastes = 8,
  TheVoid = 127,
  SoulSandValley = 170,
  CrimsonForest = 171,
  WarpedForest = 172,
  BasaltDeltas = 173,
};
typedef uint32_t NetherBiomes;

/**
 * <div rustbindgen hide></div>
 */
typedef struct Noise Noise;

typedef struct NetherGen {
  uint64_t seed;
  struct Noise *_noise;
  bool is_3d;
} NetherGen;

struct NetherGen *create_new_nether(uint64_t seed);

void delete(struct NetherGen *nether_gen);

NetherBiomes get_biome(struct NetherGen *nether_gen, int32_t x, int32_t y, int32_t z);
