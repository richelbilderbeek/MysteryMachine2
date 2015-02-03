//The pins that measure which position rotary switch 1 and 2 have
const int pins_input_1[] = {A0,A1,A2,A3,A4};
const int pins_input_2[] = {8,9,10,11,12};
const int n_inputs_1{5};
const int n_inputs_2{5};

//The pins to power the to leds
const int pins_led_top[] = {2,3,4};
const int n_leds_top{3};

void setup()
{
  for (int i{0}; i!=n_inputs_1; ++i) 
  { 
    pinMode(pins_input_1[i],INPUT);
  }
  for (int i{0}; i!=n_inputs_2; ++i) 
  { 
    pinMode(pins_input_2[i],INPUT);
  }
  for (int i{0}; i!=n_leds_top; ++i) 
  { 
    pinMode(pins_led_top[i],OUTPUT);
  }
}

int GetInput1()
{
  for (int i{0}; i!=n_inputs_1; ++i) 
  { 
    if (digitalRead(pins_input_1[i])) { return i; }
  }
  return -1;
}

int GetInput2()
{
  for (int i{0}; i!=n_inputs_2; ++i) 
  { 
    if (digitalRead(pins_input_2[i])) { return i; }
  }
  return -1;
}

void loop()
{
  const int a{GetInput1()};  
  const int b{GetInput2()};  
  if (a == -1 || b == -1) return;
  const int d{abs(a-b)};
  digitalWrite(pins_led_top[0],d == 1 || d == 3 ? HIGH : LOW);
  digitalWrite(pins_led_top[1],d == 0 ? HIGH : LOW);
  digitalWrite(pins_led_top[2],d == 2 || d == 4 ? HIGH : LOW);
  delay(100);
}

