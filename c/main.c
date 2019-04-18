#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>

/* A few overall notes on below implementation:
    1. It is bad practice to mix all of this stuff into a single file. It is just done here because this is an
    exercise and is quite short to allow one to read all the code in one place.
    
    2. The license of the hashing algorithm is GPLv3 despite this exercise being declared MIT license. I am notably
    *not* permitting the hashing algorithm here to be used as MIT despite being the author of this algorithm. It is
    just included here for convenience of reading. The license of the algorithm is still MIT. In real production use
    such a mix of licenses would be forbidden.
    
    3. The below code is not strictly ANSI C; I am defining some loop variables within the loops, which is not
    allow in ANSI C.
    
    4. The test input provided in the project currently is extremely minimal. In actual production authorship of
    such code a more extensive test suite should be done. Of specific note is using field names longer than 255
    character. The initial implementation of this did not support that and later commit did. As a result, in proper
    development a test would be added for that to prevent regression.
    
    5. The maximum count supported by this program is that storable in a 2 byte unsigned integer. It could be
    adjusted to support more but that is a current limitation.
*/

// Hashing algorithm from here: https://github.com/nanoscopic/xml-bare/blob/master/sh_hash_func.c
typedef uint32_t u4;
typedef uint16_t u2;
typedef uint8_t u1;
u4 hashstr( const char *str, const u2 namelen, u4 seed ) {
  u4 sum = seed;
  const char *pos = str;
  u2 i;
  for( i = 0; i < namelen; i++ ) {
    sum += *pos;
    if( sum > 16711661 ) sum -= 16711661; // prime - prevent overflow - 16,711,678 is max
    sum *= 257; // prime
    pos++;
    i++;
  }
  if( namelen < 6 ) { // ensure seed is scrambled some
    for( i=0; i < ( 6 - namelen ); i++ ) {
      sum += 137; // prime
      if( sum > 16711661 ) sum -= 16711661; // prime - prevent overflow - 16711678 is max
      sum *= 257; // prime
    }
  }
  return sum;
}

typedef struct bucketS {
   u2 count;
   char *key;
   u2 len;
} bucket;

#define BUCKETCOUNT 5000

int main( int argc, char *argv[] ) {
    FILE *fh = fopen("input","r");
    size_t memSize = sizeof( bucket ) * BUCKETCOUNT;
    bucket *buckets = calloc( 1, memSize );
    u4 seed = 0;
    
    u1 setupSuccess = 0;
    u2 ai; // Keys longer than a two byte unsigned int will overflow. Don't do that.
    
    // Attempt to find non-collision up to 10000 times. While loop is theoretically safe but bad practice
    for( int j=0;j<10000;j++ ) {
        if( j ) memset( (void *) buckets, 0x00, memSize );
        for( ai=1; ai<argc; ai++ ) {
            char *arg = argv[ai];
            u2 argLen = strlen( arg );
            // We are not checking sanity of inputs. They should in theory be checked to be valid names
            // Valid names are ascii 33 to 126 excluding colon
            
            u4 hash = hashstr( arg, argLen, seed );
            u2 bucketNum = hash % BUCKETCOUNT;
            bucket *newBucket = &buckets[ bucketNum ];
            
            // Check for collision; if one occurs restart process with a new seed
            if( newBucket->len ) {
                seed++;
                continue;
            }
            newBucket->len = argLen;
            newBucket->key = arg;
            // count will already be zero
        }
        setupSuccess = 1;
        break;
    }
    if( !setupSuccess ) {
        fprintf( stderr, "Failure to find nonoverlap\n" );
        return 1;
    }
    
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    
    /* Using simple getline here to process one line at a time
       While this does work, it would be more efficient potentially to use a memory mapped
       file. Possibly even one could read chunks of the file at a time, and do processing
       in parallel of each chunk on a separate thread. */
    while( (read = getline( &line, &len, fh ) ) != -1 ) {
        char c1 = line[0];
        // As per rfc822 field names ( keys ) should be between ascii 33 and 126, excluding colon
        // As per rfc2616 additional lines of content for a header will have their initial character
        //   by a space/LWS ( eg, not a field name character )
        if( c1 < 33 || c1 > 126 || c1 == ':' ) continue;
        
        u2 keyLen = 0;
        for( u2 i=0;i<len;i++ ) {
            if( line[i] == ':' ) {
                line[i] = 0x00;
                keyLen = i;
                break;
            }
        }
        if( !keyLen ) continue;
        
        u4 hash = hashstr( line, keyLen, seed );
        u2 bucketNum = hash % BUCKETCOUNT;
        bucket *loc = &buckets[ bucketNum ];
        
        // This check will automatically ignore empty buckets since their len is set to 0
        if( loc->len != keyLen ) continue;
        
        /* This check will ignore field names that weren't specifically asked for but happen to
           hash to a bucket already being used */
        if( strncmp( loc->key, line, keyLen ) ) continue;
        
        loc->count++;
    }
    if( line ) free( line );
    
    fclose( fh );
    
    for( ai=1; ai<argc; ai++ ) {
        char *arg = argv[ai];
        u2 argLen = strlen( arg );
        u4 hash = hashstr( arg, argLen, seed );
        u2 bucketNum = hash % BUCKETCOUNT;
        bucket *loc = &buckets[ bucketNum ];
        printf("Field name:%s Count:%i\n", arg, loc->count );
    }
    
    return 0;
}