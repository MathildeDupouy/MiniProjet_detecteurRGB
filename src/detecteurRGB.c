// Programme de base TP II ENS
#include <stdio.h>
#include "LPC8xx.h"
#include "syscon.h"
#include "lib_ENS_II1_lcd.h"
#include "i2c.h"
#include "mrt.h"
#include "rom_api.h"
#include "utilities.h"
#include "swm.h"
#include "chip_setup.h"

#include "TCS34725.h"

//Registres lies a l'extension
#define BP1 LPC_GPIO_PORT->B0[13]
#define BP2 LPC_GPIO_PORT->B0[12]
#define LED1 LPC_GPIO_PORT->B0[19]
#define LED2 LPC_GPIO_PORT->B0[17]
#define LED3 LPC_GPIO_PORT->B0[21]
#define LED4 LPC_GPIO_PORT->B0[11]

#define enfonce 0
#define relache 1


#define PERIODE_MESURE 0.1 //Une mesure toutes les PERIODE_MESURE secondes

//Initialisation variables globales
	//Variable des couleurs lues
	uint16_t rouge = 0, bleu = 0, vert = 0, intensite = 0;

	uint32_t compteur = 0;



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

/**
 * Traduit les valeurs des canaux rgb et de la luminosite et stocke la couleur en texte
 * Entrees : 	rouge, vert et bleu : valeurs des canaux rgb respectivement
 * 				intensite 			: valeurs du canal de l'intensite lumineuse
 * 				couleur				: pointer vers ou stocker le texte de la couleur
 */
void traduction_color(uint16_t rouge, uint16_t vert, uint16_t bleu, uint16_t intensite, char** couleur)
{
	char texte_couleur[16];
	*couleur = "Rapprochez-vous svp";

	//Traduction en pourcentage
	float somme = rouge + vert + bleu;
	float rougep =  rouge / somme*100;
	float bleup = bleu / somme*100;
	float vertp = vert / somme*100;

	if (rougep > 50) *couleur = "rouge";
	else if (bleup > 45)  *couleur = "bleu";
	else if (vertp > 42)  *couleur = "vert";

	else if ((rougep >= 20) && (rougep <= 40)) //Rouge vaut 30% a dix pourcents pres
	{
		if ((vertp >= 20) && (vertp <= 40)) //vert vaut 30% a dix pourcents pres
		{
			*couleur = (intensite > 32767) ? "blanc" : "noir";
		}
	}
	else if(bleup <25) *couleur = "jaune";
	else if(vertp <25) *couleur = "rose";
	else if(rougep <25) *couleur = "cyan";

	sprintf(texte_couleur, "r%d%% v%d%% b%d%%", (int) (rougep+0.5), (int) (vertp+0.5), (int) (bleup+0.5));
	affichage(*couleur, texte_couleur);
}


void MRT_IRQHandler(void)
{
	//Lecture des canaux du detecteur
	TCS_read_colors(&rouge, &vert, &bleu, &intensite);
	compteur++;
	LPC_MRT->Channel[0].STAT |= (1 << MRT_INTFLAG); //On abaisse le drapeau
}

int main(void) {
	//Variables
		//Variables pour les etats des boutons
		uint8_t bp1 = BP1;
		uint8_t bp1_prec = BP1;
		uint8_t bp2 = BP2;
		uint8_t bp2_prec = BP2;
		//Variable des couleurs lues precedemment
		uint16_t rouge_prec = rouge, bleu_prec = bleu, vert_prec = vert, intensite_prec = intensite;
		char couleur[16];

	//Configuration de l'horloge à 15 MHz
		LPC_PWRD_API->set_fro_frequency(30000);

	// Peripheral reset to the GPIO0 and pin interrupt modules. '0' asserts, '1' deasserts reset.
		LPC_SYSCON->PRESETCTRL0 &=  (GPIO0_RST_N & GPIOINT_RST_N);
		LPC_SYSCON->PRESETCTRL0 |= ~(GPIO0_RST_N & GPIOINT_RST_N);

	//Mise en fonctionnement des périphériques utilisés
		LPC_SYSCON->SYSAHBCLKCTRL0 |= (IOCON | GPIO0 | SWM | CTIMER0 | GPIO_INT | MRT);

	//Configuration du MRT
		//Reset du MRT
		LPC_SYSCON->PRESETCTRL0 &= MRT_RST_N;
		LPC_SYSCON->PRESETCTRL0 |= ~MRT_RST_N;
		//mode repeat interrupt
		//LPC_MRT->Channel[0].CTRL |= (1 << MRT_INTEN);
		LPC_MRT->Channel[0].CTRL &= ~(1 << MRT_MODE);
		//Tempo du MRT (apres initialisation LCD car fait appel a l'I2C)
		//LPC_MRT->Channel[0].INTVAL =  (uint32_t) 15000000 * PERIODE_MESURE ;
		//LPC_MRT->Channel[0].INTVAL |=  ForceLoad;

	//Confuguration interruption MRT
		NVIC->ISER[0] = (1<<10); //enable interruption MRT
		NVIC->IP[2] &= ~(0b11 << 22); //Priorite maximale pour l'interruption du mrt

		//Initialisation de l'afficheur lcd et affichage d'un texte
		init_lcd();
		affichage("Detecteur colors", "appuyez sur BP2!");
		TCS_write_reg(TCS34725_ENABLE, (1 << AEN) | (1 << PON));

	//Enabale du  MRT
		//LPC_MRT->Channel[0].CTRL |= (1 << MRT_INTEN);

	while (1) {
		bp1 = BP1;
		bp2 = BP2;


		if(bp2==enfonce && bp2_prec != bp2)
		{
			//arret de la mesure continue
			LPC_MRT->Channel[0].CTRL &= ~(1 << MRT_INTEN);
			//Lecture des canaux du detecteur
			TCS_read_colors(&rouge, &vert, &bleu, &intensite);
		}
		if (bp1==enfonce && bp1_prec != bp1)
		{
			//Mise en marche du timer
			LPC_MRT->Channel[0].CTRL |= (1 << MRT_INTEN);
			LPC_MRT->Channel[0].INTVAL =  (uint32_t) 15000000 * PERIODE_MESURE ;
			LPC_MRT->Channel[0].INTVAL |=  ForceLoad;
		}
		//Affichage lors d'un changement des valeurs rgb (variables globales)
		if ((rouge != rouge_prec) || (vert != vert_prec) || (bleu != bleu_prec) || (intensite != intensite_prec))
		{
			traduction_color(rouge, vert, bleu, intensite, &couleur);
			rouge_prec = rouge;
			bleu_prec = bleu;
			vert_prec = vert;
			intensite_prec = intensite;
		}


		bp1_prec = bp1;
		bp2_prec = bp2;
	} // end of while(1)

} // end of main
