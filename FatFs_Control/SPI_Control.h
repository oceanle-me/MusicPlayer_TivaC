/*
 * SPI_Control.h
 *
 *  Created on: 26 thg 6, 2021
 *      Author: Dell
 */

#ifndef SPI_CONTROL_H_
#define SPI_CONTROL_H_
#include <stdint.h>
//! - SSI0 peripheral
//! - GPIO Port A peripheral (for SSI0 pins)
//! - SSI0Clk - PA2
//! - SSI0Fss - PA6
//! - SSI0Rx (TM4C123x) / SSI0XDAT0 (TM4C129x) - PA4
//! - SSI0Tx (TM4C123x) / SSI0XDAT1 (TM4C129x) - PA5


void SPI_Init(void);

void DelayUs (uint32_t n);
void DelayMs (uint32_t n);

#endif /* SPI_CONTROL_H_ */
