#pragma once

void resetfpshistory();
void updatefpshistory(int millis);
void getframemillis(float &avg, float &bestdiff, float &worstdiff);
void getfps(int &fps, int &bestdiff, int &worstdiff);
void limitfps(int &millis, int curmillis);
