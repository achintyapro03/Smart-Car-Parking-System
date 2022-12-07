#include <Wire.h>

#include <LiquidCrystal_I2C.h>

#include<time.h>

#include <TimeLib.h>

#include <Servo.h>

int pos = 0; //initial position
LiquidCrystal_I2C lcd1(0x27, 16, 2);
LiquidCrystal_I2C lcd2(0x26, 16, 2);
Servo entryServo;
Servo exitServo;
//pin 14for entry and 15 for exit

int num = 0;
int len = 0;
int entryInpNo = 1;
int exitInpNo = 1;
int car_reg[100];
int otp[100];
int enter_time[100];
int exit_time[100];
int total_fare[100];
int car_reg_counter = 0;

int s1 = A0; //ir sensor pins of slots
int s2 = A1;
int s3 = A2;
int s4 = A3;
int slarr[] = {
    s1,
    s2,
    s3,
    s4
};
int parkingState[4];

void setup() {
    Serial.begin(9600);

    entryServo.attach(51);
    exitServo.attach(52);
    Serial.begin(9600);
    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);
    pinMode(A5, INPUT);
    pinMode(A4, INPUT);
    for (int i = 13; i >= 2; i--) {
        pinMode(i, INPUT);
    }
    lcd1.begin();
    lcd1.backlight();
    lcd1.clear();
    //  lcd1.setCursor(0, 0);
    lcd2.begin();
    lcd2.backlight();
    lcd2.clear();
    //  lcd2.setCursor(0, 0);
    //  lcd1.print("ok");
    //  lcd2.print("please");
    // 
    // Serial.println("done !!");

}

int calcDig(int i, int j, int p1, int p2) {
    int r = p1 + 1 - i;
    int c = p2 + 1 - j;
    Serial.println("in calc");
    Serial.println(i);
    Serial.println(j);
    return (3 * (r - 1) + c);
}

void getInput(int p1, int p2) {
    num = 0;
    len = 0;
    Serial.println(num);

    while (1) {
        int flag = 0;
        for (int i = p1; i > p1 - 3; i--) {
            if (digitalRead(i)) {
                for (int j = p2; j > p2 - 3; j--) {
                    if (digitalRead(j)) {
                        len += 1;
                        num = num * 10 + calcDig(i, j, p1, p2);
                        while ((digitalRead(i) == 1 && digitalRead(j) == 1)) {}
                        flag = 1;
                    }
                }
            }
            if (flag) break;
        }

        if (len == 4) {
            Serial.println(num);
            break;
        }
    }
}

int find_car_i(int car_reg_no) {
    for (int i = 0; i < 100; i++) {
        if (car_reg_no == car_reg[i]) {
            return i;
        }
    }
    return -1;
}

int otp_generator() {

    int number1[10] = {
        1432,
        5472,
        6243,
        6123,
        9856,
        9623,
        7452,
        9262,
        1392,
        5139
    };
    int number = rand() % 10;
    return number1[number];
}

void fare_calculator(int x) {
    int fare = exit_time[x] - enter_time[x];
    total_fare[x] = fare * .24;
}

int nearestSlot() {
    int left = -1;
    int right = -1;
    for (int i = 1; i >= 0; i--) {
        if (parkingState[i] == 0) {
            left = i;
            break;
        }
    }
    for (int i = 2; i <= 3; i++) {
        if (parkingState[i] == 0) {
            right = i;
            break;
        }
    }
    if (abs(1.5 - left) <= abs(1.5 - right)) {
        return left;
    } else
        return right;
}

void gateOpen(int gate) {
    Serial.println("START notor");
    Serial.println(gate);
    Serial.println(entryServo.read());
    for (pos = 0; pos <= 90; pos += 1) {
        if (gate == 51)
            entryServo.write(pos);
        else
            exitServo.write(pos);
        delay(50);
    }
    for (pos = 90; pos >= 0; pos -= 1) {
        if (gate == 51)
            entryServo.write(pos);
        else
            exitServo.write(pos);
        delay(50);
    }
}

int entryInput() {
    Serial.println("entered inp");
    int nearest = nearestSlot();

    if (nearest == -1) {
        lcd1.clear();
        lcd1.setCursor(0, 0);
        lcd1.print("No free slots");
        lcd1.setCursor(0, 1);
        lcd1.print("Sorry ;..( ");
        delay(3000);
        entryInpNo = 0;
        return;
    }

    num = 0;
    len = 0;
    Serial.println("Input");
    int seconds = second();
    int minutes = minute();
    int hours = hour();
    int in_timing = seconds + (60 * minutes) + (hours * 3600);
    enter_time[car_reg_counter] = in_timing;

    lcd1.clear();
    lcd1.setCursor(0, 0);
    lcd1.print("Enter reg no");
    int otp_of_car = 0;
    getInput(13, 10);
    int reg_number = num;

    lcd1.clear();
    lcd1.print("Confirm reg no");
    lcd1.setCursor(0, 1);
    lcd1.print(reg_number);
    lcd1.print(" 1-ok 2-no");

    int x = 0;
    int y = 0;
    while (1) {
        x = digitalRead(13) && digitalRead(10);
        y = digitalRead(13) && digitalRead(9);

        if (x || y) {
            break;
        }
    }

    if (x) {
        entryInpNo = 0;
        while ((digitalRead(13) == 1 && digitalRead(10) == 1)) {}
    } else if (y) {
        entryInpNo = 1;
        num = 0;
        len = 0;
        while ((digitalRead(13) == 1 && digitalRead(9) == 1)) {}
        return;
    }

    car_reg[car_reg_counter] = reg_number;

    otp_of_car = otp_generator();

    lcd1.clear();
    lcd1.setCursor(0, 0);
    lcd1.print("Your otp");
    lcd1.setCursor(0, 1);
    lcd1.print(otp_of_car);

    otp[car_reg_counter] = otp_of_car;
    car_reg_counter++;
    delay(5000);

    lcd1.clear();
    lcd1.setCursor(0, 0);
    lcd1.print("Nearest slot : ");
    lcd1.print(nearest + 1);
    // lcd1.setCursor(0, 1);
    // lcd1.print("Take a ");
    // lcd1.print((nearest <= 1.5) ? "left" : "right");
    gateOpen(51);
    Serial.println("Gate opening");
    delay(3000);
}

int exitInput() {
    Serial.println("In exit");
    int exit_counter = 0;

    int carIdx;
    int reg_counter = 0;
    int validReg = 0;
    int reg_number;
    while (validReg == 0 && reg_counter < 3) {
        lcd2.clear();
        lcd2.setCursor(0, 0);
        lcd2.print("Enter reg no");
        getInput(7, 4);
        reg_number = num;
        carIdx = find_car_i(reg_number);
        reg_counter++;
        if (carIdx == -1) {
            lcd2.clear();
            lcd2.setCursor(0, 0);
            lcd2.print("Reg num missing");
            lcd2.setCursor(0, 1);
            lcd2.print((3 - reg_counter));
            lcd2.print(" attempts left");
            delay(3000);
        } else {
            validReg = 1;
            break;
        }
    }

    lcd2.clear();
    lcd2.print("Confirm reg no");
    lcd2.setCursor(0, 1);
    lcd2.print(reg_number);
    lcd2.print(" 1-ok 2-no");
    int x = 0;
    int y = 0;
    while (1) {
        x = digitalRead(7) && digitalRead(4);
        y = digitalRead(7) && digitalRead(3);
        if (x || y) {
            break;
        }
    }

    if (validReg == 1) {
        if (x) {
            exitInpNo = 0;
            while ((digitalRead(7) == 1 && digitalRead(4) == 1)) {}
        } else if (y) {
            exitInpNo = 1;
            while ((digitalRead(7) == 1 && digitalRead(3) == 1)) {}
            return;
        }
        int validOTP = 0;

        int carIdx = find_car_i(reg_number);

        while (validOTP == 0 && exit_counter < 3) {
            lcd2.clear();
            lcd2.setCursor(0, 0);
            lcd2.print("Enter OTP");
            getInput(7, 4);
            int otpEnt = num;
            if (otpEnt == otp[carIdx] && exit_counter < 3) {
                lcd2.clear();
                lcd2.setCursor(0, 0);
                lcd2.print("CORRECT OTP!");
                delay(2000);

                validOTP = 1;
                break;
            } else if (exit_counter < 3) {
                exit_counter++;
                lcd2.clear();
                lcd2.setCursor(0, 0);
                lcd2.print("WRONG OTP!");
                lcd2.setCursor(0, 1);
                lcd2.print((3 - exit_counter));
                lcd2.print(" attempts left");
                delay(4000);
            } //else case..when wrong otp is entered

        }

        if (validOTP == 1) {
            int seconds = second();
            int minutes = minute();
            int hours = hour();
            int out_timing = seconds + (60 * minutes) + (hours * 3600);
            exit_time[carIdx] = out_timing;
            fare_calculator(carIdx);
            lcd2.clear();
            lcd2.setCursor(0, 0);
            lcd2.print("Fare: Rs");
            // lcd2.setCursor(0, 1);
            lcd2.print(total_fare[carIdx]);
            delay(5000);
            // lcd2.print(" attempts left")
            Serial.println("Your total fare: Rs");
            Serial.print(total_fare[carIdx]);
        } else {
            lcd2.clear();
            lcd2.setCursor(0, 0);
            lcd2.print("Fine amt:Rs1000");
            delay(5000);
        }
    } else {
        lcd2.clear();
        lcd2.setCursor(0, 0);
        lcd2.print("Wrong reg num");
        lcd2.setCursor(0, 1);
        lcd2.print("Fine amt :Rs1000");
        delay(4000);

    }

    gateOpen(52);
    car_reg[carIdx] = -1;
    otp[carIdx] = -1;

}

int checkState() { //returns no. of empty slot to display in lcd

    int count = 0;
    for (int i = 0; i < 4; i++) {
        // Serial.print(i);
        // Serial.print(" : ");
        // Serial.println(analogRead(slarr[i]));
        if (analogRead(slarr[i]) < 500) {
            count += 1;
            parkingState[i] = 1;
        } else {
            parkingState[i] = 0;
        }

    }

    // for (int i = 0; i < 4; i++) {
    //     Serial.print(i);
    //     Serial.print(" : ");
    //     Serial.println(parkingState[i]);
    // }
    return count;
}
// 
void loop() {
    // Serial.println("hi" );
    // Serial.println(analogRead(A5));
    // delay(200);
    // Serial.println(analogRead(A4));

    // delay(1000);
    // lcd2.setCursor(0, 0);
    // lcd2.print("hello");
    checkState();
    if (analogRead(A4) < 500) {
        lcd1.clear();
        while (entryInpNo == 1)
            entryInput();
        entryInpNo = 1;
        lcd1.clear();
    }
    if (analogRead(A5) < 300) {
        lcd2.clear();
        while (exitInpNo == 1) {
            Serial.println("exit loop");
            exitInput();
        }
        exitInpNo = 1;
        lcd2.clear();
    }

    // delay(2000);
}