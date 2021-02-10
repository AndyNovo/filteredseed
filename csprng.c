#include "finders.h"
#include <stdio.h>
#include <stdlib.h>
#include <gcrypt.h>
#define MAXBUFLEN 20000
#include <string.h>

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

char netherchecker(uint64_t seed){
  unsigned long modulus = 1UL << 48;
  unsigned long AA = 341873128712;
  unsigned long BB = 132897987541;
  
  //the structure in pos/pos quadrant is within 8*16 (128) of 0,0
  if ( ( ( ( (0x5deece66dUL*((seed + 30084232UL) ^ 0x5deece66dUL) + 11) % modulus ) >> 17 ) % 23) > 8){
    return 0;
  }
  if (((( 0x5deece66dUL* (0x5deece66dUL*((seed + 30084232UL) ^ 0x5deece66dUL) + 11) + 11) % modulus ) >> 17 ) % 23 > 8){
    return 0;
  }

  //now check the neg/pos quadrant (0 <= pos coord <= 128 and -128 <= neg coord <= -64 )
  if ( ( ( ( (0x5deece66dUL*((seed + 30084232UL - AA) ^ 0x5deece66dUL) + 11) % modulus ) >> 17 ) % 23) < 19){
    return 0;
  }
  if (((( 0x5deece66dUL* (0x5deece66dUL*((seed + 30084232UL - AA) ^ 0x5deece66dUL) + 11) + 11) % modulus ) >> 17 ) % 23 > 8){
    return 0;
  }

    //now check the pos/neg quadrant (0 <= pos coord <= 128 and -128 <= neg coord <= -64 )
  if ( ( ( ( (0x5deece66dUL*((seed + 30084232UL - BB) ^ 0x5deece66dUL) + 11) % modulus ) >> 17 ) % 23) > 8){
    return 0;
  }
  if (((( 0x5deece66dUL* (0x5deece66dUL*((seed + 30084232UL - BB) ^ 0x5deece66dUL) + 11) + 11) % modulus ) >> 17 ) % 23 < 19){
    return 0;
  }

  //not bothering with the neg/neg quadrant
  return 1;
}

int validateSeed(uint64_t seed){
  return 0;
}

int main () {
    printf("Finding you a random good seed for RGSG category... v 0.2 \n");
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
    int ii;

    int has_lower = 0;
    uint64_t lower48 = 0;
    uint64_t upper16 = 0;
    int is_done = 0;

    initBiomes();

    // Note that some structure configs changed with the Minecraft versions.
    const StructureConfig sconf = VILLAGE_CONFIG;
    const StructureConfig sconf_portal = RUINED_PORTAL_CONFIG;
    int mc = MC_1_16;

    LayerStack g;
    setupGenerator(&g, mc);
    int valid;
    Pos p;
    int valid2;
    Pos p2;
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
      if (seedcounter % 100000 == 0){
        printf("tested %d seeds so far\n", seedcounter);
      }
      if (has_lower < 1){
        lower48 = seed >> 16;
        p = getStructurePos(sconf, lower48, 0, 0, &valid);
        // The structure position depends only on the region coordinates and
        // the lower 48-bits of the world seed.
        p2 = getStructurePos(sconf_portal, lower48, 0, 0, &valid);

        // Look for a seed with the structures at the origin chunk.
        if (!valid || p.x >= 96 || p.z >= 96 || p2.x >= 96 || p2.z >= 96 || !netherchecker(lower48) ){
          has_lower = 0;
        } else {
          has_lower = seedcounter;
        }
      } else {
        upper16 = seed >> 48;
        seed = lower48 | (upper16 << 48);
        if (isViableStructurePos(sconf.structType, mc, &g, seed, p.x, p.z) 
            && isViableStructurePos(sconf_portal.structType, mc, &g, seed, p2.x, p2.z)){
          is_done = 1;
          printf("Seed %ld is a good seed (close vilage, close nether portal, 3ish close nether structures).\n", seed);
          printf("Examined %d seeds took lower after %d seeds\n", seedcounter, has_lower);
        }
      }
    }
    return 0;
}