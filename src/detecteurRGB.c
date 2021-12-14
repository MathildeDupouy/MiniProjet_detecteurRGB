// Programme de base TP II ENS
#include <stdio.h>
#include "LPC8xx.h"
#include "syscon.h"
#include "lib_ENS_II1_lcd.h"
#include "i2c.h"
#include "rom_api.h"
#include "utilities.h"
#include "swm.h"
#include "chip_setup.h"

#include "TCS34725.h"


#define BP1 LPC_GPIO_PORT->B0[13]
#define BP2 LPC_GPIO_PORT->B0[12]
#define LED1 LPC_GPIO_PORT->B0[19]
#define LED2 LPC_GPIO_PORT->B0[17]
#define LED3 LPC_GPIO_PORT->B0[21]
#define LED4 LPC_GPIO_PORT->B0[11]

#define enfonce 0
#define relache 1

uint8_t erreur_demande = 0;

/**
 * Le type est par défaut : Repeated byte protocol transaction
 * reg_ad : adresse du registre que l'on veut appeler, sur 5bits
 * valeur : valeur a ecrire dans le registre
 */
void TCS_write_reg(uint8_t reg_ad,uint8_t valeur){

	uint8_t I2CMasterBuffer[3]; // ad, #reg, valeur
	uint8_t I2CWriteLength=2;
	I2CMasterBuffer[0]=TCS34725_ADDRESS;

	//Command code
	I2CMasterBuffer[1]=(0b100 << 5) | reg_ad; //CMD 1, TYPE 00, ADDR = 0b100AD
	//ADDRID = 0b1001 0010 = 92 (AD 0x12)
	//ADDRRed = 0b1001 0110 = 96 (AD 0x16)
	I2CMasterBuffer[2]=valeur;
	I2CmasterWrite(I2CMasterBuffer, I2CWriteLength );
}

uint8_t TCS_read_reg(uint8_t reg_ad){
	uint8_t I2CMasterBuffer[2]; // ad, #reg
	uint8_t I2CSlaveBuffer[1];
	uint8_t I2CWriteLength=1;
	uint8_t I2CReadLength=1;
	I2CMasterBuffer[0]=TCS34725_ADDRESS;
	I2CMasterBuffer[1]=(0b100 << 5) | reg_ad;
	I2CmasterWriteRead( I2CMasterBuffer, I2CSlaveBuffer, I2CWriteLength, I2CReadLength );
	// autre possibilité :
//	I2CmasterWrite(I2CMasterBuffer, I2CWriteLength );
//	I2CmasterRead( MCP23_I2C_AD, I2CSlaveBuffer, I2CReadLength );
	return I2CSlaveBuffer[0];
}


/**
 * Lecture d'une couleur
 * color   :   caractere 'r' (rouge), 'v' (vert), 'b' (bleu)
 */
uint8_t TCS_read_color(char color){
	uint8_t reg_ad;
	uint8_t valeur = 0;

	switch (color) {
		case 'r':
			reg_ad = 0x16; //low_byte
			break;
		case 'v':
			reg_ad = 0x18;
			break;
		case 'b':
			reg_ad = 0x1A;
			break;
		default:
			erreur_demande = 1;
			break;
	}
	if (reg_ad != 0)
	{
		valeur = TCS_read_reg(reg_ad);
	}
	return valeur;
}

/**
 * Lecture des couleurs
 * Entrees : intensite, rouge, vert et bleu   :  pointeur vers ou stocker les couleurs et l'intensite
 */
void TCS_read_colors(uint16_t *rouge, uint16_t *vert, uint16_t *bleu, uint16_t *intensite)
{
	uint8_t reg_ad = 0x14; //Adresse du bit de la donnée de clarte

	uint8_t I2CMasterBuffer[2]; // ad, #reg
	uint8_t I2CSlaveBuffer[1];
	uint8_t I2CWriteLength=1;
	uint8_t I2CReadLength=8;
	I2CMasterBuffer[0]=TCS34725_ADDRESS;
	I2CMasterBuffer[1]=(0b101 << 5) | reg_ad; //Command, auto-increment protocol, adresse du registre
	I2CmasterWriteRead( I2CMasterBuffer, I2CSlaveBuffer, I2CWriteLength, I2CReadLength );

	//Recuperation des donnees (rgb avec lowbyte puis high byte a chaque fois)
	*intensite = (I2CSlaveBuffer[1]<<8) | I2CSlaveBuffer[0];
	*rouge = (I2CSlaveBuffer[3]<<8) | I2CSlaveBuffer[2];
	*vert = (I2CSlaveBuffer[5]<<8) | I2CSlaveBuffer[4];
	*bleu = (I2CSlaveBuffer[7]<<8) | I2CSlaveBuffer[6];

}


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

	if (rougep > 55) *couleur = "rouge";
	else if (bleup > 45)  *couleur = "bleu";
	else if (vertp > 45)  *couleur = "vert";

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

	sprintf(texte_couleur, "r%d%% v%d%% b%d%%", (int)rougep, (int)vertp, (int)bleup);
	affichage(*couleur, texte_couleur);
}

int main(void) {
	//Variables pour les etats des boutons
	uint8_t bp1 = BP1;
	uint8_t bp1_prec = BP1;
	uint8_t bp2 = BP2;
	uint8_t bp2_prec = BP2;


	//Variable des couleurs lues
	uint16_t rouge, bleu, vert, intensite;
	char couleur[16];

	//Configuration de l'horloge à 15 MHz
		LPC_PWRD_API->set_fro_frequency(30000);

	// Peripheral reset to the GPIO0 and pin interrupt modules. '0' asserts, '1' deasserts reset.
		LPC_SYSCON->PRESETCTRL0 &=  (GPIO0_RST_N & GPIOINT_RST_N);
		LPC_SYSCON->PRESETCTRL0 |= ~(GPIO0_RST_N & GPIOINT_RST_N);

	//Mise en fonctionnement des périphériques utilisés
		LPC_SYSCON->SYSAHBCLKCTRL0 |= (IOCON | GPIO0 | SWM | CTIMER0 | GPIO_INT);


	//Initialisation de l'afficheur lcd et affichage d'un texte
	init_lcd();
	affichage("Detecteur colors", "appuyez sur BP2!");
	TCS_write_reg(0x00, 0b00000011);

	while (1) {
		bp1 = BP1;
		bp2 = BP2;

		if(bp2==enfonce && bp2_prec != bp2)
		{
			//Lecture des canaux du detecteur
			TCS_read_colors(&rouge, &vert, &bleu, &intensite);
			traduction_color(rouge, vert, bleu, intensite, &couleur);

		}
		bp1_prec = bp1;
		bp2_prec = bp2;
	} // end of while(1)

} // end of main
