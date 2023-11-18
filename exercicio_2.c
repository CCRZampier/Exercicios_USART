#include "stm32f10x.h"

void SysTick_Handler() {
	// Envia o caractere 'T' pela serial.
	USART2->DR = 'T';
}

int main() {
	// Habilita a USART2 e GPIOA.
	RCC->APB1ENR = RCC_APB1ENR_USART2EN;
	RCC->APB2ENR = RCC_APB2ENR_IOPAEN;

	// Configura PA2 e PA3 como saída (TX) e entrada (RX) respectivamente.
	GPIOA->CRL &= ~(GPIO_CRL_MODE2 | GPIO_CRL_CNF2)
			& ~(GPIO_CRL_MODE3 | GPIO_CRL_CNF3);
	GPIOA->CRL |= (GPIO_CRL_MODE2 | GPIO_CRL_CNF2_1)
			| (GPIO_CRL_CNF3_1);

	// Habilita o pull-up do PA3.
	GPIOA->ODR |= GPIO_ODR_ODR3;

	// Habilita a USART2, TX, RX e PCE. Configura para 9 bits de dados.
	USART2->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_PCE | USART_CR1_UE
			| USART_CR1_M;

	// Configura o Baud Rate.
	USART2->BRR = 8000000 / 19200;

	// Configura o SysTick para gerar uma interrupção a cada 500 ms.
	SysTick->LOAD = 5000000 - 1;
	SysTick->CTRL = SysTick_CTRL_ENABLE | SysTick_CTRL_TICKINT;

	while (1) {}
}
