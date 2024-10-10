
#ifndef SPI_H_
#define SPI_H_

#define SPI1EN			(1U<<12)
#define SPI2EN			(1U<<14)

#define CR1_CPHA		(1U<<0)
#define CR1_CPOL		(1U<<1)
#define CR1_MSTR		(1U<<2)
#define CR1_BR			(1U<<3)
#define CR1_SPE			(1U<<6)
#define CR1_LSB			(1U<<7)
#define CR1_RXONLY		(1U<<10)
#define CR1_DFF			(1U<<11)
#define CR1_BIDIMODE	(1U<<15)

#define CR2_SSOE		(1U<<2)

#define SR_BSY			(1U<<7)
#define SR_TXE			(1U<<1)
#define SR_RXNE			(1U<<0)
#define SPI2_SCK 		(1U<<10)
#define SPI2_MISO		(1U<<14)
#define SPI2_CS			(1U<<15)

void spi1_init(void);
void reset(void);
void spi1_transmit(uint8_t* data,uint32_t size);
void spi1_transmit_24bits(uint32_t data,uint32_t size);
void ws2812_transmit(uint32_t* data,uint32_t size);
void ws2812_delayed_transmit(uint32_t* data,uint32_t size,uint32_t delay);

void SPI2_init_8bit(void);
void SPI2_init_16bit(void);
uint8_t SPI2_receive_8bit(void);
uint16_t SPI2_receive_16bit(void);
void cs_enable(void);
void cs_disable(void);

#endif /* SPI_H_ */
