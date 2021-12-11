#ifndef BYTERANGE_H
#define BYTERANGE_H

#include "Arduino.h"
#include <stddef.h>
#include <stdint.h>
#include <algorithm>

class ByteRange {
  private:
    byte _minVal, _maxVal, _floorVal, _ceilingVal;

  public:
    // Constructors
    // Default range from 0 to 255
    // 0                                                               255
    // |<===============================================================>|
    ByteRange();
    // Define floor and ceiling values. Min and max get the floor and ceiling values.
    // val0                                                           val1
    // |<===============================================================>|
    ByteRange(byte val0, byte val1);

    // Define the 4 values in sequence. Floor, Min, Max and Ceiling.
    // val0          val1                         val2                val3
    // |-------------<===============================>-------------------|
    ByteRange(byte val0, byte val1, byte val2, byte val3);

    // Get or Set Range Extremes
    bool SetFloor(byte value);
    bool SetCeiling(byte value);
    byte GetFloor() { return _floorVal; }
    byte GetCeiling() { return _ceilingVal; }
    byte GetMaxSize() { return (_ceilingVal - _floorVal) + 1; }

    // Get or Set Range Values
    bool SetMin(byte value);
    bool SetMax(byte value);
    bool SetMinMax(byte value0, byte value1);
    byte GetMin() { return _minVal; }
    byte GetMax() { return _maxVal; }
    bool IncreaseMin();
    bool IncreaseMax();
    bool DecreaseMin();
    bool DecreaseMax();
    byte GetSize() { return (_maxVal - _minVal) + 1; }

};
#endif
