#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>

// Hashing algorithm from here: https://github.com/nanoscopic/xml-bare/blob/master/sh_hash_func.c
#define BYTE_MULT 257
typedef uint64_t u8;
typedef uint32_t u4;
typedef uint16_t u2;
typedef uint8_t u1;
u4 hashstr( const char *str, const u1 namelen, u4 seed ) {
  u4 sum = seed;//4,294,967,295 / 257 = ... ( nearest prime )
  const char *pos = str;
  u1 i;
  for( i = 0; i < namelen; i++ ) {
    sum += *pos;
    if( sum > 16711661 ) sum -= 16711661; // prime - prevent overflow - 16,711,678 is max
    sum *= BYTE_MULT; // prime
    pos++;
    i++;
  }
  if( namelen < 6 ) { // ensure seed is scrambled some
    for( i=0; i < ( 6 - namelen ); i++ ) {
      sum += 137; // prime
      if( sum > 16711661 ) sum -= 16711661; // prime - prevent overflow - 16711678 is max
      sum *= BYTE_MULT; // prime
    }
  }
  return sum;
}

typedef struct bucketS {
   u2 count;
   char *key;
   u1 len;
} bucket;

int main( int argc, char *argv[] ) {
    FILE *fh = fopen("input","r");
    
    size_t memSize = sizeof( bucket ) * 5000;
    bucket *buckets = calloc( 1, memSize );
    
    int seed = 0;
    
    int success = 0;
    int ai;
    for( int j=0;j<100;j++ ) {
        if( j ) memset( (void *) buckets, 0x00, memSize );
        for( ai=1; ai<argc; ai++ ) {
            char *arg = argv[ai];
            int argLen = strlen( arg );
            u4 hash = hashstr( arg, argLen, seed );
            u2 bucketNum = hash % 5000;
            //printf("key: %s bucket: %i\n", arg, bucketNum );
            bucket *newBucket = &buckets[ bucketNum ];
            if( newBucket->len ) {
                seed++;
                continue;
            }
            newBucket->len = argLen;
            newBucket->key = arg;
            // count will already be zero
        }
        success = 1;
        break;
    }
    if( !success ) {
        fprintf(stderr, "Failure to find nonoverlap\n" );
        exit(1);
    }
    
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while( (read = getline( &line, &len, fh ) ) != -1 ) {
        char c1 = line[0];
        if( c1 < 33 || c1 > 126 ) continue;
        
        u1 keyLen = 0;
        for( int i=0;i<len;i++ ) {
            if( line[i] == ':' ) {
                line[i] = 0x00;
                keyLen = i;
                break;
            }
        }
        if( !keyLen ) continue;
        
        //printf("Field name:%s\n", line );
        u4 hash = hashstr( line, keyLen, seed );
        u2 bucketNum = hash % 5000;
        bucket *loc = &buckets[ bucketNum ];
        if( loc->len != keyLen ) continue;
        if( strncmp( loc->key, line, keyLen ) ) continue;
        loc->count++;
        //printf("Bucket num: %i\n", bucketNum );
    }
    if( line ) free( line );
    
    fclose( fh );
    
    for( int ai=1; ai<argc; ai++ ) {
        char *arg = argv[ai];
        int argLen = strlen( arg );
        u4 hash = hashstr( arg, argLen, seed );
        u2 bucketNum = hash % 5000;
        bucket *loc = &buckets[ bucketNum ];
        printf("Field name:%s Count:%i\n", arg, loc->count );
    }
    
    return 0;
}