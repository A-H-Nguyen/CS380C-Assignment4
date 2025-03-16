int square(int num, int a, int b) {
    int x = 0;
    int z = 1;
    for (int i = 0; i < 10; i++) {
        for(int j = 0; j < 30; j++) {
            x = a+b+i;
            z = 1+1;
        }
    }
    return x + z;
}

int main() {
  return square(0,1,2);
}
