#include <iostream>
#include <string>

namespace nanothumb {
    using namespace std;

    class Image {
    public:
        int m_width;
        int m_height;
    };

    class Conv {
    private:
        string m_in;
        Image in;
        Image out;

    public:
        Conv(string in) {
            m_in = in;
            cout << "open jpeg: " << in << endl;
        }

        ~Conv() {
            cout << "free jpeg" << endl;
        }

        inline void thumb(string out, double rate, int quality) {
            cout << "- convert " << m_in << " to " << out;
            cout << " rate:" << rate << " quality:" << quality << endl;
        }

    };

    inline void thumb(string in, string out, double rate, int quality) {
        nanothumb::Conv c(in);
        c.thumb(out, rate, quality);
    }
}

int main()
{
    {
        nanothumb::Conv c("a.jpg");
        c.thumb("b.jpg", 0.2, 90);
        c.thumb("b.jpg", 0.5, 90);
    }

    {
        nanothumb::thumb("c.jpg", "d.jpg", 0.6, 90);
    }

    return 0;
}
