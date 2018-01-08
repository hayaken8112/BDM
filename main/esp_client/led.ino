void setLED_roll(float *data, uint32_t color, bool is_me)
{
    uint8_t index = (uint8_t)((data[1] + 90) / 12);
    setLED(index, 3, color, is_me);
}

// n:index, m:number of LEDs (must be odd)
void setLED(uint8_t n, uint8_t m, uint32_t color, bool is_me)
{
    uint8_t start_i = is_me ? 0 : 15;
    uint8_t end_i = is_me ? 15 : 30;
    n = n + start_i;
    m = (m + 1) / 2;
    for (uint8_t i = start_i; i < end_i; i++)
    {
        if (n - m < i && i < n + m)
        {
            strip.setPixelColor(i, color);
        }
        else
        {
            strip.setPixelColor(i, strip.Color(0, 0, 0));
        }
    }
    strip.show();
}