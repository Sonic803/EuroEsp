
void configUpdate(void);

struct enableAdc
{
    bool pots[2];
    bool jacks[3];
};

struct enableOut
{
    bool vco;
    bool lfo;
    bool pwm[2];
    bool digi[2];
};
