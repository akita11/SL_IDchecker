#include <Arduino.h>
#include <M5Unified.h>
#include <PN532_I2C.h>
#include <PN532.h>

#define USE_MIFARE_CASSIC 

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);	

#define CARD_FELICA 1
#define CARD_MIFARE 0

String getCardID(uint8_t cardType = CARD_FELICA) {
	String id = "";
	uint8_t ret;
  uint8_t idm[8];
	if (cardType == CARD_FELICA){
  	uint16_t systemCode = 0xFFFF;
  	uint8_t requestCode = 0x01;       // System Code request
  	uint8_t pmm[8];
  	uint16_t systemCodeResponse;
  	ret = nfc.felica_Polling(systemCode, requestCode, idm, pmm, &systemCodeResponse, 30); // timeout=100 -> about 3sec
  	if (ret == 1){
			for (byte i = 0; i < 8; i++) {
				id += String(idm[i], HEX);
			}
		}
		//printf("card ID: %s (%d)\n", id.c_str(), id.length());
	}
	else{
		uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  	// 'uid' will be populated with the UID, and uidLength will indicate
  	// if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  	ret = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, idm, &uidLength);
    if (ret) {
	    // Display some basic information about the card
  	  printf("Found an ISO14443A card, uid=%x (len=%d)\n", idm, idm, uidLength);
	  	if (ret == 1){
				for (byte i = 0; i < 8; i++) {
					id += String(idm[i], HEX);
				}
			}
			//printf("card ID: %s (%d)\n", id.c_str(), id.length());
	    if (uidLength == 4){
	      // We probably have a Mifare Classic card ... 
     		printf("Seems to be a Mifare Classic card (4 byte UID)\n");
	  	  // Now we need to try to authenticate it for read/write access
      	// Try with the factory default KeyA: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
      	printf("Trying to authenticate block 4 with default KEYA value\n");
      	uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	  		// Start with block 4 (the first block of sector 1) since sector 0
	  		// contains the manufacturer data and it's probably better just
	  		// to leave it alone unless you know what you're doing
      	ret = nfc.mifareclassic_AuthenticateBlock(idm, uidLength, 4, 0, keya);
      	if (ret){
 	      	printf("Sector 1 (Blocks 4..7) has been authenticated\n");
        	uint8_t data[16];
        	// If you want to write something to block 4 to test with, uncomment
					// the following line and this text should be read back in a minute
        	// data = { 'a', 'd', 'a', 'f', 'r', 'u', 'i', 't', '.', 'c', 'o', 'm', 0, 0, 0, 0};
       	 	// success = nfc.mifareclassic_WriteDataBlock (4, data);
        	// Try to read the contents of block 4
        	ret = nfc.mifareclassic_ReadDataBlock(4, data);
        	if (ret){
          	// Data seems to have been read ... spit it out
          	printf("Reading Block 4: %x\n", data);
          	// Wait a bit before reading the card again
          	delay(1000);
        	}
        	else{
          	printf("Ooops ... unable to read the requested block.  Try another key?\n");
        	}
      	}
      	else{
        	printf("Ooops ... authentication failed: Try another key？\n");
      	}
			}
    }    
    if (uidLength == 7){
      // We probably have a Mifare Ultralight card ...
      printf("Seems to be a Mifare Ultralight tag (7 byte UID)\n");
      // Try to read the first general-purpose user page (#4)
      printf("Reading page 4: ");
      uint8_t data[32];
      ret = nfc.mifareultralight_ReadPage (4, data);
      if (ret){
        // Data seems to have been read ... spit it out
        printf("%x\n", data);
        // Wait a bit before reading the card again
        delay(1000);
      }
      else{
        printf("Ooops ... unable to read the requested page!?\n");
      }
    }
	}
	return(id);
}

void setup() {
	M5.begin();
	//M5.Ex_I2C.begin(); // need for ATOMS3's Grove port
  Wire.begin(32, 33); // Core2's PortA
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();

	M5.Display.clear();
	M5.Display.setTextSize(3);
	M5.Display.setTextScroll(true);
	M5.Display.printf("RFID Reader\n");
}

void loop() {
	String cardID;
	cardID = getCardID(CARD_MIFARE);
	if (cardID.length() > 0) {
		M5.Display.printf("Mifare, ID=%s\n", cardID.c_str());
		delay(1000);
	}
	cardID = getCardID(CARD_FELICA);
	if (cardID.length() > 0) {
		M5.Display.printf("Felica, ID=%s\n", cardID.c_str());
		delay(1000);
	}
}
