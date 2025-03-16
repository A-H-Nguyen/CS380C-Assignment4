
int main() {
    int a = 10;
    int b = 5;
    int sum = 0;

    // A loop where `a * b` is loop invariant
    for (int i = 0; i < 5; ++i) {
        sum += a * b;  // a * b is loop invariant and safe to hoist
    }


    return sum;
}