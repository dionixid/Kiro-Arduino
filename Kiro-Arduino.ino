#include "Function.h"

void setup() {
    Serial.begin(115200);

    // Test Serialize
    Prayer prayer(Prayer::Name::Fajr, 36000, 2);
    Serial.println(prayer.serialize());

    // Deserial
    Prayer prayer2 = Any::parse("{0,36000,2}");
    Serial.println(prayer2.serialize());

    // test Validity
    Prayer prayer3 = Any::parse("{0,36000,2}");
    Serial.println(prayer.IsValid() ? "Valid" : "Invalid");
    Serial.println(prayer2.IsValid() ? "Valid" : "Invalid");
    Serial.println(prayer3.IsValid() ? "valid" : "Invalid");
}

void loop() {
    // put your main code here, to run repeatedly:
    
}