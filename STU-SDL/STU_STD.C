

int clamp(int val, int lo, int hi)
{
    return ( (val < lo) ? lo : ( (val > hi) ? hi : val ) );
}

float clampf(float val, float lo, float hi)
{
    return ((val < lo) ? lo : ((val > hi) ? hi : val));
}
