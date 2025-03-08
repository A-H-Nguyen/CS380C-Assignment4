int main() {
  int x = 0, y, z;

  while (x < 5) {
    y = 0;
    x++;
  }

  while (y < 5) {
    z = 0;
    y++;
  }

  while (z < 5) {
    z++;
  }

  return x + y + z;
}
