#include "com_atelieryl_wonderdroid_WonderSwan.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

#include <stdbool.h>
#include <stdint.h>
#include "wswan.h"

#include "gfx.h"
#include "memory.h"
#include "start.h"
#include "sound.h"
#include "v30mz.h"
#include "rtc.h"
#include "eeprom.h"

#include "log.h"

uint32_t rom_size;
int wsc = 0; /*color/mono*/
uint16_t WSButtonStatus;
uint32_t sram_size;
uint32_t eeprom_size;

JNIEXPORT void JNICALL Java_com_atelieryl_wonderdroid_WonderSwan_reset(JNIEnv * env, jclass obj) {
	LOGE("v30mz_reset()");
	v30mz_reset();
	LOGE("WSwan_MemoryReset()");
	WSwan_MemoryReset();
	LOGE("WSwan_GfxReset()");
	WSwan_GfxReset();
	LOGE("WSwan_SoundReset()");
	wswan_soundreset();
	LOGE("WSwan_InterruptReset()");
	WSwan_InterruptReset();
	LOGE("WSwan_RTCReset()");
	WSwan_RTCReset();
	LOGE("WSwan_EEPROMReset()");
	WSwan_EEPROMReset();

	wsMakeTiles();
	wsSetVideo(wsVMode, TRUE);

	for (int u0 = 0; u0 < 0xc9; u0++)
		WSwan_writeport(u0, startio[u0]);

	v30mz_set_reg(NEC_SS, 0);
	v30mz_set_reg(NEC_SP, 0x2000);
}

JNIEXPORT void JNICALL Java_com_atelieryl_wonderdroid_WonderSwan_load(JNIEnv * env, jclass obj, jstring filename,
		jboolean iswsc, jstring name, jint year, jint month, jint day, jint blood, jint sex) {

	if (iswsc) {
		LOGE("Emulating a WonderSwan Color");
		wsc = 1;
		wsVMode = 0x7;

	}
	else {
		wsVMode = 0x0;
	}

	// char convertedfilename[] = filename;
	char temp[512];
	const jbyte *str;
	str = (*env)->GetStringUTFChars(env, filename, NULL);
	if (str == NULL) {
		//return NULL; /* OutOfMemoryError already thrown */
		//break;
	}

	snprintf(temp, sizeof(temp), "Loading %s", str);
	LOGE(temp);
	FILE* file = fopen(str, "r");
	if (file != NULL) {
		LOGE("The file loaded!!!");
	}

	struct stat st;
	stat(str, &st);
	rom_size = st.st_size;

	//rom_size = uppow2(fp->size);
	// rom_size = 4 * 1024 * 1024;
	//sprintf(tempstring, "size of rom: %d", rom_size);
	// LOGE(tempstring);
	if (wsCartROM != NULL) {
		free(wsCartROM);
	}
	wsCartROM = (uint8_t *) calloc(1, rom_size);

	fread(wsCartROM, sizeof(uint8_t), rom_size, file);

	fclose(file);

	uint8_t header[10];
	memcpy(header, wsCartROM + rom_size - 10, 10);

	sram_size = 0;
	eeprom_size = 0;

	switch (header[5]) {
		case 0x01:
			LOGE("This is a header 0x01 ROM.");
			sram_size = 8 * 1024;
			break;
		case 0x02:
			LOGE("This is a header 0x02 ROM.");
			sram_size = 32 * 1024;
			break;
		case 0x03:
			LOGE("This is a header 0x03 ROM.");
			sram_size = 16 * 65536;
			break;
		case 0x04:
			LOGE("This is a header 0x04 ROM.");
			sram_size = 32 * 65536;
			break; // Dicing Knight!

		case 0x10:
			LOGE("This is a header 0x10 ROM.");
			eeprom_size = 128;
			break;
		case 0x20:
			LOGE("This is a header 0x20 ROM.");
			eeprom_size = 2 * 1024;
			break;
		case 0x50:
			LOGE("This is a header 0x50 ROM.");
			eeprom_size = 1024;
			break;
	}

	// sprintf(tempstring, "SRAM size is 0x%x", sram_size);
	// LOGE(tempstring);

	// sprintf(tempstring, "EEPROM size is 0x%x", eeprom_size);
	//  LOGE(tempstring);

	if (header[6] & 0x1) {
//LOGE("Game orientation is vertical");
	}
	else {
//LOGE("Game orientation is horizontal");
	}

	v30mz_init(WSwan_readmem20, WSwan_writemem20, WSwan_readport, WSwan_writeport);
	// sprintf(tempstring, "WSwan_MemoryInit(%d, %d)", wsc, sram_size);
	// LOGE(tempstring);
	WSwan_MemoryInit(wsc, sram_size); // EEPROM and SRAM are loaded in this func.

	const jbyte *namestr;
	str = (*env)->GetStringUTFChars(env, name, NULL);
	if (str == NULL) {
		//return NULL; /* OutOfMemoryError already thrown */
		//break;
	}
	WSwan_EEPROMInit("", (uint16_t) year, (uint8_t) month, (uint8_t) day, (uint8_t) sex, (uint8_t) blood);

// LOGE("WSwan_GfxInit()");
	WSwan_GfxInit();

// LOGE("WSwan_SoundInit()");
	wswan_soundinit();

// LOGE("wsMakeTiles()");
	wsMakeTiles();

//LOGE("reset()");
//reset();
}

JNIEXPORT jint JNICALL Java_com_atelieryl_wonderdroid_WonderSwan__1execute_1frame(JNIEnv *env, jclass obj,
		jboolean skip, jboolean audio, jobject framebuffer, jshortArray audiobuffer) {

	if (ramLock)
		return 0;

	/*LOGE("Start of frame");
	unsigned reg = v30mz_get_reg(1);
	LOGE("Program counter now at %d", reg);*/

	// execute the active frame cycles
	uint16_t* fb = (uint16_t*) (*env)->GetDirectBufferAddress(env, framebuffer);
	while (wsExecuteLine(fb, skip) < 144) {};

	// grab the audio if we want it
	jint samples = 0;
	if(audio){
		int16_t* ab = (int16_t*) (*env)->GetShortArrayElements(env, audiobuffer, NULL);
		samples = (jint) wswan_soundflush(ab);
		(*env)->ReleaseShortArrayElements(env, audiobuffer, ab, 0);
	}
	else {
		wswan_soundclear();
	}

	// execute the vblank section
	while (wsExecuteLine(NULL, FALSE) != 0) {}

	// return the number of new audio samples
	return samples;
}

JNIEXPORT void JNICALL Java_com_atelieryl_wonderdroid_WonderSwan_updatebuttons(JNIEnv *env, jclass obj,
		jboolean y1, jboolean y2, jboolean y3, jboolean y4, jboolean x1, jboolean x2, jboolean x3, jboolean x4,
		jboolean a, jboolean b, jboolean start) {

	uint16_t newbuttons = 0x0000;

	if (start)
		newbuttons |= 0x100;
	if (a)
		newbuttons |= 0x200;
	if (b)
		newbuttons |= 0x400;
	if (x1)
		newbuttons |= 0x1;
	if (x2)
		newbuttons |= 0x2;
	if (x3)
		newbuttons |= 0x4;
	if (x4)
		newbuttons |= 0x8;
	if (y1)
		newbuttons |= 0x10;
	if (y2)
		newbuttons |= 0x20;
	if (y3)
		newbuttons |= 0x40;
	if (y4)
		newbuttons |= 0x80;

	WSButtonStatus = newbuttons;
}

JNIEXPORT void JNICALL Java_com_atelieryl_wonderdroid_WonderSwan_execute_1vblank(JNIEnv *env, jclass obj) {
	//while(wsExecuteLine(NULL, FALSE) != 0){}
}

JNIEXPORT void JNICALL Java_com_atelieryl_wonderdroid_WonderSwan_loadbackupdata(JNIEnv *env, jclass obj,
		jstring filename) {

	LOGE("loading backup data");

	char temp[256];
	const jbyte *str;
	str = (*env)->GetStringUTFChars(env, filename, NULL);
	if (str == NULL) {
		return; /* OutOfMemoryError already thrown */
	}

	snprintf(temp, sizeof(temp), "eeprom size %d, sram size %d", eeprom_size, sram_size);

	FILE* file = fopen(str, "r");
	if (file != NULL) {
		if (sram_size) {
			LOGE("Loading SRAM");
			fread(wsSRAM, sizeof(uint8_t), sram_size, file);
		}

		else if (eeprom_size) {
			LOGE("Loading eeprom");
			fread(wsEEPROM, sizeof(uint8_t), eeprom_size, file);
		}

		fclose(file);
	}

}

JNIEXPORT void JNICALL Java_com_atelieryl_wonderdroid_WonderSwan_storebackupdata(JNIEnv *env, jclass obj,
		jstring filename) {
	LOGE("storing backup data");

	char temp[256];
	const jbyte *str;
	str = (*env)->GetStringUTFChars(env, filename, NULL);
	if (str == NULL) {
		return; /* OutOfMemoryError already thrown */
		//break;
	}

	snprintf(temp, sizeof(temp), "eeprom size %d, sram size %d", eeprom_size, sram_size);

	FILE* file = fopen(str, "w");
	if (file != NULL) {
		if (sram_size) {
			LOGE("Writing SRAM");
			fwrite(wsSRAM, sizeof(uint8_t), sram_size, file);
		}
		else if (eeprom_size) {
			LOGE("Writing eeprom");
			fwrite(wsEEPROM, sizeof(uint8_t), eeprom_size, file);
		}
		fclose(file);
	}

}

JNIEXPORT void JNICALL Java_com_atelieryl_wonderdroid_WonderSwan_loadramdata(JNIEnv *env, jclass obj,
		jstring filename) {

	LOGE("loading ram data");

	char temp[256];
	const jbyte *str;
	str = (*env)->GetStringUTFChars(env, filename, NULL);
	if (str == NULL) {
		return; /* OutOfMemoryError already thrown */
	}

	snprintf(temp, sizeof(temp), "ram size always 65536");

	FILE* file = fopen(str, "r");
	if (file != NULL) {
		LOGE("Loading RAM");
		ramLock = true;

		//v30mz_reset();
		//WSwan_GfxReset();
		//wsMakeTiles();

		uint8_t toLoad[65536 + sizeof(unsigned) * 14 + sram_size + 1 + 1312 + 28 + 547 + 8224 + 132104 + 1048576];
		fread(toLoad, sizeof(uint8_t), 65536 + sizeof(unsigned) * 14 + sram_size + 1 + 1312 + 28 + 547 + 8224 + 132104 + 1048576, file);
		memcpy(wsRAM, toLoad, 65536);
		LOGE("Restoring registers");
		for (int i = 0; i < 14; i++) {
			unsigned *reg = malloc(sizeof(unsigned));
			memcpy(reg, toLoad + 65536 + sizeof(unsigned) * i, sizeof(unsigned));
			LOGE("LOAD Register %d value %d", i, *reg);
			v30mz_set_reg(i + 1, *reg);
			//free(reg);
		}
		LOGE("Loading SRAM with size %d", sram_size);
		memcpy(wsSRAM, toLoad + 65536 + sizeof(unsigned) * 14, sram_size);
		uint8_t *scanline = malloc(sizeof(uint8_t));
		memcpy(scanline, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size, sizeof(uint8_t));
		LOGE("Loading scanline number: %d", *scanline);
		wsLine = *scanline;
		LOGE("Loading graphics info");
		memcpy(wsMonoPal, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1, 256);
		memcpy(wsColors, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1 + 256, 32);
		memcpy(wsCols, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1 + 256 + 32, 1024);
		uint32_t *color = malloc(sizeof(uint32_t));
		LOGE("Load the color");
		memcpy(color, &wsColors[3], 4);
		LOGE("Load the color %d", *color);
		LOGE("Loading memory info");
		memcpy(&ButtonWhich, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1313, 1);
		memcpy(&ButtonReadLatch, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1314, 1);
		memcpy(&DMASource, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1315, 4);
		memcpy(&DMADest, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1319, 4);
		memcpy(&DMALength, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1323, 2);
		memcpy(&DMAControl, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1325, 1);
		LOGE("Load DMA Control %d", DMAControl);
		memcpy(&SoundDMASource, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1326, 4);
		memcpy(&SoundDMALength, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1330, 2);
		memcpy(&SoundDMAControl, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1332, 1);
		memcpy(BankSelector, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1333, 4);
		memcpy(&CommControl, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1337, 1);
		memcpy(&CommData, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1338, 1);
		memcpy(&WSButtonStatus, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1339, 2);
		LOGE("Loading more graphics info");
		memcpy(SpriteTable, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1341, 512);
		memcpy(&SpriteCountCache, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1853, 4);
		memcpy(&DispControl, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1857, 1);
		memcpy(&BGColor, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1858, 1);
		memcpy(&LineCompare, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1859, 1);
		memcpy(&SPRBase, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1860, 1);
		memcpy(&SpriteStart, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1861, 1);
		memcpy(&SpriteCount, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1862, 1);
		memcpy(&FGBGLoc, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1863, 1);
		memcpy(&FGx0, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1864, 1);
		memcpy(&FGy0, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1865, 1);
		memcpy(&FGx1, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1866, 1);
		memcpy(&FGy1, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1867, 1);
		memcpy(&SPRx0, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1868, 1);
		memcpy(&SPRy0, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1869, 1);
		memcpy(&SPRx1, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1870, 1);
		memcpy(&SPRy1, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1871, 1);
		memcpy(&BGXScroll, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1872, 1);
		memcpy(&BGYScroll, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1873, 1);
		memcpy(&FGXScroll, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1874, 1);
		memcpy(&FGYScroll, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1875, 1);
		memcpy(&LCDControl, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1876, 1);
		memcpy(&LCDIcons, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1877, 1);
		memcpy(&BTimerControl, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1878, 1);
		memcpy(&HBTimerPeriod, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1879, 2);
		memcpy(&VBTimerPeriod, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1881, 2);
		memcpy(&HBCounter, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1883, 2);
		memcpy(&VBCounter, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1885, 2);
		memcpy(&VideoMode, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1887, 1);
		LOGE("Loading color map");
		memcpy(ColorMapG, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1888, 32);
		memcpy(ColorMap, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 1920, 8192);
		LOGE("Loading tile cache");
		memcpy(wsTCache, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 10112, 32768);
		LOGE("X) Load the tile cache %d", wsTCache[12444]);
		uint8_t *testvar5 = malloc(sizeof(uint8_t));
		memcpy(testvar5, wsTCache + 12444, 1);
		LOGE("Y) Load the tile cache %d", *testvar5);
		uint8_t *testvar6 = malloc(sizeof(uint8_t));
		memcpy(testvar6, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 10112 + 12444, 1);
		LOGE("Z) Load the tile cache %d", *testvar6);
		memcpy(wsTCacheFlipped, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 42880, 32768);
		memcpy(wsTileRow, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 75648, 8);
		memcpy(wsTCacheUpdate, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 75656, 512);
		memcpy(wsTCache2, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 76168, 32768);
		memcpy(wsTCacheFlipped2, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 108936, 32768);
		memcpy(wsTCacheUpdate2, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 141704, 512);
		LOGE("Loading tile");
		memcpy(&tiles[0][0][0][0], toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 142216, 1048576);
		LOGE("A) Load the 23rd tile %d", tiles[0][1][0][7]);
		uint8_t *testvar2 = malloc(sizeof(uint8_t));
		memcpy(testvar2, tiles + 23, 1);
		LOGE("D) Load the 23rd tile %d", *testvar2);
		uint8_t *testvar3 = malloc(sizeof(uint8_t));
		memcpy(testvar3, toLoad + 65536 + sizeof(unsigned) * 14 + sram_size + 142216 + 23, 1);
		LOGE("C) Load the 23rd tile %d", *testvar3);
		LOGE("B) Sanity check: %p %p", (void*)(tiles + 23), (void*)&tiles[23][0][0][0]);
		wsSetVideo(wsVMode, TRUE);
		ramLock = false;
		fclose(file);
		LOGE("Load complete");
	}

}

JNIEXPORT void JNICALL Java_com_atelieryl_wonderdroid_WonderSwan_storeramdata(JNIEnv *env, jclass obj,
		jstring filename) {

	LOGE("storing ram data");

	char temp[256];
	const jbyte *str;
	str = (*env)->GetStringUTFChars(env, filename, NULL);
	if (str == NULL) {
		return; /* OutOfMemoryError already thrown */
	}

	snprintf(temp, sizeof(temp), "ram size always 65536");

	FILE* file = fopen(str, "w");
	if (file != NULL) {
		LOGE("Writing RAM");
		ramLock = true;
		uint8_t toWrite[65536 + sizeof(unsigned) * 14 + sram_size + 1 + 1312 + 28 + 547 + 8224 + 132104 + 1048576];
		memcpy(toWrite, wsRAM, 65536);
		for (int i = 0; i < 14; i++) {
			unsigned reg = v30mz_get_reg(i + 1);
			LOGE("SAVE Register %d value %d", i, reg);
			memcpy(toWrite + 65536 + sizeof(unsigned) * i, &reg, sizeof(unsigned));
		}
		LOGE("Saving SRAM with size %d", sram_size);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14, wsSRAM, sram_size);
		uint8_t *scanline = malloc(sizeof(uint8_t));
		memcpy(scanline, &wsLine, 1);
		LOGE("Saving scanline number: %d", *scanline);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size, scanline, 1);
		LOGE("Saving graphics info");
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1, wsMonoPal, 256);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1 + 256, wsColors, 32);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1 + 256 + 32, wsCols, 1024);
		uint32_t *color = malloc(sizeof(uint32_t));
		memcpy(color, toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1 + 256 + 12, 4);
		LOGE("Save the color %d", *color);
		LOGE("Saving memory info");
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1313, &ButtonWhich, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1314, &ButtonReadLatch, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1315, &DMASource, 4);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1319, &DMADest, 4);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1323, &DMALength, 2);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1325, &DMAControl, 1);
		uint8_t *testvar = malloc(sizeof(uint8_t));
		memcpy(testvar, &DMAControl, 1);
		LOGE("Save DMA Control %d", *testvar);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1326, &SoundDMASource, 4);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1330, &SoundDMALength, 2);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1332, &SoundDMAControl, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1333, BankSelector, 4);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1337, &CommControl, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1338, &CommData, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1339, &WSButtonStatus, 2);
		LOGE("Saving more graphics info");
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1341, SpriteTable, 512);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1853, &SpriteCountCache, 4);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1857, &DispControl, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1858, &BGColor, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1859, &LineCompare, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1860, &SPRBase, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1861, &SpriteStart, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1862, &SpriteCount, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1863, &FGBGLoc, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1864, &FGx0, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1865, &FGy0, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1866, &FGx1, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1867, &FGy1, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1868, &SPRx0, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1869, &SPRy0, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1870, &SPRx1, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1871, &SPRy1, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1872, &BGXScroll, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1873, &BGYScroll, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1874, &FGXScroll, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1875, &FGYScroll, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1876, &LCDControl, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1877, &LCDIcons, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1878, &BTimerControl, 1);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1879, &HBTimerPeriod, 2);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1881, &VBTimerPeriod, 2);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1883, &HBCounter, 2);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1885, &VBCounter, 2);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1887, &VideoMode, 1);
		LOGE("Saving color map");
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1888, ColorMapG, 32);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 1920, ColorMap, 8192);
		LOGE("Saving tile cache");
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 10112, wsTCache, 32768);
		uint8_t *testvar3 = malloc(sizeof(uint8_t));
		memcpy(testvar3, toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 10112 + 12444, 1);
		LOGE("Save the tile cache %d", *testvar3);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 42880, wsTCacheFlipped, 32768);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 75648, wsTileRow, 8);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 75656, wsTCacheUpdate, 512);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 76168, wsTCache2, 32768);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 108936, wsTCacheFlipped2, 32768);
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 141704, wsTCacheUpdate2, 512);
		LOGE("Saving tile");
		memcpy(toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 142216, tiles, 1048576);
		uint8_t *testvar2 = malloc(sizeof(uint8_t));
		memcpy(testvar2, toWrite + 65536 + sizeof(unsigned) * 14 + sram_size + 142216 + 23, 1);
		LOGE("Save the tile %d", *testvar2);
		fwrite(toWrite, sizeof(uint8_t), 65536 + sizeof(unsigned) * 14 + sram_size + 1 + 1312 + 28 + 547 + 8224 + 132104 + 1048576, file);
		LOGE("W) Sanity check: %p %p %p %p %p", (void*)&tiles[23][0][0][0], (void*)&tiles[0][0][0][0], (void*)&tiles[0][1][0][7], (void*)(tiles + 23), (void*)(tiles) + 23);
		ramLock = false;
		fclose(file);
	}

}
