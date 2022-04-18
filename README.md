# Luchtmeter

## Concept en doelen

Het concept is om een luchtmeter te maken die de volgende waardes meet: temperatuur, luchtvochtigheid, CO2 en TVOC. Deze data moet vervolgens naar Thingspeak gestuurd worden om hieruit informatie te krijgen om de actuele luchtkwaliteit te zien van mijn slaapkamer.

![simpeldataline](/afbeeldingen/data_pipeline.png)

Als extra wil ik dat mijn IoT device makkelijk is in te stellen voor de gebruiker. Hierbij wil ik dat het Wi-Fi en api-key niet hardcoded zijn, maar juist met een interface is in te vullen.

Het ten eindelijk doel is om dit concept te kunnen gebruiken in het project van mijn studie. Ons concept is daar om een luchtmeter te maken die persoonlijk advies kan geven om de luchtkwaliteit binnenshuis te verbeteren. De luchtmeter stuurt de data naar onze server en de server stuurt 'nuttige' informatie door naar de app (client).

![simpeldataline](/afbeeldingen/simpeldataline.jpg)


## Microcontroller and sensoren

### Microcontroller

* NodeMCU (esp8266)

### Sensoren

* SGP30
* DHT11/22

## Connecties

### SGP30 sensor

 * GND on GND
 * VCC on 3.3V
 * SCL on esp8266 D1
 * SDA on esp8266 D2

### DHT11/DHT22 sensor

 * GND on GND
 * VCC on 3.3V
 * sensor to D5 / GPIO14

![ciruit-diagram](/afbeeldingen/ciruit-diagram.png)

## Installatie

Om de NodeMCU te kunnen connecten met de Aruino IDE. Moeten we via de boardermanager de NodeMCU toevoegen. Zie hier een tutorial van je hoe je dat moet doen

https://create.arduino.cc/projecthub/electropeak/getting-started-w-nodemcu-esp8266-on-arduino-ide-28184f

Wanneer dat is gedaan moeten we een paar libraries installeren. Dit doen we via de library manager.

De volgende libraries moeten we installeren:

* WiFiManager by tzapu
* ArduinoJson by bblanchon
* esp8266 by ESP8266 Community
* Adafruit SGP30 Sensor by Adafruit
* DHT sensor library by Adafruit
* Adafruit Unified Sensor by Adafruit

Als de libraries zijn geinstalleerd kunnen we de code uploaden naar de NodeMCU bord en is de luchtmeter klaar voor gebruik.

Als we het device voor de eerste keer opstarten maakt het een 'Access point' aan

![prototype](/afbeeldingen/air-meter-ap.jpeg)

Vervolgens vullen we de wi-fi gegevens in het de api-key.

![prototype](/afbeeldingen/wifiinstellen.jpeg)

Tot slot gaan we opslaan en is het IoT device ready to go.

## Inspiratie en hulpmiddelen

Het uitproberen van mijn sensoren had ik gedaan op mijn Arduino Uno bord. De libraries voor de sensoren zijn ook daarop voor gemaakt en kan dus een probleem zijn. Gelukkig was iemand mij voor en ontdekte ik dat je dat data lines van de NodeMCU kan vertalen van die van de Arduino Uno. Deze heeft de persoon in zijn readme gezet, zodat ik deze kan overnemen voor mijn eigen concept.

Voorbeeld: DHT11 sensorto d5 / GPIO14

Ik moet dus in de IDE Pinmode 14 gebruiken terwijl het geconnect is met dataline 5.

Ook zie je dat deze persoon een luchtmeter heeft gemaakt met dashboard wat al super vet is. Dit inspireerde me ten eindelijk wat ik wou doen mogelijk zou kunnen zijn.

https://github.com/paulgreg/co2-sensor

Ook zocht ik hoe ik een vriendelijk interface kon krijgen om de wi-fi in te stellen voor het Iot device. Hierop kwam ik bij een guide die uitlegde hoe je dat moest doen.

https://randomnerdtutorials.com/wifimanager-with-esp8266-autoconnect-custom-parameter-and-manage-your-ssid-and-password/

Tot slot zocht ik uit hoe je zonder de Thingspeak library een POST call kan doen naar Thingspeak. Hierbij gebruikte ik de volgende bron.

https://www.teachmemicro.com/send-sensor-data-thingspeak-esp8266/


#### Referentie lijst:

* Verbindingen sensoren voor NodeMCU ||  https://github.com/paulgreg/co2-sensor

* WifiManager library gebruiken op de NodeMCU ||  https://github.com/paulgreg/co2-sensor

* POST call naar Thingspeak || https://www.teachmemicro.com/send-sensor-data-thingspeak-esp8266/


## Het proces

### Sensoren testen op Arduino

Voordat ik met de NodeMCU aan de slag ging. Wou ik de sensoren gaan testen of dat ze werkte of niet. Ik ging het hierbij testen met de Arduino Uno microcontroller. Deze microcontroller wordt veel gebruikt in de prototype community dus had ik vertrouwen dan het testen van de sensoren soepel zou verlopen.

Het eind resultaat was dat de SGP30 (CO2 & TVOC) prima werkte. Alleen ben ik er achter gekomen dat er een kalibratie van 12 uur moet zijn, zodat de sensor optimaal kan metenl. Tot slot merkte ik dat de DHT11 sensor (tempatuur & luchtvochtigheid) constant op 22% luchtvochtheid bleef hangen. Ik kon concluderen dat deze sensor defect was, omdat ik zelf een luchtvochtigheid meter heb.

Ik had toen besloten om 2 nieuwe sensors te kopen. Dit zijn de DHT11 en DHT22. De DHT22 zou meer accuraat moeten zijn dan de DHT11. Dit kan ik testen met mijn luchtvochtigheidsmeter.

![prototype](/afbeeldingen/prototype.jpg)


### Sensoren testen op NodeMCU

Toen ik de nieuwe sensoren binnen kreeg. Ging ik aan de slag met de NodeMCU. Hierbij ging ik de sensoren koppelen om te testen of ze werkte met de NodeMCU. Snel kwam ik achter dat de datalines van de NodeMCU niet overeenkomen met de digitalen lijnen van de Arduino. De libraries die ik gebruik voor de sensoren zijn eigenlijk bedoelt voor de Arduino. Gelukkig vond ik een repostory van een persoon die ook met de NodeMCU aan de slag ging met de sensoren die ik heb. Na het aansluiten van de sensoren zag ik de deze werkte. Ook ging ik het verschil bekijken van de DHT11 en DHT22. Wat eruit kwam was dat de DHT11 7% ernaast zat en de DHT22 3%~ ernaast zat. Ik kon niet concluderen dat de DHT22 accurater was voor mijn project.

![prototype](/afbeeldingen/nodemcu-proto.jpg)

### WifiManager library

Om de data te versturen naar Thingspeak. Heb je een internetverbinding nodig. De NodeMCU werkt met een ESP8266 chip. Dit betekent dat de NodeMCU internet verbinding kan maken.

Hierbij moet je je eigen wi-fi gegevens invullen, zodat de ESP8266 kan communiceren met het internet. Dit kan je bereiken om hardcoded de gegevens van de wi-fi in te vullen.

Echter wou ik dit wat dynamischer en gebruikersvriendelijk maken, omdat mijn doelgroep niet de kennis beschikt om de hardcoded waardes te veranderen. Ook is het natuurlijk in het algemeen erg onhandig om elke keer de hardcoded waardes te veranderen om te kunnen verbinden met het internet.

Hierbij ging ik opzoek naar een library die het instellen van het wi-fi makkelijker maakt. Ik kwam snel uit bij de library 'WifiManager'. Wat WifiManager doet is op het moment dat het apparaat geen wi-fi heeft. Dat de ESP8266 een access point gaat aanmaken waardoor de gebruiker hiermee kan verbinden. Vervolgens dien je naar de interface te gaan. Hiervoor moet je naar een bepaalde url toe. Er is vervolgens een scherm waar je je wi-fi kan instellen en opslaan. Wanneer dit is gebeurd sluit het accesspoint en kan de NodeMCU beginnen met het meten van de luchtkwaliteit.

De tutorial van de WifiManager was makkelijk te volgen. Ik hard hierbij geen problemen ervaren.

![prototype](/afbeeldingen/wifiinstellen2.jpeg)

### POSTEN naar Thinkspeak

Nu dat er sensoren zijn die kunnen meten en er wi-fi verbinding is. Kan ik starten met het sturen van data naar Thingspeak. Ik wist dat er een library tot beschikking was om dit makkelijk te kunnen bereiken. Echter wil ik bij mijn studie project een eigen POST call doen naar een server. Hiervoor was de Thingspeak library geen optie, omdat dit geconfigureerd is op hun api server. Ik heb toen besloten om een eigen POST call te doen. Dit was makkelijk te bereiken, omdat de ESP8266 hiervoor een client heeft. Echter kwam ik lang vast te zitten, omdat ik continue een 400 error kreeg. Dit kwam, omdat in mijn url 'http://' in zat. Hierdoor kon de call niet gemaakt worden. Ik weet nog niet precies waarom, maar ik wil ik nog achter komen.

Na het fixen van de milde 'bug'. Kon de POST call succesvol worden gemaakt en was de data verwerkt naar informatie op Thingspeak.

![prototype](/afbeeldingen/thingspeak.png)

### Extra parameter toevoegen

Nu dat alles werkt zou je zeggen dat ik tevreden zou zijn. Dit was ik alleen nog niet. In mijn studie project moet de gebruiker een api-key instellen op het apparaat, zodat de data naar de juist endpoint kan verstuurd worden. Hierbij ging ik opzoek of ik dit in de WifiManager library kon doen. Ik kwam er toen achter dat de library custom velden tot beschikking stelt.

Alleen moeten deze velden ook worden opgeslagen. Dit gebeurd in de memory van de ESP8266. Om dit te bereiken het ik SPIFF gebruikt. Om het kort maar krachtig te houden maakt het een .json bestand aan met de parameter. Wanneer het apparaat uit gaat verliest het niet de api-key en kan het gebruikt worden wanneer het apparaat weer aan staat.

![prototype](/afbeeldingen/customparam.jpeg)

## Uitwerking

Om de uitwerking te demonstreren heb ik een korte video gemaakt. Hier leg ik uit wat er gebeurd en welke features in mijn IoT device zitten.

https://youtu.be/iFWaCIq1BAs

De wi-fi instellen gedeelte heb ik niet opgenomen. Dit vanwege privacy redenen. Wel kan ik dit uitleggen middels afbeeldingen die ik hebt gemaakt.

Als we het device voor de eerste keer opstarten maakt het een 'Access point' aan

![prototype](/afbeeldingen/air-meter-ap.jpeg)

Vervolgens vullen we de wi-fi gegevens in het de api-key

![prototype](/afbeeldingen/wifiinstellen.jpeg)

Tot slot gaan we opslaan en is het IoT device ready to go.

## Wat nu?

Nu dat ik een werkend prototype heb. Ga ik dit gebruik voor mijn studie project. Het enige wat ik moet veranderen in mijn code is de endpoint van waar de POST zich moet plaatsvinden.

Wat nog een toegevoegde waarde kan creëren is om een reset knop te plaatsen. Met deze knop verwijder je het wi-fi netwerk en de api-key. Op deze manier kan het apparaat makkelijk worden opnieuw ingesteld.

Tot slot moet er ook een casing zijn met een batterij, zodat deze ergens draadloos geplaatst kan worden. Wel moet ik onderzoeken hoeveel stroom de NodeMCU gebruikt als een POST doet.

## Conclusie

In dit keuzevak heb ik geleerd hoe ik data moet interpreteren. Data kunnen normale mensen niet begrijpen tenzij je het gaat omzetten naar informatie. Dit heb ik eigenlijk gedaan met mijn prototype. Je kan via Thingspeak de diagrammen zien en dan wat de trend is van deze waardes. Je kan dan makkelijk zien en begrijpen wanneer iets goed of slecht gaat.

Tot slot heb ik veel aan dit keuzevak gehad. In mijn achterhoofd had ik het project van mijn studie waarbij ik ten eindelijk een luchtmeter zou gaan maken. Hierbij kwam dit keuzevak enorm van pas, omdat ik zowel mijn leer tijd kan gebruiken voor het keuzevak en mijn project van de studie.

2 jaar terug probeerde ik dit project als hobby. Dit lukte helaas niet, omdat ik toen nog niet genoeg programmeer/hardware kennis had. Nu lukt dit project wel en ben ik daar erg trots op. Het is zo gigantisch tof om je eigen gemeten waardes te versturen naar het internet en ten eindelijk op de back-end er van alles en nog wat mee te kunnen doen!
