# Detecteur de couleur - Mini projet Informatique Industrielle 2022
Auteur : Mathilde Dupouy
## Table of contents
* [Introduction](#introduction)
* [Technologies](#technologies)
* [Utilisation](#utilisation)
* [Réalisation](#réalisation)
* [Pistes d'améliorations](#pistes d'améliorations)

## Introduction
Ce projet a pour but de réaliser un détecteur de couleur, qui peut par exemple être utile pour un utilisateur aveugle.
Ce projet comporte un fichier main.c qui permet de réaliser ces fonctions et une bibliotheque associee au capteur, TCS34725.h associee.
Il a été réalisé dans le cadre d'un mini projet d'informatique industrielle en M1 EEA à l'ENS Paris-Saclay 2021-2022.
	
## Technologies
Ce projet a été créé pour :
* Microcontrôleur : LPCXpresso804
* Ecran LCD, boutons : [extension de la LPC](https://github.com/MathildeDupouy/MiniProjet_detecteurRGB/files/7829053/Schema_Extension_LPC804_v3.pdf)
* Capteur RGB : TCS34725. Ce capteur détecte les couleurs avec les photodiodes de couleurs associées, et un filtre bloqueur d'infrarouge améliore la précision.
* Langage utilisé : C
	
## Utilisation
Le capteur TCS34725 a 4 fils à relier à la LPC : les deux fils de l'I2C (SDA et SCL), la masse et l'alimentation entre 2,7V et 3,6V par rapport à la masse, 3V typiquement.
Il faut ensuite installer le projet sur la carte. Un message d'accueil apparait alors sur l'écran LCD. Deux modes sont disponibles :
* Mode continu (bouton BP1) : L'appui sur le bouton 1 lance le mode. Le détecteur relève alors la couleur à intervalles réguliers et l'affiche sur l'écran LCD.
* Mode manuel (bouton BP2): Lors d'un appui sur le bouton BP2, la couleur est mesurée et affichée. Pour rafraîchir l'affichage il faut rappuyer sur le bouton.

Dans les deux cas, l'affichage est réalisé par le nom de la couleur sur la première ligne (ou "rapprochez-vous" si la couleur n'a pas pu être déterminée), et la valeur de chacun des canaux rgb lus en pourcentage.  Cela apporte une information suplémentaire à l'utilisateur.
Dans ce projet, huit "couleurs" peuvent être détectées : blanc, noir, rouge, vert, bleu, cyan, jaune et rose. Voir la section [réalisation](#réalisation) pour plus d'informations.

## Réalisation
### Liaison avec le capteur RGB
Le capteur RGB est le capteur TCS34725 commercialisé par ams. La liaison se fait via le protocole I2C et donne entre autre accès à des valeurs sur 16bits pour chacun des canaux R (rouge), G (vert), B (bleu) et C (clear, clarté). La datasheet de ce composant est disponible [ici](https://github.com/MathildeDupouy/MiniProjet_detecteurRGB/files/7829053/TCS34725.pdf).
Dans notre cas, l'initialisation du capteur consiste à activer son horloga interne et l'ADC qui permet de mesurer les valeurs RGBC.
La récupération des valeurs se fait ensuite via le protocole I2C et les spécificités du capteur détaillées dans la datasheet. En particulier, entre l'envoi de l'adresse et avant la lecture ou l'écriture, un registre de commande permet de choisir le mode et le registre concerné :
<p align="center">
  <img width="361" alt="image" src="https://user-images.githubusercontent.com/82039222/148555989-f27e20a0-67e4-4e45-aeb0-ce40f7dc0dbd.png">
</p>
Pour la lecture des couleurs, le registre appelé par le registre de commande est celui du premier canal de Clear, et on lit les huit registres suivants pour obtenir toutes les données RGBC. En effet les données d'un canal sont stockées dans deux registres de 8 bits, l'un donnant les bits de poid fort et l'autre les bits de poid faible. Les données sont ainsi sur 16 bits chacune.
Ainsi, ces fonctions ont été implémentées dans une bibliothèque dénommée TCS34725.h dans ce projet, pour clarifier le code et pour la réutilisabilité. On y trouve les fonctions suivantes, voir le code pour le détail de leur documentation et de leur implémentation :
```
void TCS_write_reg(uint8_t reg_ad, uint8_t valeur);
uint8_t TCS_read_reg(uint8_t reg_ad);
uint8_t TCS_read_color(char color);
void TCS_read_colors(uint16_t *rouge, uint16_t *vert, uint16_t *bleu, uint16_t *intensite);
```

### Traduction des informations en couleurs
Il s'agit alors de transcrire l'information rgb en couleur. Il a été choisi aribitrairement de traduire 6 couleurs ainsi que le noir et le blanc. Dans l'espace 3D rgb, des règles empririques sur les pourcentages relatifs des canaux ont été décidées pour traduire les couleurs. Le code est le suivant, où `rougep`, `vertp` et `bleup` représentent les valeurs des canaux en pourcentage et `couleur` stocke la chaîne de caractère correspondant à la couleur :

``` c
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
```

Soit :
* Si l'une des couleurs RGB est prépondérante (50% pour le rouge, 45% pour le bleu et 42% pour le vert), c'est celle qu'on affiche ;
* Si les trois couleurs sont autour de 30% (à 20% près), i.e. qu'elles sont à peu près égales, ce sera noir ou blanc en fonction de l'intensité du canal C (plus grand que 32767) ;
* sinon, si le bleu est le plus faible, on renvoie jaune ; si le vert est le plus faible on renvoie rose, et si le rouge est le plus faible on renvoie cyan.

Il n'a donc pas été pris en compte la réalité physique des valeurs détectées par le capteur par choix, mais elle intervient puisqu'on s'aperçoit que les valeurs de rouge ont tendance à être plus fortes que les autres. Cela est justifié par le constructeur avec cette courbe issue de la datasheet : 

<img width="212" alt="image" src="https://user-images.githubusercontent.com/82039222/148846415-9f3bed19-08fb-422c-8767-68b015d75d38.png">


## Pistes d'améliorations
Je propose trois pistes d'amélioration :
* Inclure plus de couleurs en affinant la classification ;
* Utiliser les interruptions disponibles sur le détecteur ainsi que les fonctionnalités de commaande pour économiser de l'énergie en n'allumant les différentes entités du composant seulement lorsqu'une détection est nécessaire ;
* Enregistrer des vocaux correspondants aux couleur dans la mémoire de la LPC pour les restituer via le haut-parleur par cohérence avec le public malvoyant visé.
