/// Change Date : 23/4/26
/// DONE
//Branch 1 chnages
#include <Wire.h>
#include <Adafruit_BME280.h>

// ================= I2C ADDRESSES =================
#define TCA_ADDR 0x71
#define BME_ADDR 0x76
#define MMR_ADDR 0x67

// ================= CHANNEL MAP ===================
#define CH_MMR920_1   0
#define CH_MMR920_2   1
#define CH_MMR920_3   2

#define CH_MMR940_1   3
#define CH_MMR940_2   4
#define CH_MMR940_3   5

#define CH_BME_1      6
#define CH_BME_2      7

// conversion constant
#define RAW_TO_KPA 9.80665e-7

Adafruit_BME280 bme1;
Adafruit_BME280 bme2;

// ================= TCA SELECT ====================
void tcaSelect(uint8_t ch)
{
  if (ch > 7) return;

  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << ch);
  Wire.endTransmission();
  delay(5);
}

// ================= READ 24 BIT ===================
int32_t readMMR24(uint8_t channel, uint8_t command)
{
  tcaSelect(channel);

  Wire.beginTransmission(MMR_ADDR);
  Wire.write(command);

  if (Wire.endTransmission(false) != 0)
    return 0;

  Wire.requestFrom(MMR_ADDR, 3);

  if (Wire.available() != 3)
    return 0;

  int32_t raw = 0;

  raw |= (int32_t)Wire.read() << 16;
  raw |= (int32_t)Wire.read() << 8;
  raw |= (int32_t)Wire.read();

  if (raw & 0x800000)
    raw |= 0xFF000000;

  return raw;
}

// ================= MMR INIT ======================
void mmrReset(uint8_t ch)
{
  tcaSelect(ch);
  Wire.beginTransmission(MMR_ADDR);
  Wire.write(0x72);
  Wire.endTransmission();
  delay(10);
}

void mmrMode4(uint8_t ch)
{
  tcaSelect(ch);
  Wire.beginTransmission(MMR_ADDR);
  Wire.write(0xA6);
  Wire.endTransmission();
  delay(10);
}

// ================= SETUP =========================
void setup()
{
  Serial.begin(9600);
  Wire.begin();

  // ---- MMR INIT ----
  mmrReset(CH_MMR920_1); mmrMode4(CH_MMR920_1);
  mmrReset(CH_MMR920_2); mmrMode4(CH_MMR920_2);
  mmrReset(CH_MMR920_3); mmrMode4(CH_MMR920_3);

  mmrReset(CH_MMR940_1); mmrMode4(CH_MMR940_1);
  mmrReset(CH_MMR940_2); mmrMode4(CH_MMR940_2);
  mmrReset(CH_MMR940_3); mmrMode4(CH_MMR940_3);

  // ---- BME INIT ----
  tcaSelect(CH_BME_1);
  if (!bme1.begin(BME_ADDR))
    Serial.println("BME CH6 not found");

  tcaSelect(CH_BME_2);
  if (!bme2.begin(BME_ADDR))
    Serial.println("BME CH7 not found");

  Serial.println("System Initialized");
}

// ================= LOOP ==========================
void loop()
{
  // ===== READ MMR =====
  int32_t rp1 = readMMR24(CH_MMR920_1, 0xC0);
  int32_t rp2 = readMMR24(CH_MMR920_2, 0xC0);
  int32_t rp3 = readMMR24(CH_MMR920_3, 0xC0);

  int32_t rp4 = readMMR24(CH_MMR940_1, 0xC0);
  int32_t rp5 = readMMR24(CH_MMR940_2, 0xC0);
  int32_t rp6 = readMMR24(CH_MMR940_3, 0xC0);

  int32_t rt1 = readMMR24(CH_MMR920_1, 0xC2);
  int32_t rt2 = readMMR24(CH_MMR920_2, 0xC2);
  int32_t rt3 = readMMR24(CH_MMR920_3, 0xC2);

  int32_t rt4 = readMMR24(CH_MMR940_1, 0xC2);
  int32_t rt5 = readMMR24(CH_MMR940_2, 0xC2);
  int32_t rt6 = readMMR24(CH_MMR940_3, 0xC2);

  float p1 = rp1 * RAW_TO_KPA;
  float p2 = rp2 * RAW_TO_KPA;
  float p3 = rp3 * RAW_TO_KPA;

  float p4 = rp4 * RAW_TO_KPA;
  float p5 = rp5 * RAW_TO_KPA;
  float p6 = rp6 * RAW_TO_KPA;

  float t1 = rt1 / 128.0;
  float t2 = rt2 / 128.0;
  float t3 = rt3 / 128.0;

  float t4 = rt4 / 128.0;
  float t5 = rt5 / 128.0;
  float t6 = rt6 / 128.0;

  // ===== READ BME =====
  tcaSelect(CH_BME_1);
  float bmeTemp1 = bme1.readTemperature();
  float bmePres1 = bme1.readPressure() / 1000.0;
  float bmeHum1 = bme1.readHumidity();

  tcaSelect(CH_BME_2);
  float bmeTemp2 = bme2.readTemperature();
  float bmePres2 = bme2.readPressure() / 1000.0;
  float bmeHum2  = bme2.readHumidity();

  // ===== OUTPUT =====
  Serial.println();
  Serial.println("FINAL SENSOR OUTPUT TABLE");
  Serial.println("------------------------------------------------------------");

  Serial.print("MMR920 #1   | 0 | "); Serial.print(p1,3 );Serial.print(" | "); Serial.println(t1,0);
  Serial.print("MMR920 #2   | 1 | "); Serial.print(p2,3); Serial.print(" | "); Serial.println(t2,0);
  Serial.print("MMR920 #3   | 2 | "); Serial.print(p3,3); Serial.print(" | "); Serial.println(t3,0);
  Serial.print("MMR940 #1   | 3 | "); Serial.print(p4,3); Serial.print(" | "); Serial.println(t4,0);
  Serial.print("MMR940 #2   | 4 | "); Serial.print(p5,3); Serial.print(" | "); Serial.println(t5,0);
  Serial.print("MMR940 #3   | 5 | "); Serial.print(p6,3); Serial.print(" | "); Serial.println(t6,0);
  Serial.print("BME280      | 6 | "); Serial.print(bmePres1,4); Serial.print(" | "); Serial.println(bmeTemp1,0);Serial.print(bmeHum1, 1); Serial.println(" %");

 // Serial.print("BME280*     | 7 | "); Serial.print(bmePres2,4); Serial.print(" | "); Serial.println(bmeTemp2,0);

  Serial.println("------------------------------------------------------------");

  delay(200);
}
