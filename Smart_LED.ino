#include <Matter.h>
#include <MatterLightbulb.h>

#define RED_PIN 6    // PWM-Pin für die rote LED
#define GREEN_PIN 9  // PWM-Pin für die grüne LED
#define BLUE_PIN 10  // PWM-Pin für die blaue LED

MatterColorLightbulb matter_color_bulb;  // Definiert das Matter-Objekt für die Glühbirne
volatile bool button_pressed = false;    // Speichert den Status des Buttons (gedrückt/nicht gedrückt)

void setup() {
  // Diese Funktion initialisiert die serielle Kommunikation, das Matter-Protokoll
  // und die RGB-LED-Steuerung. Sie überprüft auch, ob das Gerät bereits mit dem Matter-Hub
  // gekoppelt ist. Falls nicht, werden Pairing-Informationen bereitgestellt.
  
  Serial.begin(115200);  
  Matter.begin();        
  matter_color_bulb.begin();  

  // Hier wird der eingebaute Button konfiguriert, und ein Interrupt wird eingerichtet,
  // um auf Knopfdruck zu reagieren.
  pinMode(BTN_BUILTIN, INPUT_PULLUP);  
  attachInterrupt(digitalPinToInterrupt(BTN_BUILTIN), handle_button_press, FALLING);

  // Die Pins für die roten, grünen und blauen LEDs werden als Ausgang definiert,
  // und die LEDs werden zunächst ausgeschaltet.
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  setColor(0, 0, 0);  // Startet mit ausgeschalteten RGB-LEDs (alle Farben aus)

  // Falls das Gerät nicht mit einem Matter-Hub verbunden ist, gibt es die Pairing-Informationen
  // aus und wartet auf die Kommissionierung und Verbindung.
  if (!Matter.isDeviceCommissioned()) {
    Serial.printf("Manueller Pairing-Code: %s\n", Matter.getManualPairingCode().c_str());
    Serial.printf("QR-Code-URL: %s\n", Matter.getOnboardingQRCodeUrl().c_str());
    waitForCommissioning();
  }

  // Wartet darauf, dass das Gerät online und im Matter-Netzwerk verfügbar ist.
  Serial.println("Warte auf Matter-Geräteerkennung...");
  while (!matter_color_bulb.is_online()) delay(200);
  Serial.println("Matter-Gerät ist jetzt online");
}

void loop() {
  // Diese Funktion prüft kontinuierlich, ob der Button gedrückt wurde, um das Licht
  // ein- oder auszuschalten. Außerdem werden die RGB-Werte aus dem Matter-Netzwerk abgerufen,
  // wenn das Licht eingeschaltet ist, und auf die LEDs angewendet.
  
  if (button_pressed) {
    button_pressed = false;  
    matter_color_bulb.toggle(); 
  }

  // Überprüft, ob das Licht eingeschaltet ist und wendet entsprechende RGB-Werte an,
  // oder schaltet die LEDs aus, wenn das Licht aus ist.
  bool light_state = matter_color_bulb.get_onoff();
  if (light_state) {
    setRGBColorFromMatter();
  } else {
    setColor(0, 0, 0);  
  }
}

void setRGBColorFromMatter() {
  // Diese Funktion ruft die aktuellen RGB-Werte aus dem Matter-Netzwerk ab
  // und skaliert sie, bevor sie an die LEDs gesendet werden, um die Helligkeit zu steuern.
  
  uint8_t r, g, b;
  matter_color_bulb.get_rgb(&r, &g, &b);  // Holt die RGB-Werte von Matter

  // Skaliert die Farbwerte, um eine angemessene Helligkeit zu gewährleisten
  setColor(map(r, 0, 255, 50, 255), map(g, 0, 255, 50, 255), map(b, 0, 255, 50, 255));
}

void setColor(uint8_t red, uint8_t green, uint8_t blue) {
  // Diese Funktion setzt die Farben der RGB-LED durch Anwendung der PWM-Werte auf die entsprechenden Pins.
  
  analogWrite(RED_PIN, red);   
  analogWrite(GREEN_PIN, green); 
  analogWrite(BLUE_PIN, blue);  
}

void handle_button_press() {
  // Diese Funktion wird durch den Button-Interrupt ausgelöst und sorgt dafür,
  // dass die Statusänderung des Buttons (gedrückt/nicht gedrückt) erkannt wird.
  
  static uint32_t last_press = 0;
  if (millis() - last_press > 200) {
    button_pressed = true;
    last_press = millis();  
  }
}

void waitForCommissioning() {
  // Diese Funktion sorgt dafür, dass das Gerät wartet, bis es erfolgreich
  // mit dem Matter-Netzwerk gekoppelt und verbunden wurde.
  
  while (!Matter.isDeviceCommissioned()) {
    Serial.println("Matter-Gerät ist nicht eingebunden");
    delay(200);
  }

  // Wartet, bis das Gerät mit einem Thread-Netzwerk verbunden ist
  Serial.println("Warte auf Thread-Netzwerk...");
  while (!Matter.isDeviceThreadConnected()) delay(200);
  Serial.println("Mit Thread-Netzwerk verbunden");
}
