int main() {
  int x = 0, y = 0, z = 0;
  while ( x < 5) {
    while ( y < 5 ) {
      while ( z < 5 ) {
        z++;
      }
      y++;
    }
    x++;
  }
  return x + y + z;
}
