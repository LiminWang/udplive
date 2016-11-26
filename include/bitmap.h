#ifndef __bitmap_h__
#define __bitmap_h__

#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include "bitops.h"

extern int __bitmap_empty(const unsigned long *bitmap, unsigned int nbits);
extern int __bitmap_full(const unsigned long *bitmap, unsigned int nbits);
extern int __bitmap_equal(const unsigned long *bitmap1,
                          const unsigned long *bitmap2, unsigned int nbits);
extern void __bitmap_complement(unsigned long *dst, const unsigned long *src,
                                unsigned int nbits);
extern void __bitmap_shift_right(unsigned long *dst, const unsigned long *src,
                                 unsigned int shift, unsigned int nbits);
extern void __bitmap_shift_left(unsigned long *dst, const unsigned long *src,
                                unsigned int shift, unsigned int nbits);
extern int __bitmap_and(unsigned long *dst, const unsigned long *bitmap1,
                        const unsigned long *bitmap2, unsigned int nbits);
extern void __bitmap_or(unsigned long *dst, const unsigned long *bitmap1,
                        const unsigned long *bitmap2, unsigned int nbits);
extern void __bitmap_xor(unsigned long *dst, const unsigned long *bitmap1,
                         const unsigned long *bitmap2, unsigned int nbits);
extern int __bitmap_andnot(unsigned long *dst, const unsigned long *bitmap1,
                           const unsigned long *bitmap2, unsigned int nbits);
extern int __bitmap_intersects(const unsigned long *bitmap1,
                               const unsigned long *bitmap2, unsigned int nbits);
extern int __bitmap_subset(const unsigned long *bitmap1,
                           const unsigned long *bitmap2, unsigned int nbits);
extern int __bitmap_weight(const unsigned long *bitmap, unsigned int nbits);

extern void bitmap_set(unsigned long *map, unsigned int start, int len);
extern void bitmap_clear(unsigned long *map, unsigned int start, int len);

extern unsigned long bitmap_find_next_zero_area_off(unsigned long *map,
    unsigned long size,
    unsigned long start,
    unsigned int nr,
    unsigned long align_mask,
    unsigned long align_offset);

/**
 * bitmap_find_next_zero_area - find a contiguous aligned zero area
 * @map: The address to base the search on
 * @size: The bitmap size in bits
 * @start: The bitnumber to start searching at
 * @nr: The number of zeroed bits we're looking for
 * @align_mask: Alignment mask for zero area
 *
 * The @align_mask should be one less than a power of 2; the effect is that
 * the bit offset of all zero areas this function finds is multiples of that
 * power of 2. A @align_mask of 0 means no alignment is required.
 */
static inline unsigned long
bitmap_find_next_zero_area(unsigned long *map,
                           unsigned long size,
                           unsigned long start,
                           unsigned int nr,
                           unsigned long align_mask)
{
  return bitmap_find_next_zero_area_off(map, size, start, nr,
                                        align_mask, 0);
}


extern void bitmap_remap(unsigned long *dst, const unsigned long *src,
                         const unsigned long *old, const unsigned long *new, unsigned int nbits);
extern int bitmap_bitremap(int oldbit,
                           const unsigned long *old, const unsigned long *new, int bits);
extern void bitmap_onto(unsigned long *dst, const unsigned long *orig,
                        const unsigned long *relmap, unsigned int bits);
extern void bitmap_fold(unsigned long *dst, const unsigned long *orig,
                        unsigned int sz, unsigned int nbits);


#ifdef __BIG_ENDIAN
extern void bitmap_copy_le(unsigned long *dst, const unsigned long *src, unsigned int nbits);
#else
#define bitmap_copy_le bitmap_copy
#endif
extern unsigned int bitmap_ord_to_pos(const unsigned long *bitmap, unsigned int ord, unsigned int nbits);

#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) & (BITS_PER_LONG - 1)))
#define BITMAP_LAST_WORD_MASK(nbits) (~0UL >> (-(nbits) & (BITS_PER_LONG - 1)))

#define small_const_nbits(nbits) \
  (__builtin_constant_p(nbits) && (nbits) <= BITS_PER_LONG)

static inline void bitmap_zero(unsigned long *dst, unsigned int nbits)
{
  if (small_const_nbits(nbits))
    *dst = 0UL;
  else {
    unsigned int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
    memset(dst, 0, len);
  }
}

static inline void bitmap_fill(unsigned long *dst, unsigned int nbits)
{
  unsigned int nlongs = BITS_TO_LONGS(nbits);
  if (!small_const_nbits(nbits)) {
    unsigned int len = (nlongs - 1) * sizeof(unsigned long);
    memset(dst, 0xff,  len);
  }
  dst[nlongs - 1] = BITMAP_LAST_WORD_MASK(nbits);
}

static inline void bitmap_copy(unsigned long *dst, const unsigned long *src,
                               unsigned int nbits)
{
  if (small_const_nbits(nbits))
    *dst = *src;
  else {
    unsigned int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
    memcpy(dst, src, len);
  }
}

static inline int bitmap_and(unsigned long *dst, const unsigned long *src1,
                             const unsigned long *src2, unsigned int nbits)
{
  if (small_const_nbits(nbits))
    return (*dst = *src1 & *src2 & BITMAP_LAST_WORD_MASK(nbits)) != 0;
  return __bitmap_and(dst, src1, src2, nbits);
}

static inline void bitmap_or(unsigned long *dst, const unsigned long *src1,
                             const unsigned long *src2, unsigned int nbits)
{
  if (small_const_nbits(nbits))
    *dst = *src1 | *src2;
  else
    __bitmap_or(dst, src1, src2, nbits);
}

static inline void bitmap_xor(unsigned long *dst, const unsigned long *src1,
                              const unsigned long *src2, unsigned int nbits)
{
  if (small_const_nbits(nbits))
    *dst = *src1 ^ *src2;
  else
    __bitmap_xor(dst, src1, src2, nbits);
}

static inline int bitmap_andnot(unsigned long *dst, const unsigned long *src1,
                                const unsigned long *src2, unsigned int nbits)
{
  if (small_const_nbits(nbits))
    return (*dst = *src1 & ~(*src2) & BITMAP_LAST_WORD_MASK(nbits)) != 0;
  return __bitmap_andnot(dst, src1, src2, nbits);
}

static inline void bitmap_complement(unsigned long *dst, const unsigned long *src,
                                     unsigned int nbits)
{
  if (small_const_nbits(nbits))
    *dst = ~(*src);
  else
    __bitmap_complement(dst, src, nbits);
}

static inline int bitmap_equal(const unsigned long *src1,
                               const unsigned long *src2, unsigned int nbits)
{
  if (small_const_nbits(nbits))
    return !((*src1 ^ *src2) & BITMAP_LAST_WORD_MASK(nbits));
#ifdef CONFIG_S390
  if (__builtin_constant_p(nbits) && (nbits % BITS_PER_LONG) == 0)
    return !memcmp(src1, src2, nbits / 8);
#endif
  return __bitmap_equal(src1, src2, nbits);
}

static inline int bitmap_intersects(const unsigned long *src1,
                                    const unsigned long *src2, unsigned int nbits)
{
  if (small_const_nbits(nbits))
    return ((*src1 & *src2) & BITMAP_LAST_WORD_MASK(nbits)) != 0;
  else
    return __bitmap_intersects(src1, src2, nbits);
}

static inline int bitmap_subset(const unsigned long *src1,
                                const unsigned long *src2, unsigned int nbits)
{
  if (small_const_nbits(nbits))
    return ! ((*src1 & ~(*src2)) & BITMAP_LAST_WORD_MASK(nbits));
  else
    return __bitmap_subset(src1, src2, nbits);
}

static inline int bitmap_empty(const unsigned long *src, unsigned nbits)
{
  if (small_const_nbits(nbits))
    return ! (*src & BITMAP_LAST_WORD_MASK(nbits));

  return find_first_bit(src, nbits) == nbits;
}

static inline int bitmap_full(const unsigned long *src, unsigned int nbits)
{
  if (small_const_nbits(nbits))
    return ! (~(*src) & BITMAP_LAST_WORD_MASK(nbits));

  return find_first_zero_bit(src, nbits) == nbits;
}

static inline int bitmap_weight(const unsigned long *src, unsigned int nbits)
{
  if (small_const_nbits(nbits))
    return hweight_long(*src & BITMAP_LAST_WORD_MASK(nbits));
  return __bitmap_weight(src, nbits);
}

static inline void bitmap_shift_right(unsigned long *dst, const unsigned long *src,
                                      unsigned int shift, int nbits)
{
  if (small_const_nbits(nbits))
    *dst = (*src & BITMAP_LAST_WORD_MASK(nbits)) >> shift;
  else
    __bitmap_shift_right(dst, src, shift, nbits);
}

static inline void bitmap_shift_left(unsigned long *dst, const unsigned long *src,
                                     unsigned int shift, unsigned int nbits)
{
  if (small_const_nbits(nbits))
    *dst = (*src << shift) & BITMAP_LAST_WORD_MASK(nbits);
  else
    __bitmap_shift_left(dst, src, shift, nbits);
}



#endif