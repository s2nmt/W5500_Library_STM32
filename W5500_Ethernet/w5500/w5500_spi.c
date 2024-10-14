#include "stm32g0xx_hal.h"
#include "wizchip_conf.h"
#include "stdio.h"

extern SPI_HandleTypeDef hspi1;

uint8_t SPIReadWrite(uint8_t data)
{
	while((hspi1.Instance->SR & SPI_FLAG_TXE) != SPI_FLAG_TXE);
	*(__IO uint8_t*)&hspi1.Instance->DR = data;
	while((hspi1.Instance->SR & SPI_FLAG_RXNE) !=SPI_FLAG_RXNE);
	return (*(__IO uint8_t*)&hspi1.Instance->DR);
}

//uint8_t SPIReadWrite(uint8_t data)
//{
////	printf("Test LAN 1\n");
//	int i = 0;
//	while((hspi1.Instance->SR & SPI_FLAG_TXE) != SPI_FLAG_TXE)
//	{
//		i++;
//		if (i == 1000)
//		{
//			i = 0;
//			break;
//		}
//	}
//	*(__IO uint8_t*)&hspi1.Instance->DR = data;
//	while((hspi1.Instance->SR & SPI_FLAG_RXNE) !=SPI_FLAG_RXNE)
//	{
//		i++;
//		if (i == 1000)
//		{
//			i = 0;
//			break;
//		}
//	}
////	printf("Test LAN 2\n");
//	return (*(__IO uint8_t*)&hspi1.Instance->DR);
//}

/*uint8_t SPIReadWrite(uint8_t data)
{
	uint8_t rxData;
	HAL_SPI_TransmitReceive(&hspi1, &data, &rxData, 1, HAL_MAX_DELAY);
	return rxData;
}*/

void wizchip_select(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
}

void wizchip_deselect(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
}
uint8_t wizchip_read()
{
	uint8_t rb;
	rb=SPIReadWrite(0x00);
	return rb;
}
void wizchip_write(uint8_t wb)
{
	SPIReadWrite(wb);
}
void wizchip_readburst(uint8_t* pBuf, uint16_t len)
{
	for(uint16_t i = 0; i < len; i++)
	{
		*pBuf=SPIReadWrite(0x00);
		pBuf++;
	}
}
void wizchip_writeburst(uint8_t* pBuf, uint16_t len)
{
	for(uint16_t i =0; i < len; i++)
	{
		SPIReadWrite(*pBuf);
		pBuf++;
	}
}

void W5500IOInit()
{
	  GPIO_InitTypeDef GPIO_InitStruct = {0};

	  /* GPIO Ports Clock Enable */
	  __HAL_RCC_GPIOC_CLK_ENABLE();
	  __HAL_RCC_GPIOF_CLK_ENABLE();
	  __HAL_RCC_GPIOA_CLK_ENABLE();

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);

	  /*Configure GPIO pin : PC15 */
	  GPIO_InitStruct.Pin = GPIO_PIN_15;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  /*Configure GPIO pin : PF0 */
	  GPIO_InitStruct.Pin = GPIO_PIN_6;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void W5500Init()
{
	uint8_t tmp;
	uint8_t memsize[2][8] = {{2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};

	W5500IOInit();
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET); //CS high by defautl

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	tmp = 0xFF;
	while(tmp--);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);

	reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
	reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write);
	reg_wizchip_spiburst_cbfunc(wizchip_readburst, wizchip_writeburst);

	if(ctlwizchip(CW_INIT_WIZCHIP, (void *)memsize) == -1){
		HAL_SPI_Init(&hspi1);
	}
}
