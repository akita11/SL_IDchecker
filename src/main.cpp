#include <Arduino.h>
#include <M5Unified.h>
#include "MFRC522_I2C.h"

MFRC522 mfrc522(0x28);

String getCardID(){
	String id = "";
	if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
		//printf("no card\n");
	}
	else{
		for (byte i = 0; i < mfrc522.uid.size; i++) {
			id += String(mfrc522.uid.uidByte[i], HEX);
			//printf("%02x ", mfrc522.uid.uidByte[i]);
		}
		//printf("\n");
	}
	//printf("card ID: %s\n", id.c_str());
	return(id);
}

void setup() {
	M5.begin();
	mfrc522.PCD_Init(); // Init MFRC522
}

void loop() {
	String cardID = getCardID();
	if (cardID.length() > 0) {
		printf("Card ID: %s\n", cardID.c_str());
		delay(1000);
	}
}
