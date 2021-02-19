#include "finders.h"
#include <stdio.h>
#include <stdlib.h>
#include <gcrypt.h>
#define MAXBUFLEN 20000
#include <string.h>
#include "./minecraft_nether_gen_rs.h"

void int64ToChar(unsigned char a[], int64_t n) {
  a[0] = (n >> 56) & 0xFF;
  a[1] = (n >> 48) & 0xFF;
  a[2] = (n >> 40) & 0xFF;
  a[3] = (n >> 32) & 0xFF;
  a[4] = (n >> 24) & 0xFF;
  a[5] = (n >> 16) & 0xFF;
  a[6] = (n >> 8) & 0xFF;
  a[7] = n & 0xFF;
}

long l2norm(long x1, long z1, long x2, long z2){
  return (x1-x2)*(x1-x2) + (z1-z2)*(z1-z2);
}

uint64_t charTo64bitNum(char a[]) {
  uint64_t n = (unsigned long) (a[7] & 0xFF);
  n |= (unsigned long) (a[6] & 0xFF) << 8;
  n |= (unsigned long) (a[5] & 0xFF) << 16;
  n |= (unsigned long) (a[4] & 0xFF) << 24;
  n |= (unsigned long) (a[3] & 0xFF) << 32;
  n |= (unsigned long) (a[2] & 0xFF) << 40;
  n |= (unsigned long) (a[1] & 0xFF) << 48;
  n |= (unsigned long) (a[0] & 0xFF) << 56;
  return n;
}

uint64_t rand64()
{
  uint64_t rv = 0;
  int c,i;
  FILE *fp;
  fp = fopen("/dev/urandom", "r");

  for (i=0; i < sizeof(rv); i++) {
     do {
       c = fgetc(fp);
     } while (c < 0);
     rv = (rv << 8) | (c & 0xff);
  }
  fclose(fp);
  return rv;
}

// int netherStructureType(uint64_t seed, ){ //0 for fortress 1 for bastion
//   unsigned long modulus = 1UL << 48;
//   unsigned long AA = 341873128712;
//   unsigned long BB = 132897987541;
//   int64_t fakeseed = (seed + 30084232UL) ^ 0x5deece66dUL;
//   int64_t chunkx = nextInt(&fakeseed, 31) % 23;
//   int64_t chunkz = nextInt(&fakeseed, 31) % 23;
//   return (nextInt(&fakeseed, 31) % 5)  < 2;
// }


char bastionbiome(uint64_t seed){
  unsigned long modulus = 1ULL << 48;
  int64_t fakeseed = (seed + 30084232ULL) ^ 0x5deece66dUL;
  int64_t chunkx = next(&fakeseed, 31) % 23;
  int64_t chunkz = next(&fakeseed, 31) % 23;
  NetherGen* netherGen=create_new_nether(seed);
  NetherBiomes biome=get_biome(netherGen,chunkx*16,60,chunkz*16);
/*  if (biome==NetherWastes){
    printf("Bastion1: Wastes!\n");
  }
  if (biome==SoulSandValley){
    printf("Bastion1: SoulSand\n");
  }
  if (biome==CrimsonForest){
    printf("Bastion1: Crimson\n");
  }
  if (biome==WarpedForest){
    printf("Bastion1: Warped\n");
  }*/
  if (biome==BasaltDeltas){
    //printf("Bastion1: Basalt...\n");
    delete(netherGen);
    return 0;
  }
  delete(netherGen);
  return 1;
}

//simplex / stack layers / points
char netherchecker(uint64_t seed, int* fortressQuadrant){ 
  //return true if the nether is good (3 structures within -128 to 128 ignoring neg/neg) at 
  unsigned long modulus = 1ULL << 48;
  unsigned long AA = 341873128712;
  unsigned long BB = 132897987541;
  int bastionCount = 0;
  *fortressQuadrant = 0;
  int64_t fakeseed = (seed + 30084232ULL) ^ 0x5deece66dUL;
  int64_t chunkx = next(&fakeseed, 31) % 23;
  int64_t chunkz = next(&fakeseed, 31) % 23;
  int structureType = (next(&fakeseed, 31) % 5)  >= 2;
  bastionCount += structureType;
  if (chunkx > 8 || chunkz > 8 || structureType == 0){
    return 0;
  }
  int gotfort = 0;
  fakeseed = (seed + 30084232UL - AA) ^ 0x5deece66dUL;
  chunkx = next(&fakeseed, 31) % 23;
  chunkz = next(&fakeseed, 31) % 23;
  structureType = (next(&fakeseed, 31) % 5)  >= 2;
  bastionCount += structureType;
  if (structureType == 0){
    *fortressQuadrant = -1;
  }
  if (chunkx >= 19 && chunkz <= 8 && structureType == 0){
    return 1;
  }

  fakeseed = (seed + 30084232UL - BB) ^ 0x5deece66dUL;
  chunkx = next(&fakeseed, 31) % 23;
  chunkz = next(&fakeseed, 31) % 23;
  structureType = (next(&fakeseed, 31) % 5)  >= 2;
  bastionCount += structureType;
  if (structureType == 0){
    *fortressQuadrant = 1;
  }
  if (chunkx <= 8 && chunkz >= 19 && structureType == 0){
    return 1;
  }

  if (bastionCount != 2){
    return 0;
  }

  return 0;
}

int validateSeed(uint64_t seed){
  return 0;
}

int main () {
    printf("Filtering seeds for FSG category... v 0.5 \n");
    FILE *fp = fopen("csprng.c","rb"); //Uses SHA256 of this source code for key
    char source[MAXBUFLEN + 1];
    if (fp != NULL) {
        size_t newLen = fread(source, sizeof(char), MAXBUFLEN, fp);
        if ( ferror( fp ) != 0 ) {
            fputs("Error reading file", stderr);
        } else {
            source[newLen++] = '\0';
        }
        fclose(fp);
    }
    unsigned char *salsaKey;
    unsigned i;
    unsigned int l = gcry_md_get_algo_dlen(GCRY_MD_SHA256); /* get digest length (used later to print the result) */

    gcry_md_hd_t h;
    gcry_md_open(&h, GCRY_MD_SHA256, GCRY_MD_FLAG_SECURE); /* initialise the hash context */
    gcry_md_write(h, source, strlen(source)); /* hash some text */
    salsaKey = gcry_md_read(h, GCRY_MD_SHA256); /* get the result */

    gcry_error_t     gcryError;
    gcry_cipher_hd_t gcryCipherHd;
    size_t           index;

    unsigned char iniVector[8];
    uint64_t NONCE = rand64();
    int seedcounter = 0;
    int64ToChar(iniVector, NONCE);

    printf("Verification Token: ");
    for (i = 0; i < 8; i++)
    {
        printf("%02x", iniVector[i]); /* print the result */
    }
    printf("\n");

    gcryError = gcry_cipher_open(
        &gcryCipherHd, // gcry_cipher_hd_t *
        GCRY_CIPHER_SALSA20,   // int
        GCRY_CIPHER_MODE_STREAM,   // int
        0);            // unsigned int
    if (gcryError)
    {
        printf("gcry_cipher_open failed:  %s/%s\n",
               gcry_strsource(gcryError),
               gcry_strerror(gcryError));
        return 1;
    }
    
    gcryError = gcry_cipher_setkey(gcryCipherHd, salsaKey, 32);
    if (gcryError)
    {
        printf("gcry_cipher_setkey failed:  %s/%s\n",
               gcry_strsource(gcryError),
               gcry_strerror(gcryError));
        return 1;
    }
    
    gcryError = gcry_cipher_setiv(gcryCipherHd, iniVector, 8);
    if (gcryError)
    {
        printf("gcry_cipher_setiv failed:  %s/%s\n",
               gcry_strsource(gcryError),
               gcry_strerror(gcryError));
        return 1;
    }
    
    size_t txtLength = 8;
    uint64_t seed = 0;
    unsigned char * encBuffer = malloc(txtLength+1);
    unsigned char * textBuffer = malloc(txtLength+1);
    memset(textBuffer, 0, 9);
    int ii, ss;

    int has_lower = 0;
    uint64_t lower48 = 0;
    uint64_t upper16 = 0;
    int is_done = 0;

    initBiomes();

    // Note that some structure configs changed with the Minecraft versions.
    const StructureConfig sconf = VILLAGE_CONFIG;
    const StructureConfig sconf_portal = RUINED_PORTAL_CONFIG;
    const StructureConfig sconf_shipwreck = SHIPWRECK_CONFIG;
    int mc = MC_1_16;
    unsigned long modulus = 1UL << 48;
    LayerStack g;
    setupGenerator(&g, mc);
    int valid;
    Pos p;
    int valid2;
    Pos p2;
    int valid3;
    Pos p3;
    Pos spawn;
    int shipModulus = 20;
    int64_t fakeseed = 0;
    int64_t carvea = 0;
    int64_t carveb = 0;
    int goodVillage = 0;
    int goodShipwreck = 0;
    int mainStructureValid = 0;
    int shipBiome = -1;
    int portBiome = -1;
    int shipchunkx=-1;
    int shipchunkz=-1;
    int shiptype = -1;
    int portcx = -1;
    int portcz = -1;
    float portOceanY = 0.0;
    int portOceanType = -1;
    float portNormalY = 0.0;
    int portNormalType = -1;
    int portModulus = 10;
    int validPort = -1;
    int portBig = -1;
    int portType = -1;
    int ohcrap  = 0;
    int fortressQuad = 0;
  
    for(ii=0; is_done < 1; ii++){
      gcryError = gcry_cipher_encrypt(
          gcryCipherHd, // gcry_cipher_hd_t
          encBuffer,    // void *
          txtLength,    // size_t
          textBuffer,    // const void *
          txtLength);   // size_t
      if (gcryError)
      {
          printf("gcry_cipher_decrypt failed:  %s/%s\n",
                gcry_strsource(gcryError),
                gcry_strerror(gcryError));
          return 1;
      }
      seedcounter++;
      seed = charTo64bitNum(encBuffer);
      if (has_lower < 1){
        lower48 = seed >> 16;
        p2 = getStructurePos(sconf_portal, lower48, 0, 0, &valid2);
        goodVillage = 0;
        goodShipwreck = 0;
        mainStructureValid = 0;
        shipBiome = -1;
        portBiome = -1;
        validPort = -1;
        if (!netherchecker(lower48, &fortressQuad) || !valid2 || (p2.x <= 80 && p2.z <= 80) || p2.x >= 144 || p2.z >= 144 ){ 
          //either no portal or nether sucks
          has_lower = 0;
        } else { //valid nethers AND close portals here
          
          portcx = p2.x >> 4;
          portcz = p2.z >> 4;
          fakeseed = (lower48) ^ 0x5deece66dL;
          carvea = nextLong(&fakeseed);
          carveb = nextLong(&fakeseed);
          fakeseed = ((portcx * carvea) ^ (portcz * carveb) ^ lower48) ^ 0x5deece66dL;
          fakeseed = fakeseed & 0xFFFFFFFFFFFF;
          portOceanY = nextFloat(&fakeseed);
          portOceanType = next(&fakeseed, 31); //raw not mod
          portNormalY = nextFloat(&fakeseed);
          portNormalType = next(&fakeseed, 31); //raw not mod
          
          //carver seed set
          if (portOceanY < .5){
            has_lower = 0;
            //restart here underground portal
          } else {
            p = getStructurePos(sconf, lower48, 0, 0, &valid);
            p3 = getStructurePos(sconf_shipwreck, lower48, 0, 0, &valid3);
            if (!valid || p.x >= 96 || p.z >= 96){
              goodVillage = 0;
            } else {
              goodVillage = 1;
            }
            if (!valid3 || p3.x >= 96 || p3.z >= 96){
              goodShipwreck = 0;
            } else {
              goodShipwreck = 1;
            }

            if (goodShipwreck > 0 || goodVillage > 0){
              has_lower = seedcounter;
            } else {
              has_lower = 0;
            }
          }
        }
      } else {
        upper16 = seed >> 48;
        seed = lower48 | (upper16 << 48);
        applySeed(&g, seed);
        mainStructureValid = 0;
        if (isViableStructurePos(sconf_portal.structType, mc, &g, seed, p2.x, p2.z)){ //always a portal
          portBiome = getBiomeAtPos(&g, p2);
          validPort = -1;
          if (isOceanic(portBiome) || portBiome == desert || portBiome == desert_hills || portBiome == swamp || portBiome == swamp_hills){
            // set Port Type for validation
            if (portOceanY < .05){
              portBig = 1;
              portType = portOceanType % 3;
            } else {
              portBig = 0;
              portType = portOceanType % 10;
            }
          } else {
            // set Port Type for validation 
            if (portNormalY < .05){
              portBig = 1;
              portType = portNormalType % 3;
            } else {
              portBig = 0;
              portType = portNormalType % 10;
            }
          }
          if ((portBig == 0) && ( portType == 0 || portType == 2 || portType == 3 || portType == 4 || portType == 5 || portType ==8 )) { //nether filter fails
            mainStructureValid = 0;
            has_lower = 0;
          } else { //nether filter passed
            if ((goodVillage > 0) && isViableStructurePos(sconf.structType, mc, &g, seed, p.x, p.z)){
              mainStructureValid = 1;
            } else {
              if ((goodShipwreck > 0) && isViableStructurePos(sconf_shipwreck.structType, mc, &g, seed, p3.x, p3.z)){
                mainStructureValid = 2;
                shipBiome = getBiomeAtPos(&g, p3);
                if (isOceanic(shipBiome)){
                  shipModulus = 20;
                } else {
                  shipModulus = 11;
                }
                shipchunkx = p3.x >> 4;
                shipchunkz = p3.z >> 4;
                fakeseed = (seed) ^ 0x5deece66dL;
                carvea = nextLong(&fakeseed);
                carveb = nextLong(&fakeseed);
                fakeseed = ((shipchunkx * carvea) ^ (shipchunkz * carveb) ^ seed) ^ 0x5deece66dL;
                fakeseed = fakeseed & 0xFFFFFFFFFFFF;
                fakeseed = (0x5deece66dUL*fakeseed + 11) % modulus ;
                fakeseed = (0x5deece66dUL*fakeseed + 11) % modulus ;
                shiptype = (fakeseed >> 17) % shipModulus;
                if (shipModulus == 20){
                  if (shiptype == 2 || shiptype == 5 || shiptype == 8 || shiptype == 12 || shiptype == 15 || shiptype == 18){
                    mainStructureValid = 0;
                    has_lower = 0;
                  }
                }
                if (shipModulus == 11){
                  if (shiptype == 2 || shiptype == 5 || shiptype == 9 || shiptype == 11){
                    mainStructureValid = 0;
                    has_lower = 0;
                  }
                } //note: if performance tweaking needed just presume ocean, 
                  //check in lower48, filter stronger later
              } else {
                mainStructureValid = 0; //re rolls biome ONLY
                ohcrap += 1;
                if (ohcrap > 50){
                  //printf("I give up :/\n");
                  has_lower = 0;
                  ohcrap = 0;
                }
              }
            }
          }
        }
        if (mainStructureValid > 0 && bastionbiome(seed) > 0){
          spawn = getSpawn(mc, &g, NULL, seed);
          if (spawn.x >= -48 && spawn.x <= 144 && spawn.z >= -48 && spawn.z <= 144){
            is_done = 1;
            StrongholdIter sh;
            Pos pos_sh = initFirstStronghold(&sh, mc, seed);
            long sh_dist = 0xffffffffffff;
            long temp = 0;
            LayerStack g;
            setupGenerator(&g, mc);
            applySeed(&g, seed);
            int i, N = 3;
            for (i = 1; i <= N; i++)
            {
                if (nextStronghold(&sh, &g, NULL) <= 0)
                    break;
                if (fortressQuad == -1){
                  temp = l2norm(sh.pos.x, sh.pos.z, -1200L, 1200L);
                  if (temp < sh_dist){
                    sh_dist = temp;
                  }
                } else if (fortressQuad == 1){
                  temp = l2norm(sh.pos.x, sh.pos.z, 1200L, -1200L);
                  if (temp < sh_dist){
                    sh_dist = temp;
                  }
                }
            }
            if (sh_dist > 300*300){
              mainStructureValid = 0;
              has_lower = 0;
              is_done = 0;
            } else {
              if (mainStructureValid == 1){
                printf("Village Spawn\n");
              }
              if (mainStructureValid == 2){
                printf("Shipwreck Spawn\n");
              }
              printf("Seed %ld\n", seed);
              printf("Examined %d seeds took lower after %d seeds\n", seedcounter, has_lower);
            }            
          }
        }
      }
    }
    return 0;
}