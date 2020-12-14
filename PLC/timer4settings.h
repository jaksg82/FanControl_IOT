/* Configure the PWM clock */
void pwm6configure()
{
  // TCCR4B configuration
  TCCR4B = 4; /* 4 sets 23437Hz */

  // TCCR4C configuration
  TCCR4C = 0;

  // TCCR4D configuration
  TCCR4D = 0;  // Fast PWM - Single slope

  // PLL Configuration
  //PLLFRQ = (PLLFRQ & 0xCF) | 0x00;  // Do Not Use PLL output
  //PLLFRQ = (PLLFRQ & 0xCF) | 0x10;  // Use PLL output divided by 1 = 96MHz
  //PLLFRQ = (PLLFRQ & 0xCF) | 0x20;  // Use PLL output divided by 1.5 = 64MHz
  PLLFRQ = (PLLFRQ & 0xCF) | 0x30;  // Use PLL output divided by 2 = 48MHz

  // Terminal count for Timer 4 PWM
  OCR4C = 255;
}

// Set PWM to D6 (Timer4 D)
// Argument is PWM between 0 and 255
void pwmSet6(int value)
{
  OCR4D = value;  // Set PWM value
  DDRD |= 1 << 7; // Set Output Mode D7
  TCCR4C |= 0x09; // Activate channel D
}
