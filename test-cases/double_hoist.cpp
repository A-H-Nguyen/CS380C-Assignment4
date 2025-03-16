int do_work(int a, int b) {
    int x = 0;
    for (int i = 0; i < 5; i++) {
        int c = a + b;
        x = a + c;
    }
    return x;
}


int main() {
    int a = do_work(3, 5);
    return a;
}