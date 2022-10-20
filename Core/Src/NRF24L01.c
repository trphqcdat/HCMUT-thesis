#include "stm32f1xx_hal.h"
#include "NRF24L01.h"

extern SPI_HandleTypeDef hspi1;
#define NRF24_SPI &hspi1

#define NRF24_CE_PORT   	GPIOB
#define NRF24_CE_PIN    	GPIO_PIN_6

#define NRF24_CSN_PORT   	GPIOB
#define NRF24_CSN_PIN    	GPIO_PIN_7

void CS_Select(void)
{
	HAL_GPIO_WritePin(NRF24_CSN_PORT, NRF24_CSN_PIN, GPIO_PIN_RESET);
}

void CS_UnSelect(void)
{
	HAL_GPIO_WritePin(NRF24_CSN_PORT, NRF24_CSN_PIN, GPIO_PIN_SET);
}

void CE_Enable(void)
{
	HAL_GPIO_WritePin(NRF24_CE_PORT, NRF24_CE_PIN, GPIO_PIN_SET);
}

void CE_Disable(void)
{
	HAL_GPIO_WritePin(NRF24_CE_PORT, NRF24_CE_PIN, GPIO_PIN_RESET);
}

void nrf24_WriteReg(uint8_t Reg, uint8_t Data)									// Singular byte write on a register
{
	uint8_t buf[2];
	buf[0] = Reg|1<<5;
	buf[1] = Data;
	CS_Select();																									// Pull the CS Pin LOW to select the device
	HAL_SPI_Transmit(NRF24_SPI, buf, 2, 1000);
	CS_UnSelect();																								// Pull the CS HIGH to release the device
}

void nrf24_WriteRegMulti(uint8_t Reg, uint8_t *data, int size)	// Multiple bytes write, starting from a defined register
{
	uint8_t buf[2];
	buf[0] = Reg|1<<5;
	CS_Select();																									// Pull the CS Pin LOW to select the device
	HAL_SPI_Transmit(NRF24_SPI, buf, 1, 100);
	HAL_SPI_Transmit(NRF24_SPI, data, size, 1000);
	CS_UnSelect();																								// Pull the CS HIGH to release the device
}

uint8_t nrf24_ReadReg(uint8_t Reg)															// Singular byte read from a register
{
	uint8_t data = 0;
	CS_Select();																									// Pull the CS Pin LOW to select the device
	HAL_SPI_Transmit(NRF24_SPI, &Reg, 1, 100);
	HAL_SPI_Receive(NRF24_SPI, &data, 1, 100);
	CS_UnSelect();																								// Pull the CS HIGH to release the device
	return data;
}

void nrf24_ReadReg_Multi(uint8_t Reg, uint8_t *data, int size)	// Multiple bytes read from a register
{
	CS_Select();																									// Pull the CS Pin LOW to select the device
	HAL_SPI_Transmit(NRF24_SPI, &Reg, 1, 100);
	HAL_SPI_Receive(NRF24_SPI, data, size, 1000);					
	CS_UnSelect();																								// Pull the CS HIGH to release the device
}

void nrfsendCmd (uint8_t cmd)																		// Send the command to the NRF
{
	CS_Select();																									// Pull the CS Pin LOW to select the device
	HAL_SPI_Transmit(NRF24_SPI, &cmd, 1, 100);
	CS_UnSelect();																								// Pull the CS HIGH to release the device
}

void nrf24_reset(uint8_t REG)
{
	if (REG == STATUS)
	{
		nrf24_WriteReg(STATUS, 0x00);
	}
	else if (REG == FIFO_STATUS)
	{
		nrf24_WriteReg(FIFO_STATUS, 0x11);
	}
	else {
	nrf24_WriteReg(CONFIG, 0x08);
	nrf24_WriteReg(EN_AA, 0x3F);
	nrf24_WriteReg(EN_RXADDR, 0x03);
	nrf24_WriteReg(SETUP_AW, 0x03);
	nrf24_WriteReg(SETUP_RETR, 0x03);
	nrf24_WriteReg(RF_CH, 0x02);
	nrf24_WriteReg(RF_SETUP, 0x0E);
	nrf24_WriteReg(STATUS, 0x00);
	nrf24_WriteReg(OBSERVE_TX, 0x00);
	nrf24_WriteReg(CD, 0x00);
	uint8_t rx_addr_p0_def[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
	nrf24_WriteRegMulti(RX_ADDR_P0, rx_addr_p0_def, 5);
	uint8_t rx_addr_p1_def[5] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2};
	nrf24_WriteRegMulti(RX_ADDR_P1, rx_addr_p1_def, 5);
	nrf24_WriteReg(RX_ADDR_P2, 0xC3);
	nrf24_WriteReg(RX_ADDR_P3, 0xC4);
	nrf24_WriteReg(RX_ADDR_P4, 0xC5);
	nrf24_WriteReg(RX_ADDR_P5, 0xC6);
	uint8_t tx_addr_def[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
	nrf24_WriteRegMulti(TX_ADDR, tx_addr_def, 5);
	nrf24_WriteReg(RX_PW_P0, 0);
	nrf24_WriteReg(RX_PW_P1, 0);
	nrf24_WriteReg(RX_PW_P2, 0);
	nrf24_WriteReg(RX_PW_P3, 0);
	nrf24_WriteReg(RX_PW_P4, 0);
	nrf24_WriteReg(RX_PW_P5, 0);
	nrf24_WriteReg(FIFO_STATUS, 0x11);
	nrf24_WriteReg(DYNPD, 0);
	nrf24_WriteReg(FEATURE, 0);
	}
}

void NRF24_Init(void)
{
	CE_Disable();																										// Disable the chip before configuring the device
	nrf24_reset (0);																								// Reset all
	nrf24_WriteReg(CONFIG, 0);  																		// Will be configured late
	nrf24_WriteReg(EN_AA, 0);  																			// No Auto ACK
	nrf24_WriteReg (EN_RXADDR, 0);  																// Not enabling any data pipe right now
	nrf24_WriteReg (SETUP_AW, 0x03);  															// 5 bytes for the TX/RX address
	nrf24_WriteReg (SETUP_RETR, 0);   															// No retransmission
	nrf24_WriteReg (RF_CH, 0);  																		// will be setup during Tx or RX
	nrf24_WriteReg (RF_SETUP, 0x0E);   															// Power = 0db, data rate = 2Mbps
	CE_Enable();																										// Enable the chip after configuring the device
}

void NRF24_TxMode(uint8_t *Address, uint8_t channel)							// Set up the Tx mode
{
	CE_Disable();																										// Disable the chip before configuring the device
	nrf24_WriteReg (RF_CH, channel);  															// Select the channel
	nrf24_WriteRegMulti(TX_ADDR, Address, 5);  											// Write the TX address
	uint8_t config = nrf24_ReadReg(CONFIG);													// Power up the device
//config = config | (1<<1);   																		// Write 1 in the PWR_UP bit
	config = config & (0xF2);    																		// Write 0 in the PRIM_RX, and 1 in the PWR_UP, and all other bits are masked
	nrf24_WriteReg (CONFIG, config);
	CE_Enable();																										// Enable the chip after configuring the device
}

uint8_t NRF24_Transmit(uint8_t *data)															// Transmit the data
{
	uint8_t cmdtosend = 0;
	CS_Select();																										// Select the device
	cmdtosend = W_TX_PAYLOAD;																				// Payload command
	HAL_SPI_Transmit(NRF24_SPI, &cmdtosend, 1, 100);
	HAL_SPI_Transmit(NRF24_SPI, data, 32, 1000);										// Send the payload
	CS_UnSelect();																									// Unselect the device
	HAL_Delay(1);
	uint8_t fifostatus = nrf24_ReadReg(FIFO_STATUS);
	if ((fifostatus&(1<<4)) && (!(fifostatus&(1<<3))))							// Check the fourth bit of FIFO_STATUS to know if the TX fifo is empty
	{
		cmdtosend = FLUSH_TX;
		nrfsendCmd(cmdtosend);
		nrf24_reset (FIFO_STATUS);																		// Reset FIFO_STATUS
		return 1;
	}
	return 0;
}

void NRF24_RxMode(uint8_t *Address, uint8_t channel)							// Set up the Rx mode
{
	CE_Disable();																										// Disable the chip before configuring the device
	nrf24_reset (STATUS);
	nrf24_WriteReg (RF_CH, channel);  															// Select the channel
	uint8_t en_rxaddr = nrf24_ReadReg(EN_RXADDR);										// Select data pipe 2
	en_rxaddr = en_rxaddr | (1<<2);
	nrf24_WriteReg (EN_RXADDR, en_rxaddr);
	nrf24_WriteRegMulti(RX_ADDR_P1, Address, 5);  									// Write the Pipe1 address
	nrf24_WriteReg(RX_ADDR_P2, 0xEE);  															// Write the Pipe2 LSB address
	nrf24_WriteReg (RX_PW_P2, 32);   																// 32 bit payload size for pipe 2
	uint8_t config = nrf24_ReadReg(CONFIG);													// Power up the device in Rx mode
	config = config | (1<<1) | (1<<0);
	nrf24_WriteReg (CONFIG, config);
	CE_Enable();																										// Enable the chip after configuring the device
}

uint8_t isDataAvailable(int pipenum)
{
	uint8_t status = nrf24_ReadReg(STATUS);
	if ((status&(1<<6))&&(status&(pipenum<<1)))
	{
		nrf24_WriteReg(STATUS, (1<<6));
		return 1;
	}
	return 0;
}

void NRF24_Receive (uint8_t *data)
{
	uint8_t cmdtosend = 0;
	CS_Select();																										// Select the device
	cmdtosend = R_RX_PAYLOAD;																				// Payload command
	HAL_SPI_Transmit(NRF24_SPI, &cmdtosend, 1, 100);
	HAL_SPI_Receive(NRF24_SPI, data, 32, 1000);											// Receive the payload
	CS_UnSelect();																									// Unselect the device
	HAL_Delay(1);
	cmdtosend = FLUSH_RX;
	nrfsendCmd(cmdtosend);
}

void NRF24_ReadAll (uint8_t *data)																// Read all the Register data
{
	for (int i=0; i<10; i++)
	{
		*(data+i) = nrf24_ReadReg(i);
	}
	nrf24_ReadReg_Multi(RX_ADDR_P0, (data+10), 5);
	nrf24_ReadReg_Multi(RX_ADDR_P1, (data+15), 5);
	*(data+20) = nrf24_ReadReg(RX_ADDR_P2);
	*(data+21) = nrf24_ReadReg(RX_ADDR_P3);
	*(data+22) = nrf24_ReadReg(RX_ADDR_P4);
	*(data+23) = nrf24_ReadReg(RX_ADDR_P5);
	nrf24_ReadReg_Multi(RX_ADDR_P0, (data+24), 5);
	for (int i=29; i<38; i++)
	{
		*(data+i) = nrf24_ReadReg(i-12);
	}
}









