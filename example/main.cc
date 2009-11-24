#include "../nanothumb.h"

int main() {
    nanothumb::Gic *a = nanothumb::gic_jpeg_open("a.jpg");
    nanothumb::Gic *b = nanothumb::convert(a, 0.2);
    nanothumb::gic_write_image(b, "b.jpg", 80);

    return 0;
}
