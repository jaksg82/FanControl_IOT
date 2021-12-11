#include "ByteRange.h"

// Constructors
// Default range from 0 to 255
// 0                                                               255
// |<===============================================================>|
ByteRange::ByteRange() {
  _minVal = 0;
  _maxVal = 255;
  _floorVal = 0;
  _ceilingVal = 255;
}

// Define floor and ceiling values. Min and max get the floor and ceiling values.
// val0                                                           val1
// |<===============================================================>|
ByteRange::ByteRange(byte val0, byte val1) {
  byte v0, v1;
  v0 = min(val0, val1);
  v1 = max(val0, val1);

  if(v0 < v1) {
    _floorVal = v0;
    _ceilingVal = v1;
  } else {  // v0 == v1
    if(v1 < 255) {
      _floorVal = v1;
      _ceilingVal = v1 + 1;
    } else {
      _floorVal = v1 - 1;
      _ceilingVal = v1;
    }
  }
  _minVal = _floorVal;
  _maxVal = _ceilingVal;
}

// Define the 4 values in sequence. Floor, Min, Max and Ceiling.
// val0          val1                         val2                val3
// |-------------<===============================>-------------------|
ByteRange::ByteRange(byte val0, byte val1, byte val2, byte val3) {
  byte v0, v1, v2, v3;
  byte vals[4] = {val0, val1, val2, val3 };
  std::sort(vals, vals+4);
  // Get the real order of the values
  v0 = vals[0];
  v1 = vals[1];
  v2 = vals[2];
  v3 = vals[3];

  if(v0 < v3) {
    _floorVal = v0;
    _ceilingVal = v3;
    if(v1 < v2) {
      _minVal = v1;
      _maxVal = v2;
    } else { // MIN and MAX values are the same
      if(v1 > _floorVal) {  // Change the MIN value first if possible
        _minVal = v1 - 1;
        _maxVal = v2;
      } else {
        _minVal = v1;
        _maxVal = v2 + 1;
      }
    }
  } else { // Edge case: 4 identical values
    if(v3 < 255) {
      _floorVal = v3;
      _ceilingVal = v3 + 1;
    } else {
      _floorVal = v3 - 1;
      _ceilingVal = v3;
    }
    _minVal = _floorVal;
    _maxVal = _ceilingVal;
  }
}

// Change the FLOOR value
// ===>> This will change also the other values if necessary
// Return FALSE when the passed value is 255 because there is no value range to define the CEILING
bool ByteRange::SetFloor(byte value) {
  if(value < _ceilingVal) {
    _floorVal = value;
    if(_minVal < _floorVal) { _minVal = _floorVal; }
    if(_maxVal < _floorVal) { _maxVal = _floorVal + 1; }
    return true;
  } else {
    if(value < 255) {
      _floorVal = value;
      _ceilingVal = _floorVal + 1;
      _minVal = _floorVal;
      _maxVal = _ceilingVal;
      return true;
    } else {
      return false;
    }
  }
}

// Change the CEILING value
// ===>> This will change also the other values if necessary
// Return FALSE when the passed value is 0 because there is no value range to define the FLOOR
bool ByteRange::SetCeiling(byte value) {
  if(value > _floorVal) {
    _ceilingVal = value;
    if(_minVal > _ceilingVal) { _minVal = _ceilingVal - 1; }
    if(_maxVal > _ceilingVal) { _maxVal = _ceilingVal; }
    return true;
  } else {
    if(value > 0) {
      _ceilingVal = value;
      _floorVal = _ceilingVal - 1;
      _minVal = _floorVal;
      _maxVal = _ceilingVal;
      return true;
    } else {
      return false;
    }
  }
}


// Change the MIN value
// ===>> This will change also the MAX value if necessary
// Return FALSE when the passed value is outside the range defined by FLOOR and CEILING values
bool ByteRange::SetMin(byte value) {
  if(value >= _floorVal && value < _ceilingVal) {
    _minVal = value;
    if(value >= _maxVal) { _maxVal = value + 1; }
    return true;
  }
  return false;
}


// Change the MAX value
// ===>> This will change also the MIN value if necessary
// Return FALSE when the passed value is outside the range defined by FLOOR and CEILING values
bool ByteRange::SetMax(byte value) {
  if(value > _floorVal && value <= _ceilingVal) {
    _maxVal = value;
    if(value <= _minVal) { _minVal = value - 1; }
    return true;
  }
  return false;
}


// Change the MIN and MAX value
// Return FALSE when the passed value is outside the range defined by FLOOR and CEILING values
bool ByteRange::SetMinMax(byte value0, byte value1) {
  byte v0 = min(value0, value1);
  byte v1 = max(value0, value1);
  bool r0 = this->SetMin(v0);
  bool r1 = this->SetMax(v1);
  return r0 || r1;
}


// Increase by 1 the MIN value
// ===>> This will change also the MAX value if necessary
// Return FALSE when the passed value is outside the range defined by FLOOR and CEILING values
bool ByteRange::IncreaseMin() {
  return this->SetMin(this->_minVal + 1);
}


// Increase by 1 the MAX value
// Return FALSE when the passed value is outside the range defined by FLOOR and CEILING values
bool ByteRange::IncreaseMax() {
  return this->SetMax(this->_maxVal + 1);
}


// Decrease by 1 the MIN value
// Return FALSE when the passed value is outside the range defined by FLOOR and CEILING values
bool ByteRange::DecreaseMin() {
  return this->SetMin(this->_minVal - 1);
}


// Decrease by 1 the MAX value
// ===>> This will change also the MIN value if necessary
// Return FALSE when the passed value is outside the range defined by FLOOR and CEILING values
bool ByteRange::DecreaseMax() {
  return this->SetMax(this->_maxVal - 1);
}
