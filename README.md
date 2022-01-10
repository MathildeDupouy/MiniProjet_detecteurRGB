# Detecteur de couleur - Mini projet Informatique Industrielle 2022
Auteur : Mathilde Dupouy
## Table of contents
* [Introduction](#introduction)
* [Technologies](#technologies)
* [Utilisation](#utilisation)
* [Réalisation](#réalisation)

## Introduction
Ce projet a pour but de réaliser un détecteur de couleur, qui peut par exemple être utile pour un utilisateur aveugle.
Ce projet comporte un fichier main.c qui permet de réaliser ces fonctions et une bibliotheque associee au capteur, TCS34725.h associee.
Il a été réalisé dans le cadre d'un mini projet d'informatique industrielle en M1 EEA à l'ENS Paris-Saclay 2021-2022.
	
## Technologies
Ce projet a été créé pour :
* Microcontrôleur : LPCXpresso804
* Ecran LCD, boutons : [extension de la LPC](https://github.com/MathildeDupouy/MiniProjet_detecteurRGB/files/7829053/Schema_Extension_LPC804_v3.pdf)
* Capteur RGB : TCS34725
* Langage utilisé : C
	
## Utilisation
Après avoir installer le projet sur la carte, un message d'accueil apparait sur l'écran LCD. Deux modes sont alors disponibles :
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

### Traduction des informations en couleurs
Il s'agit alors de transcrire l'informatio rgb en couleur. Il a été choisit aribitrairement de traduire 6 couleurs ainsi que le noir et le blanc. Dans l'espace 3D rgb, des règles empririques sur les pourcentages relatifs des canaux ont été décidées pour traduire les couleurs. 
 
