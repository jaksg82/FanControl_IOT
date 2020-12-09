#ifndef DEBUG_H
#define DEBUG_H


class Debug {
public:
	template <class T>
	static void print(T value) {
		if (Serial) {
			Serial.print(value);
		}
	}

	template <class T>
	static void println(T value) {
		if (Serial) {
			Serial.println(value);
		}
	}


};

#endif
