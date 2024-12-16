#pragma once

int InitSerial();
char ReadSerial();
void ReadSerial(char *s, int c);
void WriteSerial(char a);
void WriteSerial(char *s);
