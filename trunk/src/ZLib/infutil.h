/* infutil.h -- types and macros common to blocks and codes
 * Copyright (C) 1995-1998 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

#ifndef _INFUTIL_H
#define _INFUTIL_H
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
      TYPE,     /* get type bits (3, including end bit) */
      LENS,     /* get lengths for stored */
      STORED,   /* processing stored block */
      TABLE,    /* get table lengths */
      BTREE,    /* get bit lengths tree for a dynamic block */
      DTREE,    /* get length, distance trees for a dynamic block */
      CODES,    /* processing fixed or dynamic block */
      DRY,      /* output remaining window bytes */
      DONE,     /* finished last block, done */
      BAD}      /* got a data error--stuck here */
inflate_block_mode;

/* inflate blocks semi-private state */
//##ModelId=402850A50339
struct inflate_blocks_state {

  /* mode */
	//##ModelId=402850A50377
  inflate_block_mode  mode;     /* current inflate_block mode */

  /* mode dependent information */
  union {
    uInt left;          /* if STORED, bytes left to copy */
    struct {
      uInt table;               /* table lengths (14 bits) */
      uInt index;               /* index into blens (or border) */
      uIntf *blens;             /* bit lengths of codes */
      uInt bb;                  /* bit length tree depth */
      inflate_huft *tb;         /* bit length decoding tree */
    } trees;            /* if DTREE, decoding info for trees */
    struct {
      inflate_codes_statef 
         *codes;
    } decode;           /* if CODES, current state */
  } sub;                /* submode */
	//##ModelId=402850A50372
  uInt last;            /* true if this block is the last block */

  /* mode independent information */
	//##ModelId=402850A5036D
  uInt bitk;            /* bits in bit buffer */
	//##ModelId=402850A50368
  uLong bitb;           /* bit buffer */
	//##ModelId=402850A50363
  inflate_huft *hufts;  /* single malloc for tree space */
	//##ModelId=402850A5035E
  Bytef *window;        /* sliding window */
	//##ModelId=402850A50359
  Bytef *end;           /* one byte after sliding window */
	//##ModelId=402850A50354
  Bytef *read;          /* window read pointer */
	//##ModelId=402850A5034F
  Bytef *write;         /* window write pointer */
	//##ModelId=402850A5034A
  check_func checkfn;   /* check function */
	//##ModelId=402850A5033C
  uLong check;          /* check on output */

};


/* defines for inflate input/output */
/*   update pointers and return */
#define UPDBITS		{s->bitb=b;s->bitk=k;}
#define UPDIN		{z->avail_in=n;z->total_in+=p-z->next_in;z->next_in=p;}
#define UPDOUT		{s->write=q;}
#define UPDATE		{UPDBITS UPDIN UPDOUT}
#define LEAVE		{UPDATE return inflate_flush(s,z,r);}
/*   get bytes and bits */
#define LOADIN		{p=z->next_in;n=z->avail_in;b=s->bitb;k=s->bitk;}
#define NEEDBYTE	{if(n)r=Z_OK;else LEAVE}
#define NEXTBYTE	(n--,*p++)
#define NEEDBITS(j) {while(k<(j)){NEEDBYTE;b|=((uLong)NEXTBYTE)<<k;k+=8;}}
#define DUMPBITS(j) {b>>=(j);k-=(j);}
/*   output bytes */
#define WAVAIL		(uInt)(q<s->read?s->read-q-1:s->end-q)
#define LOADOUT		{q=s->write;m=(uInt)WAVAIL;}
#define WRAP		{if(q==s->end&&s->read!=s->window){q=s->window;m=(uInt)WAVAIL;}}
#define FLUSH		{UPDOUT r=inflate_flush(s,z,r); LOADOUT}
#define NEEDOUT		{if(m==0){WRAP if(m==0){FLUSH WRAP if(m==0) LEAVE}}r=Z_OK;}
#define OUTBYTE(a)	{*q++=(Byte)(a);m--;}
/*   load local pointers */
#define LOAD		{LOADIN LOADOUT}

/* masks for lower bits (size given to avoid silly warnings with Visual C++) */
extern const uInt inflate_mask[17];

/* copy as much as possible from the sliding window to the output area */
extern int inflate_flush OF((inflate_blocks_statef*, z_streamp, int));

struct internal_state      {int dummy;}; /* for buggy compilers */


#ifdef __cplusplus
}
#endif

#endif


