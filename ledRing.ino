

void ledRingInit()
{
  strip.begin();
  strip.setBrightness(50);
  strip.show(); // Initialize all pixels to 'off'
}
// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*1; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    vTaskDelay(wait);
  }
}
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void setLedLightCount(int n)
{
  strip.clear();
  for(int i=0;i<n;i++)
    strip.setPixelColor(i,255,0,0);
  strip.show();
}
void setLedLight(int n,int sid)
{
  if(n>5)return;
  if(sid>2)return;
  strip.clear();
  for(int i=0;i<=n;i++)
    strip.setPixelColor(i,255,0,0);
  for(int i=7;i>=8-sid;i--)
    strip.setPixelColor(i,0,255,0);
  strip.show();
}
