#ifndef UTITLFUNC_BMS_H_
#define UTITLFUNC_BMS_H_

uint16_t calc_crc(uint8_t *buf, uint8_t len);
void fill_display(void);

void load_default();
void Setting_Save();
void Setting_Load();
bool upload_settings();
void print_data(uint16_t* data, int len, int br);

double interpolate( double x,double x0, double x1, double y0, double y1);
void modulate_oil_temp();
uint16_t get_o2();
#endif