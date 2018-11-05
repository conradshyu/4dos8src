typedef unsigned long int UINT4;// 4-byte word
typedef struct {		// MD5 context
  UINT4 state[4];		// state (ABCD)
  UINT4 count[2];		// number of bits, modulo 2^64 (LSB first)
  unsigned char buffer[64];	// input buffer
} MD5_CTX;

void MD5Init (MD5_CTX *);
void MD5Update (MD5_CTX *, const unsigned char _far *, unsigned int);
void MD5Final (unsigned char [16], MD5_CTX *);
