#include <stdint.h>
#ifndef _74HCT597_H_
#define _74HCT597_H_

#define SHIFT_MR			(1U<<1)
#define SHIFT_PL			(1U<<2)
#define SHIFT_STCP			(1U<<3)


void shift_init(void);
void shift_load(void);
uint8_t shift_read_8bit(void);
uint16_t shift_read_16bit(void);
uint64_t shift_read_64bit(void);
#endif /* _74HCT597_H_ */
