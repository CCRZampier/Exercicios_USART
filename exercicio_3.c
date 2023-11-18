#include "stm32f10x.h"
#include <string.h>

#define SIZE 12
char MainBuf[SIZE];

void USART_Send_String(uint8_t * str) {
	for (uint8_t i = 0; i < *str; i++) {
		USART2->DR = str[i];
		while ((USART2->SR & USART_SR_TC) == 0) {}
		USART2->SR &= ~USART_SR_TC;
	}
}

void SysTick_Handler() {
	// Caso o botão 1, conectado em PA7, esteja apertado.
	// Envia a frase "switch1 on\n" pela serial.
	if ((GPIOA->IDR & GPIO_IDR_IDR7) == 0)
		USART_Send_String("switch1 on\n");
}

void Check_Command() {
	if (strcmp(MainBuf, "led1 on") == 0) {
		GPIOB->ODR &= ~GPIO_ODR_ODR11;
	} else if (strcmp(MainBuf, "led1 off") == 0) {
		GPIOB->ODR |= GPIO_ODR_ODR11;
	}
}

void USART2_IRQHandler() {
	uint8_t RxData = USART2->DR;
	USART2->SR &= ~USART_SR_RXNE;
	if (RxData == '\n') {
		Check_Command();
		memset(MainBuf, 0, sizeof MainBuf);
		return;
	}
	char tmpBuf[2] = { RxData, '\0' };
	strcat(MainBuf, tmpBuf);
}

int main() {
	// Habilita a USART2, GPIOA e GPIOB.
	RCC->APB1ENR = RCC_APB1ENR_USART2EN;
	RCC->APB2ENR = RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;

	// Configura PB11 como saída.
	GPIOB->CRH &= ~(GPIO_CRH_MODE11 | GPIO_CRH_CNF11);
	GPIOB->CRH |= (GPIO_CRH_MODE11);

	// Configura PA2 e PA3 como saída (TX) e entrada (RX) respectivamente.
	GPIOA->CRL &= ~(GPIO_CRL_MODE2 | GPIO_CRL_CNF2)
			& ~(GPIO_CRL_MODE3 | GPIO_CRL_CNF3);
	GPIOA->CRL |= (GPIO_CRL_MODE2 | GPIO_CRL_CNF2_1) | GPIO_CRL_CNF3_1
			| GPIO_CRL_CNF7_1;

	// Habilita o pull-up do PA3 e PA7.
	GPIOA->ODR |= GPIO_ODR_ODR3 | GPIO_ODR_ODR7;

	// Habilita a USART, TX, RX e interrupção do RX.
	USART2->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_UE | USART_CR1_RXNEIE;

	// Configura o Baud Rate.
	USART2->BRR = 8000000 / 9600;

	// Habilita a interrupção da USART2;
	// NVIC é um registrador para configurar interrupções. ISET é o registrador
	// para habilitar essas interrupções, esse é um vetor de tamanho 8 de 32 bits.
	// USART2_IRQn é um macro para o número 38, o qual habilita as interrupções
	// da USART2.
	// Portanto, para fazer a configuração adequada, é necessário descobrir a
	// posição no vetor ele está, através da divisão inteira do seu valor IRQn por
	// 32, e dar o shift até a posição do bit que está, obtido com o módulo, ou
	// seja, o resto da divisão do valor de IRQn por 32.
	//
	// Para mais informações veja:
	// Capítulo 10.1.2 do manual de referência (RM0008) para microcontroladores STM32.
	// https://developer.arm.com/documentation/107706/0100/Exceptions-and-interrupts-overview/NVIC-registers-for-interrupt-management
	NVIC->ISER[USART2_IRQn / 32] |= 1 << (USART2_IRQn % 32);

	// Configura o SysTick para gerar uma interrupção a cada 200 ms.
	SysTick->LOAD = 2000000 - 1;
	SysTick->CTRL = SysTick_CTRL_ENABLE | SysTick_CTRL_TICKINT;

	while (1) {}
}
