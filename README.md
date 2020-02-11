# RRFTracker_Multiplex
Suivi temps réel de l'activité du réseau [RRF](https://f5nlg.wordpress.com/2015/12/28/nouveau-reseau-french-repeater-network/) (Réseau des Répéteurs Francophones). Une video du fonctionnement est visible sur [Youtube](https://www.youtube.com/watch?v=APK_efbEtOQ) ;)

## Liste des composants

Voici la liste des composants dont vous aurez besoin :

- 1 breadboard
- 1 ESP32
- 1 TCA9548A
- 4 écrans Oled type SD1106 128x64 pixels
- 3 résistances 330 ohms
- 3 LEDs de couleurs (rouge, vert, bleu...)
- quelques cables et straps

## Schéma de montage

Quelques explications :

- La sortie 22 de l'ESP32 est reliée à l'entrée SCL du TCA9548A.
- La sortie 21 de l'ESP32 est reliée à l'entrée SDA du TCA9548A.
- Les sorties 2, 4 et 5 de l'ESP32 servent pour les LEDs.
- On utilise les sorties 5V et GND de l'ESP32 pour alimenter les écrans et le TCA9548A.
- Il reste juste à cabler les sorties SC[0-3] et SD[0-3] du TCA9548A vers les entrées SCL et SDA des différents écrans Oled.

![alt text](https://github.com/armel/RRFTracker_Multiplex/blob/master/RRFTracker_multiplex.png)

## Remarques

Rien ne vous empêche de connecter jusqu'à 8 périphériques I2C. Si vous avez des écrans en plus ou des capteurs divers, le TCA9548A est parfait pour cela.

Par convention, j'ai utilisé les couleurs suivantes pour les cables et straps :


| Couleur    |   Fonction  | 
| ---------- | ----------- | 
| Rouge      |    +5V      | 
| Noir       |   GND       |
| Vert.   	|   CLOCK     | 
| Violet     |    DATA     | 
| Orange     |    LEDs     | 
