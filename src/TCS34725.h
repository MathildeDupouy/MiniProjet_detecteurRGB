/*!
 *  Informatique industrielle mini-projet 2021
 *  Auteur : Mathilde Dupouy
 *
 *  Pour la définition de tous les registres :
 *  Copyright (c) 2013, Adafruit Industries
 *  All rights reserved.
 *
 */
#ifndef _TCS34725_H_
#define _TCS34725_H_


#define TCS34725_ADDRESS 0x52 //(0x29)     //I2C address 0b01010010

#define TCS34725_ENABLE 0x00
#define AEN 1
#define PON 0

#define TCS34725_CDATAL (0x14) // Intensite canal bits de poids faibles
#define TCS34725_CDATAH (0x15) // Intensite canal bits de poids forts
#define TCS34725_RDATAL (0x16) // Rouge canal bits de poids faibles
#define TCS34725_RDATAH (0x17) // Rouge canal bits de poids forts
#define TCS34725_GDATAL (0x18) // Vert canal bits de poids faibles
#define TCS34725_GDATAH (0x19) // Vert canal bits de poids forts
#define TCS34725_BDATAL (0x1A) // Bleu canal bits de poids faibles
#define TCS34725_BDATAH (0x1B) // Bleu canal bits de poids forts



/**
 * Ecrire une valeur dans un registre du TCS34725
 * Le type est par défaut : Repeated byte protocol transaction
 * reg_ad : adresse du registre que l'on veut appeler, sur 5bits
 * valeur : valeur a ecrire dans le registre
 */
void TCS_write_reg(uint8_t reg_ad, uint8_t valeur);

/**
 * Lire une valeur dans un registre du TCS34725
 * Le type est par défaut : Auto increment byte protocol transaction
 * reg_ad : adresse du registre que l'on veut appeler, sur 5bits
 * valeur : valeur a ecrire dans le registre
 */
uint8_t TCS_read_reg(uint8_t reg_ad);

/**
 * Lecture de la couleur demandee en argument
 * color   :   caractere 'r' (rouge), 'v' (vert), 'b' (bleu)
 */
uint8_t TCS_read_color(char color);

/**
 * Lecture des couleurs
 * Entrees : intensite, rouge, vert et bleu   :  pointeur vers ou stocker les couleurs et l'intensite
 */
void TCS_read_colors(uint16_t *rouge, uint16_t *vert, uint16_t *bleu, uint16_t *intensite);


#endif
