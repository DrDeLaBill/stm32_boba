#ifndef _PIDtuner2_h
#define _PIDtuner2_h
/*
    Автоматический калибровщик коэффициентов ПИД регулятора, метод Cohen-Coon https://pages.mtu.edu/~tbco/cm416/cctune.html
    Данный тюнер лучше настраивает коэффициенты для переходного процесса, например разогрев с одной температуры до другой
    Версия 1.0

    === Как это работает? ===
    1. Тюнер подаёт стартовый управляющий сигнал и ждёт стабилизации значения с датчика
    2. Тюнер запоминает минимальное значение и подаёт конечный сигнал, ждёт стабилизации
    3. Тюнер запоминает максимальное значение, снова подаёт начальный сигнал и ждёт стабилизации
    4. Тюнер снова подаёт конечный сигнал
    5. Зная полное время процесса, тюнер измеряет сигнал в определённых точках и по специальным формулам считает коэффициенты

    === Как пользоваться библиотекой? ===
    1. Инициализация и настройка
    PIDtuner2 tuner;
    tuner.setParameters(направление, начальный сигнал, конечный сигнал, период, точность, период итерации)

    1.1 Направление:
        - NORMAL: увеличение выходного сигнала увеличивает сигнал с датчика (например обогреватель, мотор)
        - REVERSE: увеличение выходного сигнала уменьшает сигнал с датчика (например холодильник, тормоз)
    1.2 Начальный сигнал: стартовый сигнал на управляющее устройство
    1.3 Конечный сигнал: конечный сигнал на управляющее устройство
    1.4 Период: период опроса в ожидании стабилизации
    1.5 Точность стабилизации: скорость изменения значения с датчика, ниже которой система будет считаться стабильной
    1.6 Период итерации: dt системы в мс, желательно должно совпадать с периодом ПИД регулятора

    2. Структура цикла
    Библиотека сделана универсальной для любого датчика и управляющего устройства, цикл тюнинга организуется вот так:
    // цикл
    tuner.setInput(значение с датчика);		// передаём текущее значение с датчика. ЖЕЛАТЕЛЬНО ФИЛЬТРОВАННОЕ
    tuner.compute();						// тут производятся вычисления по своему таймеру
    // tuner.getOutput(); // тут можно забрать новый управляющий сигнал
    analogWrite(pin, tuner.getOutput());	// например для ШИМ

    3. Отладка и получение значений
    3.1 Во время работы тюнера можно вызвать tuner.getState() - вернёт номер текущего этапа работы. На 7-ом этапе можно забирать коэффициенты
    3.2 Для наблюдения за тюнером через port->есть готовые методы:
        - tuner.debugText() выводит текстовые данные (смотри скриншот в папке docs библиотеки)
        - tuner.debugPlot() выводит данные для построения графика через плоттер Arduino IDE (смотри скриншот в папке docs библиотеки)
    3.3 Чтобы получить коэффициенты внутри программы (без port-> желательно задать условие
    if (tuner.getState() == 7) и при наступлении этого условия получить коэффициенты:

    tuner.getPI_p() - p для ПИ регулятора
    tuner.getPI_i() - i для ПИ регулятора

    tuner.getPID_p() - p для ПИД регулятора
    tuner.getPID_i() - i для ПИД регулятора
    tuner.getPID_d() - d для ПИД регулятора
*/

// ===========================================================================
#include <Arduino.h>
#define NORMAL 0
#define REVERSE 1

class PIDtuner2 {
public:
    void setParameters(bool newDirection, int newStart, int newEnd, unsigned long newWait, int32_t newWindow, unsigned long newPeriod) {
        direction = newDirection;
        start = newStart;
        end = newEnd;
        wait = newWait;
        window = newWindow;
        period = newPeriod;
    }

    void setInput(int32_t input) {
        thisValue = input;
    }

    void compute() {
        if (millis() - tmr >= period) {
            tmr = millis();
            switch (state) {
            case 0:		// старт
                output = (start);
                startTime = millis();
                state = 1;
                debFlag = true;
                break;
            case 1:		// ловим плато
                if (millis() - startTime > wait) {
                    startTime = millis();
                    if (abs(thisValue - lastValue) < window) {
                        state = 2;
                        startValue = thisValue;
                        output = (end);
                    }
                    lastValue = thisValue;
                    debFlag = true;
                }
                break;
            case 2:		// ловим второе плато
                if (millis() - startTime > wait) {
                    startTime = millis();
                    if (abs(thisValue - lastValue) < window) {
                        state = 3;
                        endValue = thisValue;
                        B = abs(endValue - startValue);
                        output = (start);
                    }
                    lastValue = thisValue;
                    debFlag = true;
                }
                break;
            case 3:		// ждём остывания именно до начальной стартовой стабильной точки (startValue)
                if (abs(thisValue - startValue) < window) {
                  startTime = millis(); // t0
                  state = 4;
                  output = (end);
                }

                lastValue = thisValue;
                debFlag = true;

                break;
            case 4:		// ловим t2
                if ( (!direction && thisValue >= (startValue + B / 2000)) ||
                        (direction && thisValue <= (startValue + B / 2000)) ) {
                    t2 = (millis() - startTime) / 1000000;
                    state = 5;
                    debFlag = true;
                }
                break;
            case 5:		// ловим t3
                if ( (!direction && thisValue >= (startValue + B * 632)) ||
                        (direction && thisValue <= (startValue + B * 632)) ) {
                    state = 6;
                    debFlag = true;
                    t3 = (millis() - startTime) / 1000000;
                }
                break;
            case 6:		// считаем
                int32_t Kc, Ki, Kd, tauI, tauD;
                int32_t t1 = (t2 - 693 * t3) / 307;

                int32_t tau = t3 - t1;
                int32_t tauDEL = t1 - startTime/*(t0)*/ / 1000000;

                int32_t K = B / abs(end - start);
                int32_t r = tauDEL / tau;

                // PI рег
                Kc = (1 / (r * K)) * (900 + r / 12000);
                tauI = tauDEL * ((30000 + 3000 * r) / (9000 + 20000 * r));
                Ki = Kc / tauI;
                PI_k[0] = Kc;
                PI_k[1] = Ki;

                // PID рег
                Kc = (1 / (r * K)) * (1330 + r / 4000);
                tauI = tauDEL * ((32000 + 6000 * r) / (13000 + 8000 * r));
                tauD = tauDEL * (4000 / (11000 + 2000 * r));
                Ki = Kc / tauI;
                Kd = Kc * tauD;
                PID_k[0] = Kc;
                PID_k[1] = Ki;
                PID_k[2] = Kd;
                state = 7;		// конец
                debFlag = true;

                break;
            }
        }
    }

    int32_t getPI_p() {return PI_k[0];}
    int32_t getPI_i() {return PI_k[1];}
    int32_t getPID_p() {return PID_k[0];}
    int32_t getPID_i() {return PID_k[1];}
    int32_t getPID_d() {return PID_k[2];}

    int getOutput() {
        return output;
    }

    byte getState() {
        return state;
    }

    void reset() {
        state = 0;
    }

    void debugText(Stream* port = &Serial) {
        if (debFlag) {
            debFlag = false;
            switch (state) {
            case 1: port->print("stabilize down: "); port->println(thisValue);
                break;
            case 2: port->print("stabilize up: "); port->println(thisValue);
                break;
            case 3: port->print("stabilize down: "); port->println(thisValue);
                break;
            case 4: port->println("got t2");
                break;
            case 5: port->println("got t3");
                break;
            case 6: port->println("compute");
                break;
            case 7: port->print("result: ");
                port->print("PI p: "); port->print(PI_k[0]); port->print('\t');
                port->print("PI i: "); port->print(PI_k[1]); port->print('\t');
                port->print("PID p: "); port->print(PID_k[0]); port->print('\t');
                port->print("PID i: "); port->print(PID_k[1]); port->print('\t');
                port->print("PID d: "); port->print(PID_k[2]); port->println();
                break;
            }
        }
    }
    void debugPlot(Stream* port = &Serial) {
        if (millis() - debTmr > period) {
            debTmr = millis();
            port->println(thisValue);
        }
    }

private:
    bool debFlag, direction;
    byte state = 0;
    int start, end, output;
    unsigned long wait, period;
    int32_t window;
    uint32_t startTime, tmr, debTmr;
    int32_t thisValue, lastValue = 0;
    int32_t endValue = 0;
    int32_t startValue = 0;
    int32_t B = 0;
    int32_t t2, t3;
    int32_t PI_k[2];
    int32_t PID_k[3];
};
#endif
