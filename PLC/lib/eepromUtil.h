#ifndef EEPROMUTIL_H
#define EEPROMUTIL_H

#include <EEPROM.h>

static class EepromUtil {
private:
    static const uint8_t rangeMin = 10;
    static const uint8_t rangeMax = 60;
    static const int indexMin = 22;
    static const int indexMax = 33;

public:

    static void GetConfigFromEeprom(uint8_t* tmin, uint8_t* tmax) {
        uint8_t vmin = EEPROM.read(indexMin);
        uint8_t vmax = EEPROM.read(indexMax);
        *tmin = vmin; //> 0 ? vmin : 25; //Set default value if the eeprom value is zero
        *tmax = vmax; //> 0 ? vmax : 35; //Set default value if the eeprom value is zero
    }

    static void SetConfigToEeprom(uint8_t* tmin, uint8_t* tmax) {
        uint8_t vmin = EEPROM.read(indexMin);
        uint8_t vmax = EEPROM.read(indexMax);
        if (vmin != *tmin) { EEPROM.write(indexMin, *tmin); }
        if (vmax != *tmax) { EEPROM.write(indexMax, *tmax); }

    }

    static uint8_t FitInTemp(uint8_t value) {
        if (value < rangeMin) {
            return rangeMin;
        }
        else {
            if (value > rangeMax) {
                return rangeMax;
            }
            else {
                return value;
            }
        }
    }

    static bool isTargetValid(uint8_t tmin, uint8_t tmax) {
        if (tmin >= rangeMin && tmin < rangeMax) {
            if (tmax > rangeMin && tmax <= rangeMax) {
                if (tmin < tmax) {
                    return true;
                }
            }
        }
        return false;
    }

};

#endif
