
volatile int gabagoul = 0;
int foo() {
  int y;
  while (gabagoul < 5) {
      gabagoul++;
  }
  y = gabagoul;
  return y;
}

int bar() {
  int z;
  while (gabagoul > 0) {
    gabagoul--;
  }
  z = gabagoul;
  gabagoul = 5;
  return z;
}

int kay() {
    foo();
    bar();

    int x = foo();
    int y = bar();
    int z = 0;
    return z;
}

int main() {
  foo();
  bar();

  int x = foo(); // 5
  int y = bar(); // 0
  int z = 0;
  while (0 < gabagoul) {
    z = x+5;
    gabagoul--;
  }
  return z; // 10
}


