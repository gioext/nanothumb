#include <iostream>
#include <pthread.h>

using namespace std;

struct ThreadParam {
    int no;
    int start;
    int end;
    int *result;
};

void *work(void *n) {
    int i;
    ThreadParam *p = (ThreadParam *)n;
    *(p->result) = 0;

    for (i = p->start; i < p->end; i++) {
        *(p->result) += i;
    }
}

int main() {
    pthread_t th1;
    int result[2];

    ThreadParam p1;
    p1.no = 1;
    p1.start = 0;
    p1.end = 10000;
    p1.result = &result[0];

    ThreadParam p2;
    p2.no = 2;
    p2.start = 10001;
    p2.end = 20000;
    p2.result = &result[1];

    pthread_create(&th1, NULL, work, &p1);
    work(&p2);

    pthread_join(th1, NULL);

    cout << result[0] << endl;
    cout << result[1] << endl;
    return 0;
}
