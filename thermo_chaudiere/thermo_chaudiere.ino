#include <Wire.h>
#include <SFE_BMP180.h>
#include <RtcDS1307.h>

#define RELAIS     8
#define MINIMUM   21
#define CONFORT   23

#define MSECOND  1000UL
#define MMINUTE  60*MSECOND
#define MHOUR    60*MMINUTE
#define MDAY     24*MHOUR

SFE_BMP180 capteur;
RtcDS1307<TwoWire> Rtc(Wire);

unsigned long previousMillis = 0;

//  Programme de chauffage
unsigned short programmes[][5] = {
  {0,  6,  0, 22, 30},  // Dimanche     6h-22h30
  {1,  6,  0,  8, 30},  // Lundi        6h-8h30
  {1, 14, 30, 22, 30},  // Lundi     14h30-22h30
  {2,  6,  0,  8, 30},  // Mardi        6h-8h30
  {2, 17, 30, 22, 30},  // Mardi     17h30-22h30
  {3,  6,  0, 10, 30},  // Mercredi     6h-10h30
  {3, 17, 30, 22, 30},  // Mercredi  17h30-22h30
  {4,  6,  0, 10,  0},  // Jeudi        6h-10h00
  {4, 15,  0, 22, 30},  // Jeudi     15h00-22h30
  {5,  6,  0,  8, 30},  // Vendredi     6h-8h30
  {5, 17, 30, 22, 30},  // Vendredi  17h30-22h30
  {0,  6,  0, 22, 30}   // Samedi       6h-22h30
};

// Test si en plage de confort (présence)
bool enConfort(void) {
  bool active = 0;
  char datestring[32];
  RtcDateTime dt;

  if(!Rtc.IsDateTimeValid())
    return(0);

  dt = Rtc.GetDateTime();

  snprintf(datestring,
    sizeof(datestring)/sizeof(datestring[0]),
    "%02u %02u/%02u/%04u %02u:%02u:%02u",
    dt.DayOfWeek(),
    dt.Day(),
    dt.Month(),
    dt.Year(),
    dt.Hour(),
    dt.Minute(),
    dt.Second());
  Serial.print(datestring);

  unsigned int nbr_prog = sizeof(programmes)/sizeof(programmes[0][0])/5;
  for(int i=0; i<nbr_prog; i++) {
    if(dt.DayOfWeek() == programmes[i][0]) {
      if((dt.Hour()>programmes[i][1] || (dt.Hour()>=programmes[i][1] && dt.Minute()>=programmes[i][2])) &&
         (dt.Hour()<programmes[i][3] || (dt.Hour()<=programmes[i][3] && dt.Minute()<=programmes[i][4])))
        return(1);
    }
  }
  return(0);
}

// test si température à la consigne
bool doitChauffe(int consigne) {
  double T;
  short ret;

  if(ret=capteur.startTemperature() != 0) {
    delay(ret);
    if(capteur.getTemperature(T) != 0) {
      Serial.print("    température: ");
      Serial.print(T,2);
      Serial.print("°C");
      if(T<consigne)
        return(1);
    } else {
      Serial.print("Erreur lecture température");
    }
  } else {
    Serial.print("Erreur mesure température");
  }
  return(0);
}

// Test de l'environnement
void domoTest(void) {

  if(enConfort()) {
    Serial.print("   CONFORT(");
    Serial.print(CONFORT);
    Serial.print(")");
    if(doitChauffe(CONFORT)) {
      Serial.print("   CHAUFFE");
      digitalWrite(RELAIS, LOW);
    } else {
      Serial.print("   PAS CHAUFFE");
      digitalWrite(RELAIS, HIGH);
    }
  } else {
    Serial.print("   MINIMUM(");
    Serial.print(MINIMUM);
    Serial.print(")");
    if(doitChauffe(MINIMUM)) {
      Serial.print("   CHAUFFE");
      digitalWrite(RELAIS, LOW);
    } else {
      Serial.print("   PAS CHAUFFE");
      digitalWrite(RELAIS, HIGH);
    }
  }

  Serial.println("");
}

void setup() {
  // configuration relais
  digitalWrite(RELAIS, HIGH);
  pinMode(RELAIS, OUTPUT);

  // configuration port série
  Serial.begin(115200);

  // configuration capteur température
  if(!capteur.begin()) {
    Serial.println("Erreur BMP180 init");
    while(1);
  }

  // configuration horloge
  Rtc.Begin();
  if (!Rtc.GetIsRunning())
    Rtc.SetIsRunning(true);

  if(!Rtc.IsDateTimeValid()) {
    Serial.println("Erreur DS1307 init");
    while(1);
  }

  // test environnement
  domoTest();
}

void loop() {
  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis >= MMINUTE) {
    previousMillis = currentMillis;
    domoTest();
  }
}
