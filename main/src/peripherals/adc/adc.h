#ifdef __cplusplus
extern "C" {
#endif

void configAdcContinous(void);
void configAdcEnabled(struct enableAdc enable);
void IRAM_ATTR read_adc(void);

#ifdef __cplusplus
}
#endif