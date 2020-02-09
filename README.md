# RRFTracker_Multiplex
Suivi temps réel de l'activité du réseau [RRF](https://f5nlg.wordpress.com/2015/12/28/nouveau-reseau-french-repeater-network/) (Réseau des Répéteurs Francophones). Une video du fonctionnement est visible sur [Youtube](https://www.youtube.com/watch?v=APK_efbEtOQ) ;)

## Liste des composants

Voici la liste des composants dont vous aurez besoin :

- 1 ou 2 breadboards (j'en ai utilisé 2, voir schéma)
- 1 ESP32
- 1 TCA9548A
- 5 écrans Oled type SD1106 128x64 pixels
- 3 résistances 330 ohms
- 3 LEDs de couleurs (rouge, vert, bleu...)
- quelques cables et straps

## Schéma de montage

Quelques explications :

- La sortie 22 de l'ESP32 est reliée à l'entrée SCL du TCA9548A.
- La sortie 21 de l'ESP32 est reliée à l'entrée SDA du TCA9548A.
- Les sorties 4, 5 et 14 de l'ESP32 servent pour les LEDs.
- On utilise les sorties 5V et GND de l'ESP32 pour alimenter les écrans et le TCA9548A.
- Il reste juste à cabler les sorties SC[0-7] et SD[0-7] du TCA9548A vers les entrées SCL et SDA des différents écrans Oled.


![alt text](https://github.com/armel/RRFTracker_Multiplex/blob/master/RRFTracker_multiplex.png)
