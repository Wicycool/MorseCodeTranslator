#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/**********************************************************/
LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
/*********************************************************/

int potPin = A0;
int stickPin = A7;
int leftButtonPin = 4;
int enterButtonPin = 3;
int rightButtonPin = 2;
int buzzerPin = 7;

int potBegin = 4;
int potEnd = 1022;
char letters[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_";
String lettersString = String(letters);
char morse[37][6] = { // each thing is 6 long because of the null byte
  "-----",  // 0
  ".----",  // 1
  "..---",  // 2
  "...--",  // 3
  "....-",  // 4
  ".....",  // 5
  "-....",  // 6
  "--...",  // 7
  "---..",  // 8
  "----.",  // 9
  ".-   ",  // A
  "-... ",  // B
  "-.-. ",  // C
  "-..  ",  // D
  ".    ",  // E
  "..-. ",  // F
  "--.  ",  // G
  ".... ",  // H
  "..   ",  // I
  ".--- ",  // J
  "-.-  ",  // K
  ".-.. ",  // L
  "--   ",  // M
  "-.   ",  // N
  "---  ",  // O
  ".--. ",  // P
  "--.- ",  // Q
  ".-.  ",  // R
  "...  ",  // S
  "-    ",  // T
  "..-  ",  // U
  "...- ",  // V
  ".--  ",  // W
  "-..- ",  // X
  "-.-- ",  // Y
  "--.. ",  // Z
  "/    "   // Space or underscore
};

int charAmount = 37;
char letterDisp[8] = "???????";

char inputText[49] = "________________________________________________";
int inputTextSize = 48;
int currentInputChar = 0;
char inputPreview[9] = "????????";

double potRotation;
int selectedLetter;
int prevSelectedLetter = -1;

int stickAmt = 0;

bool leftClick = 0;
bool enterClick = 0;
bool rightClick = 0;

bool stickLeft = 0;
bool stickRight = 0;

bool prevLeftClick = 0;
bool prevEnterClick = 0;
bool prevRightClick = 0;

bool prevStickLeft = 0;
bool prevStickRight = 0;

int timer = 0;

void playSound(int ms) {
    digitalWrite(buzzerPin, LOW);
    delay(ms);
    digitalWrite(buzzerPin, HIGH);
}

void playSoundMicroseconds(int us) {
    digitalWrite(buzzerPin, LOW);
    delayMicroseconds(us);
    digitalWrite(buzzerPin, HIGH);
}

void setup() {
  lcd.init();       //initialize the lcd
  lcd.backlight();  //open the backlight
  Serial.begin(9600);
  pinMode(leftButtonPin, INPUT_PULLUP);
  pinMode(enterButtonPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

  lcd.setCursor(3,0);
  lcd.print("Morse Code");
  lcd.setCursor(3,1);
  lcd.print("Translator");

  for (double i = 0; i < 200; i++) {
    double us = 2500.0 * sin((i) * 1.0 * PI / 200.0);
    Serial.println(us);
    playSoundMicroseconds((int) us);
    delayMicroseconds((int) us);
  }

  delay(1000);
  lcd.clear();
}

/*********************************************************/
void loop() {
  stickAmt = analogRead(stickPin);

  stickLeft = (stickAmt < 350);
  stickRight = (stickAmt > 650);

  leftClick = !digitalRead(leftButtonPin);
  enterClick = !digitalRead(enterButtonPin);
  rightClick = !digitalRead(rightButtonPin);
  
  potRotation = 1 - analogRead(potPin) / (1.0 * (potEnd - potBegin));

  selectedLetter = constrain(floor(potRotation * charAmount), 0, charAmount - 1);

  timer++;
  if (timer > 10000) {
    timer -= 10000;
  }

  // Stop buzzer in case it's somehow playing
  digitalWrite(buzzerPin, HIGH);

  // Draw character selector
  for (int i = 0; i < 7; i++) {
    int letterOn = i + selectedLetter - 3;
    if (letterOn >= 0 && letterOn < charAmount) {
      letterDisp[i] = letters[letterOn];
    } else {
      letterDisp[i] = ' ';
    }
  }

  // Draw character selector
  lcd.setCursor(0, 0);
  lcd.print(letterDisp);

  // Draw helpful arrow for character selector  
  lcd.setCursor(3, 1);
  lcd.print("^");

  
  lcd.setCursor(8, 1);
  if (enterClick && !prevEnterClick) {
    inputText[currentInputChar] = letters[selectedLetter];
    currentInputChar++;
    playSound(7);
  }


  if (stickLeft && !prevStickLeft) {
    currentInputChar--;
    playSound(7);
  }
  if (stickRight && !prevStickRight) {
    currentInputChar++;
    playSound(7);
  }
  if (currentInputChar < 0) {
    currentInputChar = 0;
  }
  if (currentInputChar > inputTextSize - 1) {
    currentInputChar = inputTextSize - 1;
  }


  int letterOn;
  for (int i = 0; i < 8; i++) {
    letterOn = i + constrain(currentInputChar, 4, inputTextSize - 4) - 4;

    if (letterOn >= 0 && letterOn < inputTextSize) {
      if (letterOn == currentInputChar && timer % 14 < 7) {
        inputPreview[i] = ' ';
      } else if (i == 0 && currentInputChar > 4) {
        inputPreview[i] = '<';
      } else if (i == 7 && currentInputChar < inputTextSize - 4) {
        inputPreview[i] = '>';
      } else {
        inputPreview[i] = inputText[letterOn];
      }
    } else {
      inputPreview[i] = '-';
    }
  }

  lcd.setCursor(8, 1);
  lcd.print(inputPreview);

  if (prevSelectedLetter != selectedLetter) {
    playSound(3);
  }

  // Play morse code button pressed
  if (rightClick && !prevRightClick) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Playing...");

    int lastNonSpaceCharacter = inputTextSize - 1;
    while (inputText[lastNonSpaceCharacter] == '_' && lastNonSpaceCharacter > 0) {
      lastNonSpaceCharacter--;
    }

    for (int i = 0; i < lastNonSpaceCharacter + 1; i++) {
      if (i < 10) {
        lcd.setCursor(15, 0);
      } else {
        lcd.setCursor(14, 0);
      }
      lcd.print(i);

      char currLetter = inputText[i];
      int currLetterIdx = lettersString.indexOf(currLetter);

      lcd.setCursor(0, 1);
      lcd.print(currLetter);

      char morseRepOfLetter[6];
      for (int j = 0; j < 6; j++) {
          morseRepOfLetter[j] = morse[currLetterIdx][j];
      }

      lcd.setCursor(3,1);
      lcd.print("      ");

      lcd.setCursor(3,1);
      for (int m = 0; m < 5; m++) {
        if (morseRepOfLetter[m] == '.') {
          Serial.print('.');
          lcd.print(".");
          playSound(100);
        } else if (morseRepOfLetter[m] == '-') {
          Serial.print('-');
          lcd.print("-");
          playSound(200);
        } else if (morseRepOfLetter[m] == '/') {
          Serial.print('/');
          lcd.print("/");
          delay(200);
        }
        delay(100);
      }
      Serial.println();
    }

    lcd.clear();
  }

  // Reset button pressed
  if (leftClick && !prevLeftClick) {
    for (int i = 0; i < inputTextSize; i++) {
      inputText[i] = '_';
    }
    currentInputChar = 0;
    for (int i = 0; i < 3; i++) {
      playSound(3);
      delay(25);
    }
  }

  prevLeftClick = leftClick;
  prevEnterClick = enterClick;
  prevRightClick = rightClick;

  prevStickLeft = stickLeft;
  prevStickRight = stickRight;
  
  prevSelectedLetter = selectedLetter;

  // lcd.setCursor(0, 1);
  // lcd.print(morse[selectedLetter]);
  
  delay(25);
}
/************************************************************/