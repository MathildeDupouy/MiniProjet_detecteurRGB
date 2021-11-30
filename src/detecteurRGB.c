// Programme de base TP II ENS
#include <stdio.h>
#include "LPC8xx.h"
#include "syscon.h"
#include "lib_ENS_II1_lcd.h"
#include "rom_api.h"
#include "ctimer.h"
#include "uart.h"
#include "utilities.h"
#include "swm.h"
#include "chip_setup.h"

void affichage(char* texte_l1, char* texte_l2);
float choix_note(char car);


#define BP1 LPC_GPIO_PORT->B0[13]
#define BP2 LPC_GPIO_PORT->B0[12]
#define LED1 LPC_GPIO_PORT->B0[19]
#define LED2 LPC_GPIO_PORT->B0[17]
#define LED3 LPC_GPIO_PORT->B0[21]
#define LED4 LPC_GPIO_PORT->B0[11]

#define enfonce 0
#define relache 1

int main(void) {
	uint8_t menu = 3;
	uint8_t menu_prec = menu;

	//Variables des etats des boutons
	uint8_t bp1 = BP1;
	uint8_t bp1_prec = BP1;
	uint8_t bp2 = BP2;
	uint8_t bp2_prec = BP2;
	//Variable pour jouer les notes
	char car_usart;
	float freq;

	//Réglage de la fréquence de l'horloge
	LPC_PWRD_API->set_fro_frequency(30000);

	// Activation du périphérique d'entrées/sorties TOR, de la switchmatrix,
	// du UART, du timer ctimer0, du multi-rate timer
	LPC_SYSCON->SYSAHBCLKCTRL0 |= GPIO0 | SWM | UART0 | CTIMER0 | MRT;

	//Configuration en sortie de la broche 19
	LPC_GPIO_PORT->DIR0 |= (1 << 19);

	//Configuration de la liaison serie
		//Horloge de la liaison serie
		 LPC_SYSCON->UART0CLKSEL = FCLKSEL_FRO_CLK;

		 //Assignation des pins T et R
		 ConfigSWM(U0_TXD, DBGTXPIN);
		 ConfigSWM(U0_RXD, DBGRXPIN);

		 // reset de UART0
		 LPC_SYSCON->PRESETCTRL0 &= (UART0_RST_N);
		 LPC_SYSCON->PRESETCTRL0 |= ~(UART0_RST_N);

		 // Configuration du générateur de baud rate de USART0
		 // 7 : 115200 baud
		 // 15 : 57600 baud
		 LPC_USART0->BRG = 15;

		 // Configuration du registre USART0 CFG:
		 // 8 bits de donnees, sans parite/parite paire, un bit de stop, pas de controle de flux, mode asynchrone
		 LPC_USART0->CFG = DATA_LENG_8|PARITY_EVEN|STOP_BIT_1;

		 // Reset au cas ou une valeur est restee
		 LPC_USART0->STAT = 0xFFFF;

		 // Activation USART0
		 LPC_USART0->CFG |= UART_EN;

	//Configuration du CTIMER

		//Reset du timer0
		LPC_CTIMER0->TCR = (1<<CRST); //Mise à 1 du rst
		LPC_CTIMER0->TCR &= ~(1<<CRST); //Mise à 0 du rst

		//Pas de prediviseur pour une meilleure precision
		LPC_CTIMER0->PR = 0;

		//Reglage du modulo du compteur et de l'action réalisée
		LPC_CTIMER0->MR[3] = (int) 15000000/440;
		LPC_CTIMER0->MCR |= (1<<MR3R); //faire reset sur la valeur de MR3
		LPC_CTIMER0->MR[1] = (int) 15000000/(2 * 440);

		//Enable du MSR
		LPC_CTIMER0->MCR |= (1<<25); //25 est MR1RL

		//Mode PWM
		LPC_CTIMER0->PWMC |= (1<<PWMEN1);

		//le PWM de la variable TO_MAT0 est assigné à un pin disponible,
		LPC_SWM->PINASSIGN4 &= ~(0xFF << 8); //Mise a zero pour pouvoir ecrire dans T0_MAT1
		LPC_SWM->PINASSIGN4 |= 19<<8;


	//Initialisation de l'afficheur lcd et affichage d'un texte
	init_lcd();
	lcd_puts("Appui sur 2 btns");
	lcd_position(1, 0);
	lcd_puts("simultanement <>");

	while (1) {
		bp1 = BP1;
		bp2 = BP2;

		//Lors d'un appui sur les deux boutons simultanement
			//Le menu change d'etat
		if(bp1 == enfonce && (bp1_prec != bp1 || bp2_prec != bp2) && bp2 == enfonce)
		{
			menu = (menu == 2 || menu == 3) ? 0 : menu + 1;
		}


		switch (menu)
				{
					//menu manuel
					case 0:
					{
						if(menu_prec != menu) affichage("Mode manuel", "note jouee : ");

						if((LPC_USART0->STAT) & RXRDY)
								{
									car_usart = LPC_USART0->RXDAT; //Lecture une seule fois
									freq = choix_note(car_usart);
									//Allumage du timer
									LPC_CTIMER0->TCR = (1<<CEN); //Mise à 1 du rst
									//Modification du shadow register
									LPC_CTIMER0->MSR[1] = (int) 15000000/(2 * freq);
								}
					}
						break;
					//menu auto
					case 1:
					{
						affichage("Mode auto", "musique : ");
					}
						break;
					//menu metronome
					case 2:
					{
						affichage("Mode metronome", "bpm : ");
					}
						break;
					default:
						break;
				}

		bp1_prec = bp1;
		bp2_prec = bp2;
		menu_prec = menu;

	} // end of while(1)

} // end of main

/**
 * Affiche du texte sur les deux lignes du lcd
 * parametres :
 *  char* texte_l1 : texte pour la 1ere ligne
 *  char* texte_l2 : texte pour la deuxieme ligne
 */
void affichage(char* texte_l1, char* texte_l2)
{
	//Ligne 1
	lcd_position(0, 0);
	lcd_puts(texte_l1);
	lcd_puts("                ");
	//Ligne 2
	lcd_position(1, 0);
	lcd_puts(texte_l2);
	lcd_puts("                ");
}

float choix_note(char car)
{
	char* note;
	float frequence;
	char texte[32];

	switch (car) {
		case 'a':
		{
			note = "DO";
			frequence = 261.63;
		}
			break;
		case 'z':
		{
			note = "RE";
			frequence = 293.66;
		}
			break;
		case 'e':
		{
			note = "MI";
			frequence = 329.63;
		}
			break;
		case 'r':
		{
			note = "FA";
			frequence = 349.23;
		}
			break;
		case 't':
		{
			note = "SOL";
			frequence = 392;
		}
			break;
		case 'y':
		{
			note = "LA";
			frequence = 440;
		}
			break;
		case 'u':
		{
			note = "SI";
			frequence = 493.88;
		}
			break;
		case 'i':
		{
			note = "DO8";
			frequence = 523.25;
		}
			break;
		default:
			break;
	}
	lcd_position(1, 0);
	sprintf(texte, "note : %3s", note);
	lcd_puts(texte);
	lcd_puts("                ");
	return frequence;
}


