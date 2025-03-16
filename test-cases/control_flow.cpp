int main() {
  int x = 0;
  while (true) {
    if (x < 5) {
      x += 1;
    }
    else {
        x -= 1;
    }
    if(x==5){
        break;
    }
  }
  return x;
}
