# 2022_CHF_SwissCanSat
Projet SwissCanSat concernant la partie électronique. 
Elèves: Daniel Kopcalija, Akil Studer

- programme_principale : programme développé pour tester la cannette avec tous les composant

- programme_callback : programme utilisé pour les lancés (version amélioré du programme_principale)

- programmme_principal_rcepteur : programme utilisé pour la réception des données

- Test : tous les programmes utilisé pour tester les composants lors du choix des composants
    1. TEST BUZZER :
          pirezo receiver : programme pour allumer un buzzer a distance - récepteur (avec le buzzer)
          piezo sender : programme pour allumer un buzzer a distance - émetteur
          receiver callback : programme pour allumer un buzzer a distance avec un callback - récepteur (avec le buzzer)
          sender callback : programme pour allumer un buzzer a distance avec un callback - émetteur
    2. TEST carte SD 
          SD_card : programme pour écrire dans un fichier txt sur la carte SD
          readSDcard : programme pour lire un fichier txt sur la carte SD
    3. Test des capteurs :
          codeTestBME280 : programme de base pour utiliser le capteur BME280
          MS8607 : programme de base pour utilise le capteur MS8607
          TEST GPS : programme pour tester le GPS
          TEST capteur CO2 : programme pour tester le capteur SCD41
          TemperatureSensorTestMainProgram : programme utilisé por tester les capteurs de température (BME280, TMP177, MS8607)
          Test capteur humidité : programme utilisé pour tester les capteurs d'humidité (BME280, MS8607, SHT31, SCD41)
          Test capteur pression : programme utilisé pour testé les capteurs de pression (BME280, BMP280, MS8607)

- PCB : fichiers pour imprimer les PCB
