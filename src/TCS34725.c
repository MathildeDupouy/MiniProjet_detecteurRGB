/*!
 *  Informatique industrielle mini-projet 2022
 *  Auteur : Mathilde Dupouy
 *
 *  Bibliotheque pour le detecteur de couleur TCS34725
 */

#include <math.h>
#include <stdlib.h>
#include "utilities.h"
#include "swm.h"
#include "chip_setup.h"
#include "lib_ENS_II1_lcd.h"
#include "i2c.h"
#include "TCS34725.h"

/**
 * Ecrire une valeur dans un registre du TCS34725
 * Le type est par défaut : Repeated byte protocol transaction
 * reg_ad : adresse du registre que l'on veut appeler, sur 5bits
 * valeur : valeur a ecrire dans le registre
 */
void TCS_write_reg(uint8_t reg_ad, uint8_t valeur){

	uint8_t I2CMasterBuffer[3]; // ad, #reg, valeur
	uint8_t I2CWriteLength=2;
	I2CMasterBuffer[0]=TCS34725_ADDRESS;

	//Command code
	I2CMasterBuffer[1]=(0b100 << 5) | reg_ad; //CMD 1, TYPE 00, ADDR = 0b100AD
	I2CMasterBuffer[2]=valeur;
	I2CmasterWrite(I2CMasterBuffer, I2CWriteLength );
}

/**
 * Lire une valeur dans un registre du TCS34725
 * Le type est par défaut : Auto increment byte protocol transaction
 * reg_ad : adresse du registre que l'on veut appeler, sur 5bits
 * valeur : valeur a ecrire dans le registre
 */
uint8_t TCS_read_reg(uint8_t reg_ad){
	uint8_t I2CMasterBuffer[2]; // ad, #reg
	uint8_t I2CSlaveBuffer[1];
	uint8_t I2CWriteLength=1;
	uint8_t I2CReadLength=1;
	I2CMasterBuffer[0]=TCS34725_ADDRESS;
	I2CMasterBuffer[1]=(0b101 << 5) | reg_ad; //Command, auto-increment protocol, adresse du registre
	I2CmasterWriteRead( I2CMasterBuffer, I2CSlaveBuffer, I2CWriteLength, I2CReadLength );
	return I2CSlaveBuffer[0];
}


/**
 * Lecture de la couleur demandee en argument
 * color   :   caractere 'r' (rouge), 'v' (vert), 'b' (bleu)
 */
uint8_t TCS_read_color(char color){
	uint8_t reg_ad;
	uint8_t valeur = 0;

	switch (color) {
		case 'r':
			reg_ad = TCS34725_RDATAL; //low_byte
			break;
		case 'v':
			reg_ad = TCS34725_GDATAL;
			break;
		case 'b':
			reg_ad = TCS34725_BDATAL;
			break;
		default:
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
	uint8_t reg_ad = TCS34725_CDATAL; //Adresse du bit de la donnée de clarte

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
