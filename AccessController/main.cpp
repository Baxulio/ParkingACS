#include "MainWindow.h"
#include <QApplication>

///////////////////////// WIEGAND
#include <signal.h>
#include <sys/time.h>

MainWindow *w;

void wiegand_sequence_reset();
void show_code();
void reset_timeout_timer(long usec);
void wiegand_timeout(int u);
int setup_wiegand_timeout_handler();
void add_bit_w26(int bit);
void d0_pulse(void);
void d1_pulse(void);
void interrupt(void);

bool gpio_init();

///////////////////////////////////


int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(accesscontrollerresources);

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Baxulio");
    QCoreApplication::setApplicationName("Attraction");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    MainWindow main;
    w=&main;
    if(!gpio_init())
        return EXIT_FAILURE;

    main.show();

    return app.exec();
}

///////////////////////// WIEGAND

struct itimerval it_val;
struct sigaction sa;

struct wiegand_data {
    unsigned char p0, p1;       //parity 0 , parity 1
    quint8 facility_code;
    quint16 card_code;
    quint32 full_code;
    int code_valid;
    unsigned long bitcount;     // bits read
} wds;

struct option_s {
    int d0pin;
    int d1pin;
} options;


void wiegand_sequence_reset() {
    wds.bitcount = 0;
}

void reset_timeout_timer(long usec) {
    it_val.it_value.tv_sec = 0;
    it_val.it_value.tv_usec = usec;

    it_val.it_interval.tv_sec = 0;
    it_val.it_interval.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
        perror("setitimer");
        exit(1);
    }
}

void wiegand_timeout(int u) {
    wiegand_sequence_reset();
    if(wds.code_valid and digitalRead(BAREER_PIN)==LOW){
        w->wiegandCallback(wds.full_code);
    }
}

int setup_wiegand_timeout_handler() {
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = wiegand_timeout;
    //sa.sa_flags = SA_SIGINFO;

    if (sigaction(SIGALRM, &sa, NULL) < 0) {
        perror("sigaction");
        return 1;
    };
    return 0;
}

void add_bit_w26(int bit) {
    static int parity0 = 0;
    static int parity1 = 0;

    //Parity calculation
    if (wds.bitcount == 0) {
        parity0 = 0;
        parity1 = 0;
    }
    else if (wds.bitcount > 0 && wds.bitcount <= 12) {
        parity0 += bit;
    }
    else if (wds.bitcount >= 13 && wds.bitcount <= 24) {
        parity1 += bit;
    }
    //Code calculation
    if (wds.bitcount == 0) {
        /* Reset */
        wds.code_valid = 0;
        wds.facility_code = 0;
        wds.card_code = 0;
        wds.p0 = bit;           //parity
    }
    else if (wds.bitcount <= 8) {
        wds.facility_code <<= 1;
        if (bit)
            wds.facility_code |= 1;
    }
    else if (wds.bitcount < 25) {
        wds.card_code <<= 1;
        if (bit)
            wds.card_code |= 1;
    }
    else if (wds.bitcount == 25) {
        wds.p1 = bit;
        wds.full_code = wds.facility_code;
        wds.full_code = wds.full_code << 16;
        wds.full_code += wds.card_code;

        wds.code_valid = 1;
        //check parity
        if ((parity0 % 2) != wds.p0) {
            wds.code_valid = 0;
        }
        else if ((!(parity1 % 2)) != wds.p1) {
            wds.code_valid = 0;
        }

    }
    else if (wds.bitcount > 25) {
        wds.code_valid = 0;
        wiegand_sequence_reset();
    }

    if (wds.bitcount < WIEGANDMAXBITS) {
        wds.bitcount++;
    }

}

void d0_pulse() {
    reset_timeout_timer(WIEGAND_BIT_INTERVAL_TIMEOUT_USEC);     //timeout waiting for next bit
    add_bit_w26(0);
}

void d1_pulse() {
    reset_timeout_timer(WIEGAND_BIT_INTERVAL_TIMEOUT_USEC);     //timeout waiting for next bit
    add_bit_w26(1);
}

void interrupt(void){
    w->interrupt();
}

bool gpio_init() {

    if(setup_wiegand_timeout_handler()!=0)return false;

    if(wiringPiSetup()<0)return false;

    options.d0pin = D0_PIN;
    options.d1pin = D1_PIN;

    pinMode(BAREER_PIN, OUTPUT);
    pinMode(BREAK_PIN, INPUT);

    pinMode(options.d0pin, INPUT);
    pinMode(options.d1pin, INPUT);

    pullUpDnControl(BREAK_PIN, PUD_UP);

    pullUpDnControl(options.d0pin, PUD_OFF);
    pullUpDnControl(options.d1pin, PUD_OFF);

    wiringPiISR(BREAK_PIN, INT_EDGE_FALLING, interrupt);

    wiringPiISR(options.d0pin, INT_EDGE_FALLING, d0_pulse);
    wiringPiISR(options.d1pin, INT_EDGE_FALLING, d1_pulse);

    wiegand_sequence_reset();

    return true;
}
